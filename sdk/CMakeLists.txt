cmake_minimum_required(VERSION 3.10)
project(linx_sdk C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include platform detection
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PlatformDetection.cmake)
detect_linx_platform()

# Include find modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Find required dependencies
find_package(Mongoose REQUIRED)
find_package(Threads REQUIRED)

# Platform-specific audio dependencies
if(LINX_TARGET_PLATFORM STREQUAL "macos" OR LINX_TARGET_PLATFORM STREQUAL "linux")
    find_package(PortAudio REQUIRED)
    find_package(Opus REQUIRED)
endif()

# =============================================================================
# Collect all source files from subdirectories
# =============================================================================

# Main SDK sources
set(LINX_SDK_SOURCES
    linx_sdk.c
)

# Log module sources
set(LOG_SOURCES
    log/linx_log.c
)

# cJSON module sources  
set(CJSON_SOURCES
    cjson/cJSON.c
    cjson/cJSON_Utils.c
)

# MCP module sources
set(MCP_SOURCES
    mcp/mcp_property.c
    mcp/mcp_server.c
    mcp/mcp_tool.c
    mcp/mcp_utils.c
)

# Protocols module sources
set(PROTOCOLS_SOURCES
    protocols/linx_protocol.c
    protocols/linx_websocket.c
)

# Codecs module sources
set(CODEC_SOURCES
    codecs/codec_factory.c
)

# Audio module sources
set(AUDIO_SOURCES
    audio/audio_interface.c
)

# Platform-specific sources
if(LINX_TARGET_PLATFORM STREQUAL "macos" OR LINX_TARGET_PLATFORM STREQUAL "linux")
    # PortAudio implementation
    list(APPEND AUDIO_SOURCES audio/portaudio_mac.c)
    # Opus codec implementation
    list(APPEND CODEC_SOURCES codecs/opus_codec.c)
elseif(LINX_TARGET_PLATFORM STREQUAL "esp32")
    # ESP32 implementations (currently using stubs)
    list(APPEND AUDIO_SOURCES audio/audio_stub.c)
    list(APPEND CODEC_SOURCES codecs/codec_stub.c)
else()
    # Unknown platform - use stub implementations
    list(APPEND AUDIO_SOURCES audio/audio_stub.c)
    list(APPEND CODEC_SOURCES codecs/codec_stub.c)
endif()

# Add mongoose sources if found
if(Mongoose_FOUND)
    set(MONGOOSE_SOURCES ${Mongoose_SOURCES})
endif()

# Combine all sources
set(ALL_SOURCES
    ${LINX_SDK_SOURCES}
    ${LOG_SOURCES}
    ${CJSON_SOURCES}
    ${MCP_SOURCES}
    ${PROTOCOLS_SOURCES}
    ${CODEC_SOURCES}
    ${AUDIO_SOURCES}
    ${MONGOOSE_SOURCES}
)

# =============================================================================
# Collect all header files for installation
# =============================================================================

set(LINX_SDK_HEADERS
    linx_sdk.h
)

set(LOG_HEADERS
    log/linx_log.h
)

set(CJSON_HEADERS
    cjson/cJSON.h
    cjson/cJSON_Utils.h
)

set(MCP_HEADERS
    mcp/mcp.h
    mcp/mcp_property.h
    mcp/mcp_server.h
    mcp/mcp_tool.h
    mcp/mcp_types.h
    mcp/mcp_utils.h
)

set(PROTOCOLS_HEADERS
    protocols/linx_protocol.h
    protocols/linx_websocket.h
)

set(CODEC_HEADERS
    codecs/audio_codec.h
    codecs/codec_stub.h
    codecs/opus_codec.h
    codecs/es8388_codec.h
)

set(AUDIO_HEADERS
    audio/audio_interface.h
    audio/audio_stub.h
    audio/portaudio_mac.h
    audio/esp32_i2s.h
)

# =============================================================================
# Create unified static library
# =============================================================================

add_library(linx_sdk_static STATIC ${ALL_SOURCES})

