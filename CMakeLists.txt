# Interactive Robot Pico Project - Main CMakeLists.txt

cmake_minimum_required(VERSION 3.13)

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
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(robot_pico C CXX ASM)

# Add ST7789 library
add_subdirectory(lib/st7789-library-for-pico-main)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Collect all source files for the interactive robot
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.c"
)

# Add executable with collected source files
add_executable(robot_pico ${SOURCES})

pico_set_program_name(robot_pico "Interactive Robot")
pico_set_program_version(robot_pico "1.0")

# Enable USB stdio for communication with PC
pico_enable_stdio_uart(robot_pico 0)
pico_enable_stdio_usb(robot_pico 1)

# Add the standard library and required hardware libraries to the build
target_link_libraries(robot_pico
        pico_stdlib
        hardware_spi
        hardware_gpio
        pico_st7789
)

# Add include directories
target_include_directories(robot_pico PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/include/core
        ${CMAKE_CURRENT_LIST_DIR}/include/display
        ${CMAKE_CURRENT_LIST_DIR}/include/emotions
        ${CMAKE_CURRENT_LIST_DIR}/lib/st7789-library-for-pico-main/src/include
)

pico_add_extra_outputs(robot_pico)

