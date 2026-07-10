cmake_minimum_required(VERSION 3.8)
project(smrcore_sdk_server)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(smrcore_msgs REQUIRED)
find_package(smrcore_sdk REQUIRED)

get_target_property(SMRCORE_SDK_LIBRARY smrcore::sdk IMPORTED_LOCATION_RELEASE)
if(NOT SMRCORE_SDK_LIBRARY)
  get_target_property(SMRCORE_SDK_LIBRARY smrcore::sdk IMPORTED_LOCATION)
endif()
if(NOT SMRCORE_SDK_LIBRARY)
  message(FATAL_ERROR "smrcore::sdk does not expose an imported library location")
endif()
get_filename_component(SMRCORE_SDK_LIBRARY_DIR "${SMRCORE_SDK_LIBRARY}" DIRECTORY)

add_library(smrcore_sdk_session SHARED src/sdk_session_impl.cpp)
target_compile_features(smrcore_sdk_session PUBLIC cxx_std_17)
target_link_libraries(smrcore_sdk_session smrcore::sdk)
set_target_properties(smrcore_sdk_session PROPERTIES
  BUILD_RPATH "${SMRCORE_SDK_LIBRARY_DIR}"
  INSTALL_RPATH "$ORIGIN"
)

add_executable(smrcore_sdk_server
  src/sdk_server_node.cpp
  src/sdk_session_loader.cpp
)
target_compile_features(smrcore_sdk_server PUBLIC cxx_std_17)
ament_target_dependencies(smrcore_sdk_server
  geometry_msgs
  rclcpp
  rclcpp_action
  smrcore_msgs
)
target_link_libraries(smrcore_sdk_server ${CMAKE_DL_LIBS})
set_target_properties(smrcore_sdk_server PROPERTIES
  INSTALL_RPATH "$ORIGIN/.."
)

install(TARGETS smrcore_sdk_server DESTINATION lib/${PROJECT_NAME})
install(TARGETS smrcore_sdk_session LIBRARY DESTINATION lib)
install(
  DIRECTORY "${SMRCORE_SDK_LIBRARY_DIR}/"
  DESTINATION lib
  FILES_MATCHING PATTERN "*.so*"
)
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})

ament_package()
