#  Copyright (c) Microsoft Corporation. All rights reserved.
#  Licensed under the MIT License.

cmake_minimum_required (VERSION 3.20)

project (UltrasonicTest C)


## Uncomment the following two lines if you are using Azure Sphere SDK version 23.05 or earlier:

# azsphere_configure_tools(TOOLS_REVISION "23.05")
# azsphere_configure_api(TARGET_API_SET "16")

# Create executable
macro(compileAsC99)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")
    if (NOT IN_OPENWRT)
      set (CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
    endif()
    endif()
  else()
    set (CMAKE_C_STANDARD 99)
    set (CMAKE_CXX_STANDARD 11)
  endif()
endmacro(compileAsC99)
add_executable (${PROJECT_NAME} main.c)
target_link_libraries (${PROJECT_NAME} applibs pthread gcc_s c)
azsphere_target_hardware_definition(${PROJECT_NAME} TARGET_DIRECTORY "HardwareDefinitions/mt3620_rdb" TARGET_DEFINITION "template_appliance.json")

azsphere_target_add_image_package(${PROJECT_NAME})
