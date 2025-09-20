# Platform Detection Module for Linx SDK
# This module detects the target platform and sets appropriate variables

# Initialize platform variables
set(LINX_PLATFORM_DETECTED FALSE)
set(LINX_PLATFORM_NAME "unknown")
set(LINX_PLATFORM_TYPE "unknown")
set(LINX_IS_EMBEDDED FALSE)
set(LINX_IS_DESKTOP FALSE)

# Function to detect platform
function(detect_linx_platform)
    # Check if platform is explicitly set via command line
    if(DEFINED LINX_TARGET_PLATFORM)
        set(LINX_PLATFORM_NAME ${LINX_TARGET_PLATFORM} PARENT_SCOPE)
        message(STATUS "Platform explicitly set to: ${LINX_TARGET_PLATFORM}")
        
        # Set platform type based on explicit setting
        if(LINX_TARGET_PLATFORM MATCHES "^(esp32|esp8266|stm32|arduino|rpi_pico)$")
            set(LINX_PLATFORM_TYPE "embedded" PARENT_SCOPE)
            set(LINX_IS_EMBEDDED TRUE PARENT_SCOPE)
        elseif(LINX_TARGET_PLATFORM MATCHES "^(macos|linux|windows)$")
            set(LINX_PLATFORM_TYPE "desktop" PARENT_SCOPE)
            set(LINX_IS_DESKTOP TRUE PARENT_SCOPE)
        endif()
        
        set(LINX_PLATFORM_DETECTED TRUE PARENT_SCOPE)
        return()
    endif()

    # Auto-detect platform based on CMAKE variables
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(LINX_PLATFORM_NAME "macos" PARENT_SCOPE)
        set(LINX_PLATFORM_TYPE "desktop" PARENT_SCOPE)
        set(LINX_IS_DESKTOP TRUE PARENT_SCOPE)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        # Check if it's a cross-compilation for embedded systems
        if(CMAKE_CROSSCOMPILING)
            if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|aarch64|xtensa)$")
                set(LINX_PLATFORM_NAME "embedded_linux" PARENT_SCOPE)
                set(LINX_PLATFORM_TYPE "embedded" PARENT_SCOPE)
                set(LINX_IS_EMBEDDED TRUE PARENT_SCOPE)
            else()
                set(LINX_PLATFORM_NAME "linux" PARENT_SCOPE)
                set(LINX_PLATFORM_TYPE "desktop" PARENT_SCOPE)
                set(LINX_IS_DESKTOP TRUE PARENT_SCOPE)
            endif()
        else()
            set(LINX_PLATFORM_NAME "linux" PARENT_SCOPE)
            set(LINX_PLATFORM_TYPE "desktop" PARENT_SCOPE)
            set(LINX_IS_DESKTOP TRUE PARENT_SCOPE)
        endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(LINX_PLATFORM_NAME "windows" PARENT_SCOPE)
        set(LINX_PLATFORM_TYPE "desktop" PARENT_SCOPE)
        set(LINX_IS_DESKTOP TRUE PARENT_SCOPE)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Generic")
        # This usually indicates embedded systems
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "xtensa")
            set(LINX_PLATFORM_NAME "esp32" PARENT_SCOPE)
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
            set(LINX_PLATFORM_NAME "arm_embedded" PARENT_SCOPE)
        else()
            set(LINX_PLATFORM_NAME "generic_embedded" PARENT_SCOPE)
        endif()
        set(LINX_PLATFORM_TYPE "embedded" PARENT_SCOPE)
        set(LINX_IS_EMBEDDED TRUE PARENT_SCOPE)
    endif()

    # Set LINX_TARGET_PLATFORM for compatibility with other modules
    set(LINX_TARGET_PLATFORM ${LINX_PLATFORM_NAME} PARENT_SCOPE)
    
    set(LINX_PLATFORM_DETECTED TRUE PARENT_SCOPE)
endfunction()

# Function to load platform-specific configuration
function(load_platform_config)
    if(NOT LINX_PLATFORM_DETECTED)
        message(FATAL_ERROR "Platform not detected. Call detect_linx_platform() first.")
    endif()

    set(PLATFORM_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/platforms/${LINX_PLATFORM_NAME}.cmake")
    
    if(EXISTS ${PLATFORM_CONFIG_FILE})
        message(STATUS "Loading platform config: ${PLATFORM_CONFIG_FILE}")
        include(${PLATFORM_CONFIG_FILE})
    else()
        message(WARNING "Platform config file not found: ${PLATFORM_CONFIG_FILE}")
        message(STATUS "Using default configuration for platform: ${LINX_PLATFORM_NAME}")
    endif()
endfunction()

# Function to print platform information
function(print_platform_info)
    message(STATUS "=== Linx SDK Platform Information ===")
    message(STATUS "Platform Name: ${LINX_PLATFORM_NAME}")
    message(STATUS "Platform Type: ${LINX_PLATFORM_TYPE}")
    message(STATUS "Is Embedded: ${LINX_IS_EMBEDDED}")
    message(STATUS "Is Desktop: ${LINX_IS_DESKTOP}")
    message(STATUS "System Name: ${CMAKE_SYSTEM_NAME}")
    message(STATUS "System Processor: ${CMAKE_SYSTEM_PROCESSOR}")
    message(STATUS "Cross Compiling: ${CMAKE_CROSSCOMPILING}")
    message(STATUS "=====================================")
endfunction()

# Auto-detect platform when this module is included
detect_linx_platform()