#  Copyright (c) Microsoft Corporation. All rights reserved.
#  Licensed under the MIT License.

cmake_minimum_required (VERSION 3.20)

project (Send_telemetry C)

add_executable(${PROJECT_NAME} main.c)
## Uncomment the following two lines if you are using Azure Sphere SDK version 23.05 or earlier:
add_subdirectory(common)
# azsphere_configure_tools(TOOLS_REVISION "23.05")
# azsphere_configure_api(TARGET_API_SET "16")

target_include_directories(${PROJECT_NAME} PUBLIC
                           ${AZURE_SPHERE_API_SET_DIR}/usr/include/azureiot
                           ${AZURE_SPHERE_API_SET_DIR}/usr/include/azure_c_shared_utility
)           

# Create executable
target_link_libraries (${PROJECT_NAME} m azureiot applibs gcc_s c)

azsphere_target_add_image_package(${PROJECT_NAME})
