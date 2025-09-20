# cJSON Library CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# Set library name
set(CJSON_LIB_NAME cjson_static)

# Source files
set(CJSON_SOURCES
    cJSON.c
    cJSON_Utils.c
)

# Header files
set(CJSON_HEADERS
    cJSON.h
    cJSON_Utils.h
)

# Create object library (for use in combined library)
add_library(${CJSON_LIB_NAME} OBJECT ${CJSON_SOURCES})

# Set target properties
set_target_properties(${CJSON_LIB_NAME} PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    VERSION 1.7.16
    SOVERSION 1
)

# Include directories
target_include_directories(${CJSON_LIB_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler definitions
target_compile_definitions(${CJSON_LIB_NAME} PRIVATE
    CJSON_HIDE_SYMBOLS
)

# Compiler flags
target_compile_options(${CJSON_LIB_NAME} PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
    -fPIC
)

# Debug/Release specific flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${CJSON_LIB_NAME} PRIVATE -g -O0)
    target_compile_definitions(${CJSON_LIB_NAME} PRIVATE DEBUG)
else()
    target_compile_options(${CJSON_LIB_NAME} PRIVATE -O2)
    target_compile_definitions(${CJSON_LIB_NAME} PRIVATE NDEBUG)
endif()

# Install rules (optional)
install(TARGETS ${CJSON_LIB_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${CJSON_HEADERS}
    DESTINATION include/cjson
)

# Export library for use by other parts of the project
set(CJSON_LIBRARIES ${CJSON_LIB_NAME} PARENT_SCOPE)
set(CJSON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)