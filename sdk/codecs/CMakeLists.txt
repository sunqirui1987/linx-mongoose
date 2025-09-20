cmake_minimum_required(VERSION 3.10)

# 包含平台检测模块
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/PlatformDetection.cmake)

# 编解码器库基础源文件
set(CODEC_SOURCES
    codec_factory.c
)

set(CODEC_HEADERS
    audio_codec.h
)

# 平台特定的编解码器实现
if(LINX_TARGET_PLATFORM STREQUAL "macos" OR LINX_TARGET_PLATFORM STREQUAL "linux")
    # macOS/Linux - 使用软编码 Opus
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(OPUS REQUIRED opus)
    
    if(OPUS_FOUND)
        list(APPEND CODEC_SOURCES opus_codec.c)
        list(APPEND CODEC_HEADERS opus_codec.h)
        set(CODEC_PLATFORM_LIBS ${OPUS_LIBRARIES})
        set(CODEC_INCLUDE_DIRS ${OPUS_INCLUDE_DIRS})
        message(STATUS "Using Opus software codec for ${LINX_TARGET_PLATFORM}")
        message(STATUS "Opus libraries: ${OPUS_LIBRARIES}")
        message(STATUS "Opus include dirs: ${OPUS_INCLUDE_DIRS}")
    else()
        message(FATAL_ERROR "Opus library not found")
    endif()
    
elseif(LINX_TARGET_PLATFORM STREQUAL "esp32")
    # ESP32 - 可以使用硬件编解码器或软编码
    message(STATUS "ESP32 platform detected")
    # TODO: 根据配置选择硬件或软件编解码器
    # 硬件编解码器选项（如 ES8388）
    # list(APPEND CODEC_SOURCES es8388_codec.c)
    # list(APPEND CODEC_HEADERS es8388_codec.h)
    # set(CODEC_PLATFORM_LIBS "")
    
    # 软编码 Opus 选项（如果 ESP32 支持）
    # find_package(Opus REQUIRED)
    # list(APPEND CODEC_SOURCES opus_codec.c)
    # list(APPEND CODEC_HEADERS opus_codec.h)
    # set(CODEC_PLATFORM_LIBS Opus::opus)
    
    # 当前使用 stub 实现
    list(APPEND CODEC_SOURCES codec_stub.c)
    list(APPEND CODEC_HEADERS codec_stub.h)
    set(CODEC_PLATFORM_LIBS "")
    message(WARNING "ESP32 codec implementation not yet available, using stub")
    
elseif(LINX_TARGET_PLATFORM STREQUAL "windows")
    # Windows - 可以使用 Media Foundation 或软编码
    message(STATUS "Windows platform detected")
    # TODO: 实现 Windows Media Foundation 编解码器
    # list(APPEND CODEC_SOURCES mf_codec.c)
    # list(APPEND CODEC_HEADERS mf_codec.h)
    # set(CODEC_PLATFORM_LIBS mfplat mfreadwrite mfuuid)
    
    # 当前使用软编码 Opus
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(OPUS REQUIRED opus)
    
    if(OPUS_FOUND)
        list(APPEND CODEC_SOURCES opus_codec.c)
        list(APPEND CODEC_HEADERS opus_codec.h)
        set(CODEC_PLATFORM_LIBS ${OPUS_LIBRARIES})
        set(CODEC_INCLUDE_DIRS ${OPUS_INCLUDE_DIRS})
        message(WARNING "Windows native codec not yet available, using Opus")
    else()
        message(FATAL_ERROR "Opus library not found")
    endif()
    
elseif(LINX_TARGET_PLATFORM STREQUAL "android")
    # Android - 可以使用 MediaCodec API 或软编码
    message(STATUS "Android platform detected")
    # TODO: 实现 Android MediaCodec 编解码器
    # list(APPEND CODEC_SOURCES mediacodec_codec.c)
    # list(APPEND CODEC_HEADERS mediacodec_codec.h)
    # set(CODEC_PLATFORM_LIBS mediandk)
    
    # 当前使用 stub 实现
    list(APPEND CODEC_SOURCES codec_stub.c)
    list(APPEND CODEC_HEADERS codec_stub.h)
    set(CODEC_PLATFORM_LIBS "")
    message(WARNING "Android codec implementation not yet available, using stub")
    
else()
    # 未知平台 - 使用 stub 实现
    message(WARNING "Unknown platform: ${LINX_TARGET_PLATFORM}, using stub codec implementation")
    list(APPEND CODEC_SOURCES codec_stub.c)
    list(APPEND CODEC_HEADERS codec_stub.h)
    set(CODEC_PLATFORM_LIBS "")
endif()

# 创建编解码器库
add_library(linx_codecs STATIC ${CODEC_SOURCES})

# 包含目录
target_include_directories(linx_codecs PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 添加平台特定的包含目录
if(CODEC_INCLUDE_DIRS)
    target_include_directories(linx_codecs PRIVATE ${CODEC_INCLUDE_DIRS})
endif()

# 链接平台特定的库
if(CODEC_PLATFORM_LIBS)
    target_link_libraries(linx_codecs 
        PUBLIC ${CODEC_PLATFORM_LIBS}
    )
endif()

# 编译选项
target_compile_features(linx_codecs PRIVATE c_std_99)
target_compile_options(linx_codecs PRIVATE 
    -Wall 
    -Wextra 
    -Wno-unused-parameter
    -fPIC
)

# 编译定义
target_compile_definitions(linx_codecs PRIVATE
    OPUS_BUILD
)

# 安装头文件
install(FILES ${CODEC_HEADERS} 
    DESTINATION include/linx/codecs
)

# 安装库
install(TARGETS linx_codecs
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)