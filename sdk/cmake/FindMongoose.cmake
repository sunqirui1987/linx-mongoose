# FindMongoose.cmake - 查找 Mongoose 库
# 此模块查找 Mongoose 网络库并创建导入目标

# 设置变量
set(MONGOOSE_FOUND FALSE)
set(MONGOOSE_INCLUDE_DIRS "")
set(MONGOOSE_LIBRARIES "")

# 首先尝试使用 pkg-config
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_MONGOOSE QUIET libmongoose)
    if(PC_MONGOOSE_FOUND)
        set(MONGOOSE_INCLUDE_DIRS ${PC_MONGOOSE_INCLUDE_DIRS})
        set(MONGOOSE_LIBRARIES ${PC_MONGOOSE_LIBRARIES})
        set(MONGOOSE_FOUND TRUE)
        message(STATUS "Found Mongoose via pkg-config: ${MONGOOSE_LIBRARIES}")
    endif()
endif()

# 如果 pkg-config 失败，手动查找
if(NOT MONGOOSE_FOUND)
    # 查找头文件
    find_path(MONGOOSE_INCLUDE_DIR
        NAMES mongoose.h
        PATHS
            /opt/homebrew/include
            /usr/local/include
            /usr/include
            ${CMAKE_PREFIX_PATH}/include
        DOC "Mongoose include directory"
    )

    # 查找库文件
    find_library(MONGOOSE_LIBRARY
        NAMES mongoose libmongoose
        PATHS
            /opt/homebrew/lib
            /usr/local/lib
            /usr/lib
            ${CMAKE_PREFIX_PATH}/lib
        DOC "Mongoose library"
    )

    # 检查是否找到
    if(MONGOOSE_INCLUDE_DIR AND MONGOOSE_LIBRARY)
        set(MONGOOSE_INCLUDE_DIRS ${MONGOOSE_INCLUDE_DIR})
        set(MONGOOSE_LIBRARIES ${MONGOOSE_LIBRARY})
        set(MONGOOSE_FOUND TRUE)
        message(STATUS "Found Mongoose manually: ${MONGOOSE_LIBRARY}")
    endif()
endif()

# 获取版本信息（如果可能）
if(MONGOOSE_FOUND AND MONGOOSE_INCLUDE_DIR)
    file(READ "${MONGOOSE_INCLUDE_DIR}/mongoose.h" MONGOOSE_H_CONTENT)
    
    # 尝试提取版本号
    string(REGEX MATCH "#define MG_VERSION \"([0-9]+\\.[0-9]+)\"" 
           MONGOOSE_VERSION_MATCH "${MONGOOSE_H_CONTENT}")
    if(MONGOOSE_VERSION_MATCH)
        set(MONGOOSE_VERSION ${CMAKE_MATCH_1})
        message(STATUS "Mongoose version: ${MONGOOSE_VERSION}")
    endif()
endif()

# 创建导入目标
if(MONGOOSE_FOUND AND NOT TARGET Mongoose::mongoose)
    add_library(Mongoose::mongoose UNKNOWN IMPORTED)
    
    set_target_properties(Mongoose::mongoose PROPERTIES
        IMPORTED_LOCATION "${MONGOOSE_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${MONGOOSE_INCLUDE_DIR}"
    )
    
    # 平台特定链接库
    if(WIN32)
        set_target_properties(Mongoose::mongoose PROPERTIES
            INTERFACE_LINK_LIBRARIES "ws2_32;advapi32"
        )
    elseif(APPLE)
        # macOS 可能需要额外的框架
        set_target_properties(Mongoose::mongoose PROPERTIES
            INTERFACE_LINK_LIBRARIES ""
        )
    elseif(UNIX)
        # Linux 可能需要 pthread 和 dl
        find_package(Threads QUIET)
        if(Threads_FOUND)
            set_target_properties(Mongoose::mongoose PROPERTIES
                INTERFACE_LINK_LIBRARIES "${CMAKE_THREAD_LIBS_INIT};${CMAKE_DL_LIBS}"
            )
        endif()
    endif()
    
    message(STATUS "Created Mongoose::mongoose imported target")
endif()

# 处理标准的 find_package 参数
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Mongoose
    REQUIRED_VARS MONGOOSE_LIBRARY MONGOOSE_INCLUDE_DIR
    VERSION_VAR MONGOOSE_VERSION
)

# 设置缓存变量
mark_as_advanced(MONGOOSE_INCLUDE_DIR MONGOOSE_LIBRARY)

# 提供向后兼容的变量
if(MONGOOSE_FOUND)
    set(Mongoose_FOUND TRUE)
    set(Mongoose_INCLUDE_DIRS ${MONGOOSE_INCLUDE_DIRS})
    set(Mongoose_LIBRARIES ${MONGOOSE_LIBRARIES})
endif()