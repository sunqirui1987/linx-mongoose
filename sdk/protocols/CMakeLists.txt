cmake_minimum_required(VERSION 3.10)

# 查找 Mongoose（使用统一的查找模块）
find_package(Mongoose REQUIRED)

# 协议库源文件
set(PROTOCOLS_SOURCES
    linx_protocol.c
    linx_websocket.c
)

set(PROTOCOLS_HEADERS
    linx_protocol.h
    linx_websocket.h
)

# 创建协议库
add_library(linx_protocols STATIC ${PROTOCOLS_SOURCES})

# 包含目录
target_include_directories(linx_protocols PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../cjson
    ${CMAKE_CURRENT_SOURCE_DIR}/../log
)

# 编译选项
target_compile_features(linx_protocols PRIVATE c_std_99)
target_compile_options(linx_protocols PRIVATE 
    -Wall 
    -Wextra 
    -Wno-unused-parameter
)

# 链接依赖
target_link_libraries(linx_protocols
    PUBLIC
        Mongoose::mongoose
        cjson_static
        linx_log
)

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