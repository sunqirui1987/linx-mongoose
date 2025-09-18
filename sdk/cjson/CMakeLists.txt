# cJSON Library CMakeLists.txt
# JSON parsing library for C
# Author: SiFli Technologies
# Date: 2024-2025

cmake_minimum_required(VERSION 3.10)

# 设置项目名称和版本
project(cjson VERSION 1.7.15 LANGUAGES C)

# 设置 C 标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter")

# Debug 模式下的编译选项
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -DDEBUG")

# Release 模式下的编译选项
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -DNDEBUG")

# 定义源文件
set(CJSON_SOURCES
    cJSON.c
    cJSON_Utils.c
)

# 定义头文件
set(CJSON_HEADERS
    cJSON.h
    cJSON_Utils.h
)

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 创建静态库
add_library(cjson_static STATIC ${CJSON_SOURCES})
target_include_directories(cjson_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 创建动态库
add_library(cjson_shared SHARED ${CJSON_SOURCES})
target_include_directories(cjson_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 设置库的输出名称
set_target_properties(cjson_static PROPERTIES OUTPUT_NAME cjson)
set_target_properties(cjson_shared PROPERTIES OUTPUT_NAME cjson)

# 设置动态库版本信息
set_target_properties(cjson_shared PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# 编译定义
target_compile_definitions(cjson_static PRIVATE
    CJSON_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    CJSON_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    CJSON_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

target_compile_definitions(cjson_shared PRIVATE
    CJSON_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    CJSON_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    CJSON_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    CJSON_EXPORT_SYMBOLS
)

# 链接数学库（如果需要）
find_library(MATH_LIB m)
if(MATH_LIB)
    target_link_libraries(cjson_static ${MATH_LIB})
    target_link_libraries(cjson_shared ${MATH_LIB})
endif()

# 配置选项
option(ENABLE_CJSON_TEST "Enable cJSON test programs" OFF)
option(ENABLE_CJSON_UTILS "Enable cJSON_Utils" ON)
option(ENABLE_CJSON_VERSION_SO "Enable cJSON version in shared object" ON)

# 如果禁用 cJSON_Utils，则从构建中移除
if(NOT ENABLE_CJSON_UTILS)
    list(REMOVE_ITEM CJSON_SOURCES cJSON_Utils.c)
    list(REMOVE_ITEM CJSON_HEADERS cJSON_Utils.h)
    target_compile_definitions(cjson_static PRIVATE CJSON_DISABLE_UTILS)
    target_compile_definitions(cjson_shared PRIVATE CJSON_DISABLE_UTILS)
endif()

# 安装配置
install(TARGETS cjson_static cjson_shared
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${CJSON_HEADERS}
    DESTINATION include/cjson
)

# 创建测试程序（可选）
if(ENABLE_CJSON_TEST)
    enable_testing()
    
    # 基本测试
    add_executable(cjson_test tests/test_basic.c)
    target_link_libraries(cjson_test cjson_static)
    add_test(NAME BasicTest COMMAND cjson_test)
    
    # 性能测试
    add_executable(cjson_performance_test tests/test_performance.c)
    target_link_libraries(cjson_performance_test cjson_static)
    add_test(NAME PerformanceTest COMMAND cjson_performance_test)
    
    if(ENABLE_CJSON_UTILS)
        # Utils 测试
        add_executable(cjson_utils_test tests/test_utils.c)
        target_link_libraries(cjson_utils_test cjson_static)
        add_test(NAME UtilsTest COMMAND cjson_utils_test)
    endif()
endif()

# 生成 pkg-config 文件
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/libcjson.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/libcjson.pc
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libcjson.pc
    DESTINATION lib/pkgconfig
)

# 创建 cJSON 配置头文件
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cjson_config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/cjson_config.h
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cjson_config.h
    DESTINATION include/cjson
)

# 别名目标（为了兼容性）
add_library(cjson::cjson_static ALIAS cjson_static)
add_library(cjson::cjson_shared ALIAS cjson_shared)
add_library(cjson::cjson ALIAS cjson_shared)

# 导出目标
export(TARGETS cjson_static cjson_shared
    FILE ${CMAKE_CURRENT_BINARY_DIR}/cjsonTargets.cmake
    NAMESPACE cjson::
)

# 安装导出文件
install(EXPORT cjsonTargets
    FILE cjsonTargets.cmake
    NAMESPACE cjson::
    DESTINATION lib/cmake/cjson
)

# 创建配置文件
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cjsonConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cjsonConfig.cmake
    INSTALL_DESTINATION lib/cmake/cjson
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/cjsonConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cjsonConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cjsonConfigVersion.cmake
    DESTINATION lib/cmake/cjson
)

# 打印配置信息
message(STATUS "cJSON Library Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Enable cJSON_Utils: ${ENABLE_CJSON_UTILS}")
message(STATUS "  Enable tests: ${ENABLE_CJSON_TEST}")

if(MATH_LIB)
    message(STATUS "  Math library: ${MATH_LIB}")
else()
    message(STATUS "  Math library: NOT FOUND")
endif()