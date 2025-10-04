
cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(fw_challenge)

target_sources(app PRIVATE
    src/main.c
    src/adc_sampler.c
    src/status_led.c
    src/buttons.c
    src/ble.c
)
target_include_directories(app PRIVATE include)
