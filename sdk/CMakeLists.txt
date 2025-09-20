cmake_minimum_required(VERSION 3.10)
project(linx_sdk VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add cmake modules path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include platform detection
include(PlatformDetection)

# Print platform information
print_platform_info()

# Load platform-specific configuration
load_platform_config()

# Default compiler flags (can be overridden by platform configs)
if(NOT CMAKE_C_FLAGS)
    set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-parameter")
endif()

# Default debug and release flags
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -DDEBUG")
if(LINX_IS_EMBEDDED)
    # Optimize for size on embedded platforms
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os -DNDEBUG")
else()
    # Optimize for speed on desktop platforms
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -DNDEBUG")
endif()

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Global definitions
add_definitions(-DLINX_SDK_VERSION_MAJOR=1)
add_definitions(-DLINX_SDK_VERSION_MINOR=0)
add_definitions(-DLINX_SDK_VERSION_PATCH=0)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Add subdirectories (excluding test directories)
add_subdirectory(cjson)
add_subdirectory(audio)
add_subdirectory(codecs)
add_subdirectory(log)
add_subdirectory(mcp)
add_subdirectory(protocols)

# Main SDK library sources
set(SDK_SOURCES
    linx_sdk.c
)

set(SDK_HEADERS
    linx_sdk.h
)

# Create main SDK library
add_library(linx_sdk STATIC ${SDK_SOURCES})

# Include directories for SDK
target_include_directories(linx_sdk PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/cjson
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs
    ${CMAKE_CURRENT_SOURCE_DIR}/log
    ${CMAKE_CURRENT_SOURCE_DIR}/mcp
    ${CMAKE_CURRENT_SOURCE_DIR}/protocols
)

# Link component libraries based on platform capabilities
set(LINX_COMPONENT_LIBRARIES
    cjson_static
    linx_log
    linx_mcp
    linx_protocols
)

# Add audio and codecs only if enabled for this platform
if(LINX_ENABLE_AUDIO)
    list(APPEND LINX_COMPONENT_LIBRARIES linx_audio linx_codecs)
    message(STATUS "Audio support enabled")
else()
    message(STATUS "Audio support disabled for this platform")
endif()

# Link all component libraries
target_link_libraries(linx_sdk ${LINX_COMPONENT_LIBRARIES})

# Link platform-specific system libraries
if(LINX_SYSTEM_LIBRARIES)
    target_link_libraries(linx_sdk ${LINX_SYSTEM_LIBRARIES})
    message(STATUS "Linking system libraries: ${LINX_SYSTEM_LIBRARIES}")
endif()

# Link platform-specific audio libraries
if(LINX_ENABLE_AUDIO)
    if(LINX_PORTAUDIO_FOUND AND LINX_PORTAUDIO_LIBRARIES)
        target_link_libraries(linx_sdk ${LINX_PORTAUDIO_LIBRARIES})
        target_include_directories(linx_sdk PRIVATE ${LINX_PORTAUDIO_INCLUDE_DIRS})
        message(STATUS "Linking PortAudio: ${LINX_PORTAUDIO_LIBRARIES}")
    endif()
    
    if(LINX_OPUS_FOUND AND LINX_OPUS_LIBRARIES)
        target_link_libraries(linx_sdk ${LINX_OPUS_LIBRARIES})
        target_include_directories(linx_sdk PRIVATE ${LINX_OPUS_INCLUDE_DIRS})
        message(STATUS "Linking Opus: ${LINX_OPUS_LIBRARIES}")
    endif()
    
    # Link macOS frameworks if available
    if(LINX_MACOS_FRAMEWORKS)
        target_link_libraries(linx_sdk ${LINX_MACOS_FRAMEWORKS})
        message(STATUS "Linking macOS frameworks: ${LINX_MACOS_FRAMEWORKS}")
    endif()
endif()

# Install headers
install(FILES ${SDK_HEADERS} DESTINATION include/linx)

# Install library
install(TARGETS linx_sdk DESTINATION lib)

# Create a combined library that includes all enabled components
set(COMBINED_OBJECTS
    ${SDK_SOURCES}
    $<TARGET_OBJECTS:cjson_static>
    $<TARGET_OBJECTS:linx_log>
    $<TARGET_OBJECTS:linx_mcp>
    $<TARGET_OBJECTS:linx_protocols>
)

# Add audio and codec objects only if enabled
if(LINX_ENABLE_AUDIO)
    list(APPEND COMBINED_OBJECTS 
        $<TARGET_OBJECTS:linx_audio>
        $<TARGET_OBJECTS:linx_codecs>
    )
endif()

add_library(linx_sdk_combined STATIC ${COMBINED_OBJECTS})

# Set properties for combined library
set_target_properties(linx_sdk_combined PROPERTIES OUTPUT_NAME linx_sdk_all)

# Link the same libraries as the main SDK
target_link_libraries(linx_sdk_combined ${LINX_COMPONENT_LIBRARIES})

if(LINX_SYSTEM_LIBRARIES)
    target_link_libraries(linx_sdk_combined ${LINX_SYSTEM_LIBRARIES})
endif()

if(LINX_ENABLE_AUDIO)
    if(LINX_PORTAUDIO_FOUND AND LINX_PORTAUDIO_LIBRARIES)
        target_link_libraries(linx_sdk_combined ${LINX_PORTAUDIO_LIBRARIES})
    endif()
    
    if(LINX_OPUS_FOUND AND LINX_OPUS_LIBRARIES)
        target_link_libraries(linx_sdk_combined ${LINX_OPUS_LIBRARIES})
    endif()
    
    if(LINX_MACOS_FRAMEWORKS)
        target_link_libraries(linx_sdk_combined ${LINX_MACOS_FRAMEWORKS})
    endif()
endif()

# Install combined library
install(TARGETS linx_sdk_combined DESTINATION lib)

# Print build summary
message(STATUS "=== Linx SDK Build Summary ===")
message(STATUS "Platform: ${LINX_PLATFORM_NAME} (${LINX_PLATFORM_TYPE})")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Audio Support: ${LINX_ENABLE_AUDIO}")
message(STATUS "WebSocket Support: ${LINX_ENABLE_WEBSOCKET}")
message(STATUS "MCP Support: ${LINX_ENABLE_MCP}")
if(LINX_MISSING_DEPENDENCIES)
    message(WARNING "Some dependencies are missing. Build may fail.")
endif()
message(STATUS "===============================")