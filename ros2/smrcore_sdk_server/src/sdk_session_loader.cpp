#include "sdk_session.hpp"

#include <dlfcn.h>
#include <link.h>

#include <memory>
#include <stdexcept>

namespace smrcore_sdk_server
{
namespace
{

class LoadedSdkSession
{
public:
    LoadedSdkSession()
    {
        handle_ = dlmopen(LM_ID_NEWLM, "libsmrcore_sdk_session.so",
                          RTLD_NOW | RTLD_LOCAL);
        if (handle_ == nullptr)
        {
            throw std::runtime_error(dlerror());
        }
        create_ = reinterpret_cast<CreateSdkSessionFn>(
            dlsym(handle_, "CreateSdkSession"));
        destroy_ = reinterpret_cast<DestroySdkSessionFn>(
            dlsym(handle_, "DestroySdkSession"));
        if (create_ == nullptr || destroy_ == nullptr)
        {
            const char *error = dlerror();
            throw std::runtime_error(error != nullptr ? error
                                                      : "invalid SDK session library");
        }
    }

    ~LoadedSdkSession()
    {
        if (handle_ != nullptr)
        {
            dlclose(handle_);
        }
    }

    SdkSession *Create() const { return create_(); }
    void Destroy(SdkSession *session) const { destroy_(session); }

private:
    void *handle_ = nullptr;
    CreateSdkSessionFn create_ = nullptr;
    DestroySdkSessionFn destroy_ = nullptr;
};

struct SessionDeleter
{
    std::shared_ptr<LoadedSdkSession> library;

    void operator()(SdkSession *session) const
    {
        if (session != nullptr)
        {
            library->Destroy(session);
        }
    }
};

} // namespace

std::shared_ptr<SdkSession> LoadSdkSession()
{
    auto library = std::make_shared<LoadedSdkSession>();
    return std::shared_ptr<SdkSession>(library->Create(), SessionDeleter{library});
}

} // namespace smrcore_sdk_server
