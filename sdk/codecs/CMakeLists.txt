cmake_minimum_required(VERSION 3.10)

# 查找 Opus（使用统一的查找模块）
find_package(Opus REQUIRED)

# 编解码器库源文件
set(CODEC_SOURCES
    opus_codec.c
    codec_factory.c
)

set(CODEC_HEADERS
    opus_codec.h
    codec_factory.h
)

# 创建编解码器库
add_library(linx_codecs STATIC ${CODEC_SOURCES})

# 包含目录
target_include_directories(linx_codecs PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接库
target_link_libraries(linx_codecs 
    PUBLIC Opus::opus
)

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