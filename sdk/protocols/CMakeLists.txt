cmake_minimum_required(VERSION 3.10)
project(linx_protocols C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Protocols library sources
set(PROTOCOLS_SOURCES
    linx_protocol.c
    linx_websocket.c
)

set(PROTOCOLS_HEADERS
    linx_protocol.h
    linx_websocket.h
)

# Create protocols library
add_library(linx_protocols STATIC ${PROTOCOLS_SOURCES})

# Include directories
target_include_directories(linx_protocols PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../cjson
    ${CMAKE_CURRENT_SOURCE_DIR}/../log
)

# Compiler flags
target_compile_options(linx_protocols PRIVATE 
    -Wall 
    -Wextra 
    -Wno-unused-parameter
)

# Link dependencies
target_link_libraries(linx_protocols
    cjson_static
    linx_log
)

# For networking and threading, link required libraries
find_package(Threads)
if(Threads_FOUND)
    target_link_libraries(linx_protocols ${CMAKE_THREAD_LIBS_INIT})
endif()

# Install headers
install(FILES ${PROTOCOLS_HEADERS} DESTINATION include/linx/protocols)

# Install library
install(TARGETS linx_protocols DESTINATION lib)

# Set library properties
set_target_properties(linx_protocols PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
    PUBLIC_HEADER "${PROTOCOLS_HEADERS}"
)