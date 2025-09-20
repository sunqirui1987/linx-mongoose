# GenerateHeaders.cmake - 生成平台特定的头文件
# 
# 此模块提供以下功能:
#   - 从模板生成配置头文件
#   - 设置平台特定的宏定义
#   - 生成版本信息头文件

include(CheckIncludeFile)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckTypeSize)

# 生成配置头文件
function(generate_config_header)
    set(options "")
    set(oneValueArgs TARGET_NAME OUTPUT_FILE TEMPLATE_FILE)
    set(multiValueArgs DEFINITIONS)
    cmake_parse_arguments(GCH "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    if(NOT GCH_OUTPUT_FILE)
        set(GCH_OUTPUT_FILE "${CMAKE_BINARY_DIR}/include/linx_config.h")
    endif()
    
    if(NOT GCH_TEMPLATE_FILE)
        set(GCH_TEMPLATE_FILE "${CMAKE_SOURCE_DIR}/cmake/templates/linx_config.h.in")
    endif()
    
    # 创建输出目录
    get_filename_component(OUTPUT_DIR "${GCH_OUTPUT_FILE}" DIRECTORY)
    file(MAKE_DIRECTORY "${OUTPUT_DIR}")
    
    # 平台检测
    if(WIN32)
        set(LINX_PLATFORM_WINDOWS 1)
    elseif(APPLE)
        set(LINX_PLATFORM_MACOS 1)
    elseif(UNIX)
        set(LINX_PLATFORM_LINUX 1)
    endif()
    
    # 架构检测
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(LINX_ARCH_64BIT 1)
    else()
        set(LINX_ARCH_32BIT 1)
    endif()
    
    # 编译器检测
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set(LINX_COMPILER_GCC 1)
    elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
        set(LINX_COMPILER_CLANG 1)
    elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        set(LINX_COMPILER_MSVC 1)
    endif()
    
    # 功能检测
    check_include_file("stdint.h" HAVE_STDINT_H)
    check_include_file("stdbool.h" HAVE_STDBOOL_H)
    check_include_file("unistd.h" HAVE_UNISTD_H)
    check_include_file("sys/time.h" HAVE_SYS_TIME_H)
    check_include_file("pthread.h" HAVE_PTHREAD_H)
    
    check_function_exists("gettimeofday" HAVE_GETTIMEOFDAY)
    check_function_exists("clock_gettime" HAVE_CLOCK_GETTIME)
    check_function_exists("usleep" HAVE_USLEEP)
    
    check_type_size("size_t" SIZEOF_SIZE_T)
    check_type_size("int" SIZEOF_INT)
    check_type_size("long" SIZEOF_LONG)
    check_type_size("void*" SIZEOF_VOID_P)
    
    # 设置功能宏
    if(HAVE_STDINT_H)
        set(LINX_HAVE_STDINT_H 1)
    endif()
    
    if(HAVE_STDBOOL_H)
        set(LINX_HAVE_STDBOOL_H 1)
    endif()
    
    if(HAVE_UNISTD_H)
        set(LINX_HAVE_UNISTD_H 1)
    endif()
    
    if(HAVE_SYS_TIME_H)
        set(LINX_HAVE_SYS_TIME_H 1)
    endif()
    
    if(HAVE_PTHREAD_H)
        set(LINX_HAVE_PTHREAD_H 1)
    endif()
    
    if(HAVE_GETTIMEOFDAY)
        set(LINX_HAVE_GETTIMEOFDAY 1)
    endif()
    
    if(HAVE_CLOCK_GETTIME)
        set(LINX_HAVE_CLOCK_GETTIME 1)
    endif()
    
    if(HAVE_USLEEP)
        set(LINX_HAVE_USLEEP 1)
    endif()
    
    # 组件功能检测
    if(TARGET Opus::opus)
        set(LINX_ENABLE_OPUS 1)
    endif()
    
    if(TARGET PortAudio::portaudio)
        set(LINX_ENABLE_PORTAUDIO 1)
    endif()
    
    # 处理自定义定义
    foreach(def ${GCH_DEFINITIONS})
        if(def MATCHES "^([^=]+)=(.*)$")
            set(${CMAKE_MATCH_1} ${CMAKE_MATCH_2})
        else()
            set(${def} 1)
        endif()
    endforeach()
    
    # 生成配置文件
    configure_file("${GCH_TEMPLATE_FILE}" "${GCH_OUTPUT_FILE}" @ONLY)
    
    message(STATUS "Generated config header: ${GCH_OUTPUT_FILE}")
endfunction()

# 生成版本头文件
function(generate_version_header)
    set(options "")
    set(oneValueArgs OUTPUT_FILE TEMPLATE_FILE VERSION_MAJOR VERSION_MINOR VERSION_PATCH)
    set(multiValueArgs "")
    cmake_parse_arguments(GVH "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    if(NOT GVH_OUTPUT_FILE)
        set(GVH_OUTPUT_FILE "${CMAKE_BINARY_DIR}/include/linx_version.h")
    endif()
    
    if(NOT GVH_TEMPLATE_FILE)
        set(GVH_TEMPLATE_FILE "${CMAKE_SOURCE_DIR}/cmake/templates/linx_version.h.in")
    endif()
    
    # 设置默认版本
    if(NOT GVH_VERSION_MAJOR)
        set(GVH_VERSION_MAJOR 1)
    endif()
    
    if(NOT GVH_VERSION_MINOR)
        set(GVH_VERSION_MINOR 0)
    endif()
    
    if(NOT GVH_VERSION_PATCH)
        set(GVH_VERSION_PATCH 0)
    endif()
    
    # 创建输出目录
    get_filename_component(OUTPUT_DIR "${GVH_OUTPUT_FILE}" DIRECTORY)
    file(MAKE_DIRECTORY "${OUTPUT_DIR}")
    
    # 设置版本变量
    set(LINX_VERSION_MAJOR ${GVH_VERSION_MAJOR})
    set(LINX_VERSION_MINOR ${GVH_VERSION_MINOR})
    set(LINX_VERSION_PATCH ${GVH_VERSION_PATCH})
    set(LINX_VERSION_STRING "${LINX_VERSION_MAJOR}.${LINX_VERSION_MINOR}.${LINX_VERSION_PATCH}")
    
    # 获取构建信息
    string(TIMESTAMP LINX_BUILD_DATE "%Y-%m-%d")
    string(TIMESTAMP LINX_BUILD_TIME "%H:%M:%S")
    string(TIMESTAMP LINX_BUILD_TIMESTAMP "%Y%m%d%H%M%S")
    
    # 获取 Git 信息（如果可用）
    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE LINX_GIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --always
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE LINX_GIT_DESCRIBE
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
    endif()
    
    if(NOT LINX_GIT_HASH)
        set(LINX_GIT_HASH "unknown")
    endif()
    
    if(NOT LINX_GIT_DESCRIBE)
        set(LINX_GIT_DESCRIBE "unknown")
    endif()
    
    # 生成版本文件
    configure_file("${GVH_TEMPLATE_FILE}" "${GVH_OUTPUT_FILE}" @ONLY)
    
    message(STATUS "Generated version header: ${GVH_OUTPUT_FILE}")
    message(STATUS "Version: ${LINX_VERSION_STRING}")
    message(STATUS "Git hash: ${LINX_GIT_HASH}")
endfunction()

# 添加生成的头文件到目标
function(add_generated_headers target)
    set(options "")
    set(oneValueArgs CONFIG_HEADER VERSION_HEADER)
    set(multiValueArgs "")
    cmake_parse_arguments(AGH "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    if(AGH_CONFIG_HEADER)
        target_include_directories(${target} PRIVATE 
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
        )
        
        # 添加依赖，确保头文件在编译前生成
        add_custom_target(generate_config_header_${target}
            DEPENDS ${AGH_CONFIG_HEADER}
        )
        add_dependencies(${target} generate_config_header_${target})
    endif()
    
    if(AGH_VERSION_HEADER)
        target_include_directories(${target} PRIVATE 
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
        )
        
        # 添加依赖，确保头文件在编译前生成
        add_custom_target(generate_version_header_${target}
            DEPENDS ${AGH_VERSION_HEADER}
        )
        add_dependencies(${target} generate_version_header_${target})
    endif()
endfunction()