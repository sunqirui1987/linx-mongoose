cmake_minimum_required(VERSION 3.10)
project(linx_sdk VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -DNDEBUG")

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

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

# Link all component libraries
target_link_libraries(linx_sdk
    cjson_static
    linx_audio
    linx_codecs
    linx_log
    linx_mcp
    linx_protocols
)

# Install headers
install(FILES ${SDK_HEADERS} DESTINATION include/linx)

# Install library
install(TARGETS linx_sdk DESTINATION lib)

# Create a combined library that includes all components
add_library(linx_sdk_combined STATIC 
    ${SDK_SOURCES}
    $<TARGET_OBJECTS:cjson_static>
    $<TARGET_OBJECTS:linx_audio>
    $<TARGET_OBJECTS:linx_codecs>
    $<TARGET_OBJECTS:linx_log>
    $<TARGET_OBJECTS:linx_mcp>
    $<TARGET_OBJECTS:linx_protocols>
)

# Set properties for combined library
set_target_properties(linx_sdk_combined PROPERTIES OUTPUT_NAME linx_sdk_all)

# Install combined library
install(TARGETS linx_sdk_combined DESTINATION lib)