cmake_minimum_required(VERSION 3.8)
project(smrcore_hardware)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(smrcore_sdk REQUIRED)

function(collect_target_includes target out_var)
  if(NOT TARGET "${target}")
    return()
  endif()

  get_target_property(local_includes "${target}" INTERFACE_INCLUDE_DIRECTORIES)
  get_target_property(link_libraries "${target}" INTERFACE_LINK_LIBRARIES)
  if(NOT local_includes)
    set(local_includes)
  endif()
  if(NOT link_libraries)
    set(link_libraries)
  endif()
  set(all_includes ${local_includes})
  foreach(link_library IN LISTS link_libraries)
    if(TARGET "${link_library}")
      collect_target_includes("${link_library}" child_includes)
      list(APPEND all_includes ${child_includes})
    endif()
  endforeach()
  if(all_includes)
    list(REMOVE_DUPLICATES all_includes)
  endif()
  set("${out_var}" "${all_includes}" PARENT_SCOPE)
endfunction()

collect_target_includes(hardware_interface::hardware_interface ROS_INTERFACE_INCLUDES)
collect_target_includes(pluginlib::pluginlib PLUGINLIB_INTERFACE_INCLUDES)

get_target_property(SMRCORE_SDK_LIBRARY smrcore::sdk IMPORTED_LOCATION_RELEASE)
if(NOT SMRCORE_SDK_LIBRARY)
  get_target_property(SMRCORE_SDK_LIBRARY smrcore::sdk IMPORTED_LOCATION)
endif()
if(NOT SMRCORE_SDK_LIBRARY)
  message(FATAL_ERROR "smrcore::sdk does not expose an imported library location")
endif()
get_filename_component(SMRCORE_SDK_LIBRARY_DIR "${SMRCORE_SDK_LIBRARY}" DIRECTORY)

add_library(smrcore_hardware SHARED
  src/smrcore_system_interface.cpp
)
target_compile_features(smrcore_hardware PUBLIC cxx_std_17)
target_include_directories(smrcore_hardware PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_include_directories(smrcore_hardware SYSTEM PRIVATE
  ${ROS_INTERFACE_INCLUDES}
  ${PLUGINLIB_INTERFACE_INCLUDES}
)
target_link_libraries(smrcore_hardware
  "$<TARGET_FILE:hardware_interface::hardware_interface>"
  "$<TARGET_FILE:rclcpp_lifecycle::rclcpp_lifecycle>"
  "$<TARGET_FILE:rclcpp::rclcpp>"
  "$<TARGET_FILE:class_loader::class_loader>"
  "$<TARGET_FILE:rcutils::rcutils>"
  smrcore::sdk
)
target_link_options(smrcore_hardware PRIVATE -Wl,--as-needed)
set_target_properties(smrcore_hardware PROPERTIES
  BUILD_RPATH "${SMRCORE_SDK_LIBRARY_DIR}"
  INSTALL_RPATH "$ORIGIN"
)

pluginlib_export_plugin_description_file(hardware_interface smrcore_hardware_interface.xml)

install(
  TARGETS smrcore_hardware
  EXPORT export_smrcore_hardware
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
install(
  DIRECTORY "${SMRCORE_SDK_LIBRARY_DIR}/"
  DESTINATION lib
  FILES_MATCHING PATTERN "*.so*"
)
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY config launch DESTINATION share/${PROJECT_NAME})
install(FILES smrcore_hardware_interface.xml DESTINATION share/${PROJECT_NAME})

ament_export_targets(export_smrcore_hardware HAS_LIBRARY_TARGET)
ament_export_dependencies(hardware_interface pluginlib rclcpp rclcpp_lifecycle)
ament_package()
