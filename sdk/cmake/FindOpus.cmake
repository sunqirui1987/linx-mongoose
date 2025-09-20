# FindOpus.cmake - 查找 Opus 音频编解码库
# 
# 此模块定义以下变量:
#   OPUS_FOUND - 如果找到 Opus 库则为 TRUE
#   OPUS_INCLUDE_DIRS - Opus 头文件目录
#   OPUS_LIBRARIES - Opus 库文件
#   OPUS_VERSION - Opus 版本号（如果可用）
#
# 此模块定义以下导入目标:
#   Opus::opus - Opus 库目标

include(FindPackageHandleStandardArgs)

# 首先尝试使用 pkg-config
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_OPUS QUIET opus)
endif()

# 查找头文件
find_path(OPUS_INCLUDE_DIR
    NAMES opus.h opus/opus.h
    HINTS
        ${PC_OPUS_INCLUDEDIR}
        ${PC_OPUS_INCLUDE_DIRS}
    PATHS
        # Homebrew 路径 (macOS)
        /opt/homebrew/include
        /usr/local/include
        # 标准系统路径
        /usr/include
        /usr/include/opus
        # Windows vcpkg 路径
        ${CMAKE_PREFIX_PATH}/include
        # ESP32 组件路径
        $ENV{IDF_PATH}/components/opus/include
    PATH_SUFFIXES opus
)

# 查找库文件
find_library(OPUS_LIBRARY
    NAMES opus libopus
    HINTS
        ${PC_OPUS_LIBDIR}
        ${PC_OPUS_LIBRARY_DIRS}
    PATHS
        # Homebrew 路径 (macOS)
        /opt/homebrew/lib
        /usr/local/lib
        # 标准系统路径
        /usr/lib
        /usr/lib/x86_64-linux-gnu
        /usr/lib/aarch64-linux-gnu
        # Windows vcpkg 路径
        ${CMAKE_PREFIX_PATH}/lib
        # ESP32 组件路径
        $ENV{IDF_PATH}/components/opus/lib
)

# 尝试获取版本信息
if(OPUS_INCLUDE_DIR AND EXISTS "${OPUS_INCLUDE_DIR}/opus/opus.h")
    file(STRINGS "${OPUS_INCLUDE_DIR}/opus/opus.h" OPUS_VERSION_MAJOR_LINE
         REGEX "^#define[ \t]+OPUS_VERSION_MAJOR[ \t]+[0-9]+")
    file(STRINGS "${OPUS_INCLUDE_DIR}/opus/opus.h" OPUS_VERSION_MINOR_LINE
         REGEX "^#define[ \t]+OPUS_VERSION_MINOR[ \t]+[0-9]+")
    file(STRINGS "${OPUS_INCLUDE_DIR}/opus/opus.h" OPUS_VERSION_MICRO_LINE
         REGEX "^#define[ \t]+OPUS_VERSION_MICRO[ \t]+[0-9]+")
    
    if(OPUS_VERSION_MAJOR_LINE)
        string(REGEX REPLACE "^#define[ \t]+OPUS_VERSION_MAJOR[ \t]+([0-9]+).*" "\\1"
               OPUS_VERSION_MAJOR "${OPUS_VERSION_MAJOR_LINE}")
        string(REGEX REPLACE "^#define[ \t]+OPUS_VERSION_MINOR[ \t]+([0-9]+).*" "\\1"
               OPUS_VERSION_MINOR "${OPUS_VERSION_MINOR_LINE}")
        string(REGEX REPLACE "^#define[ \t]+OPUS_VERSION_MICRO[ \t]+([0-9]+).*" "\\1"
               OPUS_VERSION_MICRO "${OPUS_VERSION_MICRO_LINE}")
        set(OPUS_VERSION "${OPUS_VERSION_MAJOR}.${OPUS_VERSION_MINOR}.${OPUS_VERSION_MICRO}")
    endif()
elseif(PC_OPUS_VERSION)
    set(OPUS_VERSION ${PC_OPUS_VERSION})
endif()

# 设置变量
set(OPUS_INCLUDE_DIRS ${OPUS_INCLUDE_DIR})
set(OPUS_LIBRARIES ${OPUS_LIBRARY})

# 处理标准参数
find_package_handle_standard_args(Opus
    REQUIRED_VARS OPUS_LIBRARY OPUS_INCLUDE_DIR
    VERSION_VAR OPUS_VERSION
)

# 创建导入目标
if(OPUS_FOUND AND NOT TARGET Opus::opus)
    add_library(Opus::opus UNKNOWN IMPORTED)
    set_target_properties(Opus::opus PROPERTIES
        IMPORTED_LOCATION "${OPUS_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${OPUS_INCLUDE_DIR}"
    )
    
    # 添加编译定义
    set_property(TARGET Opus::opus PROPERTY
        INTERFACE_COMPILE_DEFINITIONS OPUS_BUILD
    )
    
    # 平台特定设置
    if(UNIX AND NOT APPLE)
        # Linux 可能需要数学库
        set_property(TARGET Opus::opus PROPERTY
            INTERFACE_LINK_LIBRARIES m
        )
    endif()
endif()

# 标记为高级变量
mark_as_advanced(OPUS_INCLUDE_DIR OPUS_LIBRARY)