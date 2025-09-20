cmake_minimum_required(VERSION 3.10)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/PlatformDetection.cmake)
# Detect platform
detect_linx_platform()

# 音频库通用源文件
set(AUDIO_SOURCES
    audio_interface.c
)

set(AUDIO_HEADERS
    audio_interface.h
)

# 平台特定的音频实现
if(LINX_TARGET_PLATFORM STREQUAL "macos")
    # PortAudio 实现 (macOS)
    list(APPEND AUDIO_SOURCES portaudio_mac.c)
    list(APPEND AUDIO_HEADERS portaudio_mac.h)
    set(AUDIO_PLATFORM_LIBS portaudio opus)
    set(AUDIO_INCLUDE_DIRS 
        /opt/homebrew/opt/portaudio/include
        /opt/homebrew/opt/opus/include
    )
    set(AUDIO_LIBRARY_DIRS
        /opt/homebrew/opt/portaudio/lib
        /opt/homebrew/opt/opus/lib
    )
    
elseif(LINX_TARGET_PLATFORM STREQUAL "esp32")
    # ESP32 I2S 实现
    # TODO: 添加 ESP32 I2S 音频驱动实现
    # list(APPEND AUDIO_SOURCES esp32_i2s.c)
    # list(APPEND AUDIO_HEADERS esp32_i2s.h)
    # set(AUDIO_PLATFORM_LIBS idf::driver)
    message(WARNING "ESP32 audio implementation not yet available")
    
elseif(LINX_TARGET_PLATFORM STREQUAL "windows")
    # Windows WASAPI 实现
    # TODO: 添加 Windows WASAPI 音频驱动实现
    # list(APPEND AUDIO_SOURCES wasapi_windows.c)
    # list(APPEND AUDIO_HEADERS wasapi_windows.h)
    # set(AUDIO_PLATFORM_LIBS winmm)
    message(WARNING "Windows audio implementation not yet available")
    
elseif(LINX_TARGET_PLATFORM STREQUAL "android")
    # Android OpenSL ES 实现
    # TODO: 添加 Android OpenSL ES 音频驱动实现
    # list(APPEND AUDIO_SOURCES opensles_android.c)
    # list(APPEND AUDIO_HEADERS opensles_android.h)
    # set(AUDIO_PLATFORM_LIBS OpenSLES)
    message(WARNING "Android audio implementation not yet available")
    
else()
    # 未知平台 - 使用空实现
    message(WARNING "Unknown platform: ${LINX_TARGET_PLATFORM}, using stub audio implementation")
    list(APPEND AUDIO_SOURCES audio_stub.c)
    list(APPEND AUDIO_HEADERS audio_stub.h)
    set(AUDIO_PLATFORM_LIBS "")
endif()

# 创建音频库
add_library(linx_audio STATIC ${AUDIO_SOURCES})

# 包含目录
target_include_directories(linx_audio PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 添加平台特定的包含目录
if(AUDIO_INCLUDE_DIRS)
    target_include_directories(linx_audio PUBLIC ${AUDIO_INCLUDE_DIRS})
endif()

# 添加平台特定的库目录
if(AUDIO_LIBRARY_DIRS)
    target_link_directories(linx_audio PUBLIC ${AUDIO_LIBRARY_DIRS})
endif()

# 链接平台特定的库
if(AUDIO_PLATFORM_LIBS)
    target_link_libraries(linx_audio 
        PUBLIC ${AUDIO_PLATFORM_LIBS}
    )
endif()

# 编译选项
target_compile_options(linx_audio PRIVATE -Wall -Wextra)

# 安装头文件
install(FILES ${AUDIO_HEADERS} DESTINATION include/linx/audio)

# 安装库
install(TARGETS linx_audio DESTINATION lib)