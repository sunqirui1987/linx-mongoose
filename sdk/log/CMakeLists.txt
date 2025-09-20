cmake_minimum_required(VERSION 3.10)
project(linx_log C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Log library sources
set(LOG_SOURCES
    linx_log.c
)

set(LOG_HEADERS
    linx_log.h
)

# Create log library
add_library(linx_log STATIC ${LOG_SOURCES})

# Include directories
target_include_directories(linx_log PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler flags
target_compile_options(linx_log PRIVATE 
    -Wall 
    -Wextra 
    -Wno-unused-parameter
)

# For thread-safe logging, link pthread if available
find_package(Threads)
if(Threads_FOUND)
    target_link_libraries(linx_log ${CMAKE_THREAD_LIBS_INIT})
endif()

# Install headers
install(FILES ${LOG_HEADERS} DESTINATION include/linx/log)

# Install library
install(TARGETS linx_log DESTINATION lib)

# Set library properties
set_target_properties(linx_log PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
    PUBLIC_HEADER "${LOG_HEADERS}"
)