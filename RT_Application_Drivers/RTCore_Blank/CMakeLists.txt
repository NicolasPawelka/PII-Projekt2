cmake_minimum_required(VERSION 3.20)

project(RTCore_Blank C)

# --- Definitions ---
add_compile_definitions(OSAI_FREERTOS)
add_compile_definitions(OSAI_ENABLE_DMA)


add_compile_options(-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
add_link_options(-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")


# --- Include paths ---
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/MT3620_M4_BSP/printf
    ${CMAKE_SOURCE_DIR}/MT3620_M4_BSP/mt3620/inc
    ${CMAKE_SOURCE_DIR}/MT3620_M4_BSP/CMSIS/include
    ${CMAKE_SOURCE_DIR}/MT3620_M4_BSP/CMSIS/Device/Mediatek/Include
    ${CMAKE_SOURCE_DIR}/MT3620_M4_BSP/FreeRTOS/include
    ${CMAKE_SOURCE_DIR}/MT3620_M4_BSP/FreeRTOS/portable
    ${CMAKE_SOURCE_DIR}/lib
   
)

# --- App sources ---
set(APP_SRC
    main.c
    ${CMAKE_SOURCE_DIR}/lib/VectorTable.c
    ${CMAKE_SOURCE_DIR}/MT3620_M4_BSP/FreeRTOS/tasks.c
    ${CMAKE_SOURCE_DIR}/MT3620_M4_BSP/FreeRTOS/list.c
    ${CMAKE_SOURCE_DIR}/MT3620_M4_BSP/FreeRTOS/timers.c
    ${CMAKE_SOURCE_DIR}/MT3620_M4_BSP/FreeRTOS/stream_buffer.c
    ${CMAKE_SOURCE_DIR}/MT3620_M4_BSP/FreeRTOS/queue.c
    ${CMAKE_SOURCE_DIR}/MT3620_M4_BSP/FreeRTOS/portable/heap_4.c
    ${CMAKE_SOURCE_DIR}/MT3620_M4_BSP/FreeRTOS/portable/port.c
    ${CMAKE_SOURCE_DIR}/lib/UART.c
    ${CMAKE_SOURCE_DIR}/lib/Print.c
    ${CMAKE_SOURCE_DIR}/lib/GPIO.c
    ${CMAKE_SOURCE_DIR}/lib/GPT.c
    ${CMAKE_SOURCE_DIR}/lib/LSM6DS3.c
    ${CMAKE_SOURCE_DIR}/lib/I2CMaster.c

)

# --- Executable ---
add_executable(${PROJECT_NAME} ${APP_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/linker.ld)



set(OSAI_FREERTOS 1)
# Create executable
# Einheitliche FPU-Einstellungen für Compiler + Linker
add_compile_options(-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

target_link_libraries(${PROJECT_NAME})

set_target_properties (${PROJECT_NAME} PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/linker.ld)

azsphere_target_add_image_package(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PUBLIC -L"${CMAKE_SOURCE_DIR}")
