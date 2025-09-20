cmake_minimum_required(VERSION 3.10)

# 查找 PortAudio（使用统一的查找模块）
find_package(PortAudio REQUIRED)

# 音频库源文件
set(AUDIO_SOURCES
    audio_interface.c
)

set(AUDIO_HEADERS
    audio_interface.h
)

# 根据平台添加特定源文件
if(LINX_TARGET_PLATFORM STREQUAL "macos" OR LINX_TARGET_PLATFORM STREQUAL "linux")
    list(APPEND AUDIO_SOURCES portaudio_mac.c)
    list(APPEND AUDIO_HEADERS portaudio_mac.h)
endif()

# 创建音频库
add_library(linx_audio STATIC ${AUDIO_SOURCES})

# 包含目录
target_include_directories(linx_audio PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接库
target_link_libraries(linx_audio 
    PUBLIC PortAudio::portaudio
)

# 编译选项
target_compile_options(linx_audio PRIVATE -Wall -Wextra)

# 安装头文件
install(FILES ${AUDIO_HEADERS} DESTINATION include/linx/audio)

# 安装库
install(TARGETS linx_audio DESTINATION lib)