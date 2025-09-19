cmake_minimum_required(VERSION 3.10)

# Find PortAudio
find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)

# Audio library
set(AUDIO_SOURCES
    audio_interface.c
    portaudio_mac.c
)

set(AUDIO_HEADERS
    audio_interface.h
    portaudio_mac.h
)

# Create audio library
add_library(linx_audio STATIC ${AUDIO_SOURCES})

# Include directories
target_include_directories(linx_audio PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PORTAUDIO_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(linx_audio 
    ${PORTAUDIO_LIBRARIES}
    pthread
)

# Compiler flags
target_compile_options(linx_audio PRIVATE ${PORTAUDIO_CFLAGS_OTHER})

# For macOS, we might need to find PortAudio via Homebrew
if(APPLE)
    # Try to find PortAudio via Homebrew
    find_path(PORTAUDIO_INCLUDE_DIR portaudio.h
        PATHS
        /usr/local/include
        /opt/homebrew/include
        /usr/include
    )
    
    find_library(PORTAUDIO_LIBRARY portaudio
        PATHS
        /usr/local/lib
        /opt/homebrew/lib
        /usr/lib
    )
    
    if(PORTAUDIO_INCLUDE_DIR AND PORTAUDIO_LIBRARY)
        target_include_directories(linx_audio PUBLIC ${PORTAUDIO_INCLUDE_DIR})
        target_link_libraries(linx_audio ${PORTAUDIO_LIBRARY})
        
        # macOS specific frameworks
        target_link_libraries(linx_audio 
            "-framework CoreAudio"
            "-framework AudioToolbox"
            "-framework AudioUnit"
            "-framework CoreServices"
        )
    else()
        message(WARNING "PortAudio not found. Please install via: brew install portaudio")
    endif()
endif()

# Install headers
install(FILES ${AUDIO_HEADERS} DESTINATION include/linx/audio)

# Install library
install(TARGETS linx_audio DESTINATION lib)