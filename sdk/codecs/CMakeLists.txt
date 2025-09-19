cmake_minimum_required(VERSION 3.10)
project(linx_codecs C)

# Codec library
set(CODEC_SOURCES
    codec_factory.c
    opus_codec.c
    ../log/linx_log.c
)

set(CODEC_HEADERS
    audio_codec.h
    opus_codec.h
)

# Create codec library
add_library(linx_codecs STATIC ${CODEC_SOURCES})

# Include directories
target_include_directories(linx_codecs PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/opus/include
)

# Find and link Opus library
find_library(OPUS_LIBRARY 
    NAMES opus libopus
    PATHS 
    ${CMAKE_CURRENT_SOURCE_DIR}/opus/.libs
    ${CMAKE_CURRENT_SOURCE_DIR}/opus/build/.libs
    /usr/local/lib
    /opt/homebrew/lib
    /usr/lib
    NO_DEFAULT_PATH
)

if(NOT OPUS_LIBRARY)
    # If not found, try to use the built library in opus directory
    set(OPUS_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/opus/.libs/libopus.a)
    
    # Check if the library exists
    if(NOT EXISTS ${OPUS_LIBRARY})
        message(FATAL_ERROR "Opus library not found. Please build Opus first by running 'make' in the opus directory.")
    endif()
endif()

message(STATUS "Found Opus library: ${OPUS_LIBRARY}")

# Link libraries
target_link_libraries(linx_codecs 
    ${OPUS_LIBRARY}
    m  # Math library for some Opus functions
)

# Compiler flags for C99
target_compile_features(linx_codecs PRIVATE c_std_99)
target_compile_options(linx_codecs PRIVATE 
    -Wall 
    -Wextra 
    -Wno-unused-parameter
    -fPIC
)

# Add definitions
target_compile_definitions(linx_codecs PRIVATE
    OPUS_BUILD
)

# Install headers
install(FILES ${CODEC_HEADERS} 
    DESTINATION include/linx/codecs
)

# Install library
install(TARGETS linx_codecs
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

# Enable testing support
enable_testing()

# Add test subdirectory if it exists
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test)
    add_subdirectory(test)
endif()

# Add custom target for running all tests
add_custom_target(test_all
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    COMMENT "Running all codec tests"
)