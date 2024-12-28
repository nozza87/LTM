# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)
#cmake_minimum_required(VERSION 3.5)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
#set(PICO_BOARD waveshare_rp2040_zero CACHE STRING "Board type")
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

# Include USB
set(FAMILY rp2040)
#include(${USERHOME}/.pico-sdk/sdk/2.1.0/external/pico_sdk_import.cmake)
include(${USERHOME}/.pico-sdk/sdk/2.1.0/tools/CMakeLists.txt)
include(${USERHOME}/.pico-sdk/sdk/2.1.0/lib/tinyusb/hw/bsp/family_support.cmake)

project(LTM C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(LTM LTM.cpp WS2812.cpp)

target_sources(LTM PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/hid_app.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LTM.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tusb_config.h
    ${CMAKE_CURRENT_LIST_DIR}/gears.h
    ${CMAKE_CURRENT_LIST_DIR}/gears.cpp
    ${CMAKE_CURRENT_LIST_DIR}/handbrake.h
    ${CMAKE_CURRENT_LIST_DIR}/handbrake.cpp
)

pico_set_program_name(LTM "LTM")
pico_set_program_version(LTM "0.1")

# Generate PIO header
pico_generate_pio_header(LTM ${CMAKE_CURRENT_LIST_DIR}/WS2812.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_usb(LTM 0)
pico_enable_stdio_uart(LTM 1)

# Add the standard library and any user requested libraries to the build
target_link_libraries(LTM 
    pico_stdlib
    hardware_i2c
    hardware_adc
    hardware_uart
    tinyusb_host
    tinyusb_board
    hardware_pio
)

# Add the standard include files to the build
#target_include_directories(LTM PRIVATE
target_include_directories(LTM PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

#target_compile_definitions(LTM PRIVATE
#  PICO_DEFAULT_UART=0
#  PICO_DEFAULT_UART_TX_PIN=16
#  PICO_DEFAULT_UART_RX_PIN=17
#)

pico_add_extra_outputs(LTM)

