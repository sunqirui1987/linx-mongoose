cmake_minimum_required(VERSION 3.10)
project(linx_sdk C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include platform detection (auto-detects platform)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PlatformDetection.cmake)

# Include find modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)



# =============================================================================
# Add subdirectories to build individual libraries
# =============================================================================

# Add subdirectories in dependency order
add_subdirectory(log)
add_subdirectory(cjson)
add_subdirectory(mcp)
add_subdirectory(protocols)
add_subdirectory(codecs)
add_subdirectory(audio)

# =============================================================================
# Create unified static library
# =============================================================================

# Main SDK sources (only the top-level files)
set(LINX_SDK_SOURCES
    linx_sdk.c
)

# Create the unified static library
add_library(linx_sdk_static STATIC ${LINX_SDK_SOURCES})

# Set target properties
set_target_properties(linx_sdk_static PROPERTIES
    OUTPUT_NAME linx_sdk
    VERSION 1.0.0
    SOVERSION 1
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)




# Link all the subdirectory libraries into the unified library
target_link_libraries(linx_sdk_static PUBLIC
    linx_log
    cjson_static
    linx_mcp
    linx_protocols
    linx_codecs
    linx_audio
)

# =============================================================================
# Installation configuration
# =============================================================================

# Set installation directory to build/install
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "Installation directory" FORCE)

# Install the static library
install(TARGETS linx_sdk_static
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install the main header file
install(FILES linx_sdk.h
    DESTINATION include
)

# Install subdirectory headers
install(DIRECTORY log/ 
    DESTINATION include/log
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY cjson/
    DESTINATION include/cjson  
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY mcp/
    DESTINATION include/mcp
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY protocols/
    DESTINATION include/protocols
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY codecs/
    DESTINATION include/codecs
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY audio/
    DESTINATION include/audio
    FILES_MATCHING PATTERN "*.h"
)
