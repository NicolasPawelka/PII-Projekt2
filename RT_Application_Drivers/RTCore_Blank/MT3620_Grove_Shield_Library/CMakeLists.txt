CMAKE_MINIMUM_REQUIRED(VERSION 3.8)
PROJECT(MT3620_Grove_Shield_Library C)

################################################################################
# Source groups
################################################################################
set(HAL
    src/HAL/GroveI2C.c
    src/HAL/GroveShield.c
    src/HAL/GroveUART.c
)
source_group("HAL" FILES ${HAL})


set(ALL_FILES
    ${no_group_source_files}
    ${HAL}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC include)

set(ROOT_NAMESPACE MT3620_Grove_Shield_Library)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "AzureSphere"
)

target_link_libraries (${PROJECT_NAME} applibs pthread gcc_s c)
