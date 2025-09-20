cmake_minimum_required(VERSION 3.10)
project(linx_mcp C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# MCP library sources
set(MCP_SOURCES
    mcp_property.c
    mcp_server.c
    mcp_tool.c
    mcp_utils.c
)

set(MCP_HEADERS
    mcp.h
    mcp_property.h
    mcp_server.h
    mcp_tool.h
    mcp_types.h
    mcp_utils.h
)

# Create MCP library
add_library(linx_mcp STATIC ${MCP_SOURCES})

# Include directories
target_include_directories(linx_mcp PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../cjson
    ${CMAKE_CURRENT_SOURCE_DIR}/../log
)

# Compiler flags
target_compile_options(linx_mcp PRIVATE 
    -Wall 
    -Wextra 
    -Wno-unused-parameter
)

# Link dependencies
target_link_libraries(linx_mcp
    cjson_static
    linx_log
)

# Note: As a static library, we don't need to link platform-specific libraries here.
# These will be linked when the final executable or shared library is built.

# Install headers
install(FILES ${MCP_HEADERS} DESTINATION include/linx/mcp)

# Install library
install(TARGETS linx_mcp DESTINATION lib)

# Set library properties
set_target_properties(linx_mcp PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
    PUBLIC_HEADER "${MCP_HEADERS}"
)