#  Copyright (c) Microsoft Corporation. All rights reserved.
#  Licensed under the MIT License.

cmake_minimum_required (VERSION 3.20)

project (RT_test C)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

## Uncomment the following line if you are using Azure Sphere SDK version 23.05 or earlier:
# azsphere_configure_tools(TOOLS_REVISION "23.05")
# --- FreeRTOS include paths ---
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/FreeRTOS-Kernel/include
    ${CMAKE_SOURCE_DIR}/FreeRTOS-Kernel/portable/GCC/ARM_CM4F
    ${CMAKE_SOURCE_DIR}/MT3620_M4_BSP/printf
    ${CMAKE_SOURCE_DIR}/MT3620_M4_BSP/mt3620/inc
    ${CMAKE_SOURCE_DIR}/MT3620_M4_BSP/CMSIS/include
    ${CMAKE_SOURCE_DIR}/MT3620_M4_BSP/CMSIS/Device/Mediatek/Include
    ${CMAKE_SOURCE_DIR}/MT3620_M4_BSP/mt3620/inc
)

# --- FreeRTOS source files ---
file(GLOB FREERTOS_SRC
    ${CMAKE_SOURCE_DIR}/FreeRTOS-Kernel/*.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS-Kernel/portable/MemMang/heap_4.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c
)

# --- Deine App-Quellen ---
set(APP_SRC
    main.c
)


# Create executable
add_executable (${PROJECT_NAME} ${APP_SRC} ${FREERTOS_SRC} main.c)
target_link_libraries (${PROJECT_NAME})
set_target_properties (${PROJECT_NAME} PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/linker.ld)

azsphere_target_add_image_package(${PROJECT_NAME})
