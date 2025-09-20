# FindPortAudio.cmake - 查找 PortAudio 音频库
# 
# 此模块定义以下变量:
#   PORTAUDIO_FOUND - 如果找到 PortAudio 库则为 TRUE
#   PORTAUDIO_INCLUDE_DIRS - PortAudio 头文件目录
#   PORTAUDIO_LIBRARIES - PortAudio 库文件
#   PORTAUDIO_VERSION - PortAudio 版本号（如果可用）
#
# 此模块定义以下导入目标:
#   PortAudio::portaudio - PortAudio 库目标

include(FindPackageHandleStandardArgs)

# 首先尝试使用 pkg-config
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_PORTAUDIO QUIET portaudio-2.0)
endif()

# 查找头文件
find_path(PORTAUDIO_INCLUDE_DIR
    NAMES portaudio.h
    HINTS
        ${PC_PORTAUDIO_INCLUDEDIR}
        ${PC_PORTAUDIO_INCLUDE_DIRS}
    PATHS
        # Homebrew 路径 (macOS)
        /opt/homebrew/include
        /usr/local/include
        # 标准系统路径
        /usr/include
        # Windows vcpkg 路径
        ${CMAKE_PREFIX_PATH}/include
        # Windows 标准路径
        "C:/Program Files/PortAudio/include"
        "C:/Program Files (x86)/PortAudio/include"
)

# 查找库文件
find_library(PORTAUDIO_LIBRARY
    NAMES portaudio libportaudio portaudio_static
    HINTS
        ${PC_PORTAUDIO_LIBDIR}
        ${PC_PORTAUDIO_LIBRARY_DIRS}
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
        # Windows 标准路径
        "C:/Program Files/PortAudio/lib"
        "C:/Program Files (x86)/PortAudio/lib"
)

# 尝试获取版本信息
if(PORTAUDIO_INCLUDE_DIR AND EXISTS "${PORTAUDIO_INCLUDE_DIR}/portaudio.h")
    file(STRINGS "${PORTAUDIO_INCLUDE_DIR}/portaudio.h" PORTAUDIO_VERSION_LINE
         REGEX "^#define[ \t]+PA_VERSION[ \t]+[0-9]+")
    
    if(PORTAUDIO_VERSION_LINE)
        string(REGEX REPLACE "^#define[ \t]+PA_VERSION[ \t]+([0-9]+).*" "\\1"
               PORTAUDIO_VERSION_NUM "${PORTAUDIO_VERSION_LINE}")
        # PortAudio 版本格式: XXYYZZ (XX=major, YY=minor, ZZ=patch)
        math(EXPR PORTAUDIO_VERSION_MAJOR "${PORTAUDIO_VERSION_NUM} / 10000")
        math(EXPR PORTAUDIO_VERSION_MINOR "(${PORTAUDIO_VERSION_NUM} / 100) % 100")
        math(EXPR PORTAUDIO_VERSION_PATCH "${PORTAUDIO_VERSION_NUM} % 100")
        set(PORTAUDIO_VERSION "${PORTAUDIO_VERSION_MAJOR}.${PORTAUDIO_VERSION_MINOR}.${PORTAUDIO_VERSION_PATCH}")
    endif()
elseif(PC_PORTAUDIO_VERSION)
    set(PORTAUDIO_VERSION ${PC_PORTAUDIO_VERSION})
endif()

# 设置变量
set(PORTAUDIO_INCLUDE_DIRS ${PORTAUDIO_INCLUDE_DIR})
set(PORTAUDIO_LIBRARIES ${PORTAUDIO_LIBRARY})

# 处理标准参数
find_package_handle_standard_args(PortAudio
    REQUIRED_VARS PORTAUDIO_LIBRARY PORTAUDIO_INCLUDE_DIR
    VERSION_VAR PORTAUDIO_VERSION
)

# 创建导入目标
if(PORTAUDIO_FOUND AND NOT TARGET PortAudio::portaudio)
    add_library(PortAudio::portaudio UNKNOWN IMPORTED)
    set_target_properties(PortAudio::portaudio PROPERTIES
        IMPORTED_LOCATION "${PORTAUDIO_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${PORTAUDIO_INCLUDE_DIR}"
    )
    
    # 平台特定链接库
    if(WIN32)
        # Windows 需要额外的系统库
        set_property(TARGET PortAudio::portaudio PROPERTY
            INTERFACE_LINK_LIBRARIES winmm ole32 user32
        )
    elseif(APPLE)
        # macOS 需要 Core Audio 框架
        find_library(COREAUDIO_FRAMEWORK CoreAudio)
        find_library(AUDIOTOOLBOX_FRAMEWORK AudioToolbox)
        find_library(AUDIOUNIT_FRAMEWORK AudioUnit)
        find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
        
        set_property(TARGET PortAudio::portaudio PROPERTY
            INTERFACE_LINK_LIBRARIES 
                ${COREAUDIO_FRAMEWORK}
                ${AUDIOTOOLBOX_FRAMEWORK}
                ${AUDIOUNIT_FRAMEWORK}
                ${COREFOUNDATION_FRAMEWORK}
        )
    elseif(UNIX)
        # Linux 需要 ALSA 和线程库
        find_package(Threads REQUIRED)
        set_property(TARGET PortAudio::portaudio PROPERTY
            INTERFACE_LINK_LIBRARIES 
                Threads::Threads
                asound  # ALSA
        )
    endif()
endif()

# 标记为高级变量
mark_as_advanced(PORTAUDIO_INCLUDE_DIR PORTAUDIO_LIBRARY)