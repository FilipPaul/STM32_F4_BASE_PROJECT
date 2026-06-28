cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name to match STM32Make.make target identity
set(CMAKE_PROJECT_NAME F4_WITH_ETHERNET)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Match STM32Make.make output naming convention.
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "$<IF:$<CONFIG:Debug>,F4_WITH_ETHERNET-debug,F4_WITH_ETHERNET-release>"
)

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/commandParser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/EEPROM_24AA02E48T_I_IOT_settings.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/globals.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/I2C_PCA9548APWR_expander.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/IPsettings.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/multiplexers.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/RAFION_bitbang_I2C.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/relayCards.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/RELAYS_AND_IO_DRIVER.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/shiftRegisters.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/tcpecho.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/tcpserver.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/UartMessageHandler.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/UsbMessageHanler.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/VL53L1X_RAFIONTECH.c

)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