# Set target properties
set_target_properties(linx_sdk_static PROPERTIES
    OUTPUT_NAME linx_sdk
    VERSION 1.0.0
    SOVERSION 1
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Include directories
target_include_directories(linx_sdk_static PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/log
    ${CMAKE_CURRENT_SOURCE_DIR}/cjson
    ${CMAKE_CURRENT_SOURCE_DIR}/mcp
    ${CMAKE_CURRENT_SOURCE_DIR}/protocols
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs
    ${CMAKE_CURRENT_SOURCE_DIR}/audio
)

# Add mongoose include directories if found
if(Mongoose_FOUND)
    target_include_directories(linx_sdk_static PUBLIC ${Mongoose_INCLUDE_DIRS})
endif()

# Platform-specific include directories
if(LINX_TARGET_PLATFORM STREQUAL "macos" OR LINX_TARGET_PLATFORM STREQUAL "linux")
    if(OPUS_FOUND)
        target_include_directories(linx_sdk_static PUBLIC ${OPUS_INCLUDE_DIRS})
    endif()
endif()

# Compiler definitions
target_compile_definitions(linx_sdk_static PRIVATE
    CJSON_HIDE_SYMBOLS
    LINX_SDK_VERSION="1.0.0"
)

# Compiler flags
target_compile_options(linx_sdk_static PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
    -fPIC
)

# Debug/Release specific flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(linx_sdk_static PRIVATE -g -O0)
else()
    target_compile_options(linx_sdk_static PRIVATE -O2)
endif()

# Link libraries
target_link_libraries(linx_sdk_static PUBLIC
    ${CMAKE_THREAD_LIBS_INIT}
)

# Link mongoose if found (it's a single-file library, so no actual linking needed)
if(Mongoose_FOUND AND TARGET Mongoose::Mongoose)
    target_link_libraries(linx_sdk_static PUBLIC Mongoose::Mongoose)
endif()

# Platform-specific libraries
if(LINX_TARGET_PLATFORM STREQUAL "macos" OR LINX_TARGET_PLATFORM STREQUAL "linux")
    if(TARGET PortAudio::portaudio)
        target_link_libraries(linx_sdk_static PUBLIC PortAudio::portaudio)
    endif()
    if(OPUS_FOUND)
        target_link_libraries(linx_sdk_static PUBLIC ${OPUS_LIBRARIES})
    endif()
endif()

# =============================================================================
# Install configuration
# =============================================================================

# Install the static library
install(TARGETS linx_sdk_static
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install headers with directory structure
install(FILES ${LINX_SDK_HEADERS} DESTINATION include/linx)
install(FILES ${LOG_HEADERS} DESTINATION include/linx/log)
install(FILES ${CJSON_HEADERS} DESTINATION include/linx/cjson)
install(FILES ${MCP_HEADERS} DESTINATION include/linx/mcp)
install(FILES ${PROTOCOLS_HEADERS} DESTINATION include/linx/protocols)
install(FILES ${CODEC_HEADERS} DESTINATION include/linx/codecs)
install(FILES ${AUDIO_HEADERS} DESTINATION include/linx/audio)

# Create pkg-config file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/linx_sdk.pc.in
    ${CMAKE_BINARY_DIR}/linx_sdk.pc
    @ONLY
)

install(FILES ${CMAKE_BINARY_DIR}/linx_sdk.pc DESTINATION lib/pkgconfig)

# =============================================================================
# Print configuration summary
# =============================================================================

message(STATUS "=== Linx SDK Configuration Summary ===")
message(STATUS "Platform: ${LINX_PLATFORM_NAME}")
message(STATUS "Platform Type: ${LINX_PLATFORM_TYPE}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Standard: ${CMAKE_C_STANDARD}")
message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Library Output: ${CMAKE_BINARY_DIR}/lib/liblinx_sdk.a")

if(LINX_TARGET_PLATFORM STREQUAL "macos" OR LINX_TARGET_PLATFORM STREQUAL "linux")
    message(STATUS "Audio Backend: PortAudio")
    message(STATUS "Codec Backend: Opus")
    if(OPUS_FOUND)
        message(STATUS "Opus Libraries: ${OPUS_LIBRARIES}")
    endif()
else()
    message(STATUS "Audio Backend: Stub (${LINX_TARGET_PLATFORM})")
    message(STATUS "Codec Backend: Stub (${LINX_TARGET_PLATFORM})")
endif()

message(STATUS "======================================")