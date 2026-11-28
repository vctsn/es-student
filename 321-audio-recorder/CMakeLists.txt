cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(321_audio_recorder)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_compile_options(-Wall)

# libs лежит вне папки проекта: второй аргумент задаёт каталог сборки
add_subdirectory(../libs ${CMAKE_BINARY_DIR}/libs)

# выключатель профилирования: 0 — вызовов секундомеров в образе нет
target_compile_definitions(profiling PUBLIC
    PROFILING_ENABLED=1
)

# железо прибора: единственное место проекта с SDK
add_library(platform STATIC
    platform/platform.c
)
target_include_directories(platform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# логика прибора: задачи и команды, без SDK
add_library(logic STATIC
    logic/api/api.c
    logic/api/api-callbacks.c
    logic/led-task/led-task.c
    logic/button-task/button-task.c
    logic/pi-task/pi-task.c
    logic/telemetry-task/telemetry-task.c
    logic/block-task/block-task.c
    logic/stream-task/stream-task.c
)
target_include_directories(logic PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/logic
)
target_link_libraries(logic
    PUBLIC
        stdio_text_protocol
    PRIVATE
        platform
        device
        profiling
        systime
)

target_link_libraries(platform PRIVATE
    pico_stdlib
    hardware_adc
    hardware_dma
)

add_executable(${PROJECT_NAME}
    main.c
)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    platform
    logic
    profiling
    logging
    pico_stdlib
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})
