cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(231_firmware_template)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_subdirectory(../libs ${CMAKE_BINARY_DIR}/libs)

# Модуль platform: единственный модуль проекта, видящий заголовки Pico SDK (П4, П5)
add_library(platform STATIC
    platform/platform.c
)
target_include_directories(platform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(platform PRIVATE
    pico_stdlib
)

# Библиотека логики прибора (собирается строго без заголовков SDK — П4)
add_library(logic STATIC
    logic/api/api.c
    logic/api/api-callbacks.c
)
target_include_directories(logic PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/logic
)
target_link_libraries(logic
    PUBLIC
        stdio_text_protocol
    PRIVATE
        device
)

# Исполняемый файл прошивки (main.c без заголовков SDK — П7)
add_executable(${PROJECT_NAME}
    main.c
)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/logic
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    platform
    logic
    stdio_text_protocol
    device
    pico_standard_link
)

target_compile_options(platform PRIVATE -Wall)
target_compile_options(logic PRIVATE -Wall)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})
