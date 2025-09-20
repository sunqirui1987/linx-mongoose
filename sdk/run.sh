#!/bin/bash

# Linx SDK 跨平台编译脚本
# 支持 macOS, Linux, Windows, ESP32 等多种平台

set -e  # 遇到错误时退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo -e "${PURPLE}[HEADER]${NC} $1"
}

print_step() {
    echo -e "${CYAN}[STEP]${NC} $1"
}

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDK_DIR="$SCRIPT_DIR"
BUILD_DIR="$SDK_DIR/build"
THIRD_DIR="$SDK_DIR/third"

# 默认配置
DEFAULT_BUILD_TYPE="Release"
DEFAULT_TARGET_PLATFORM=""
DEFAULT_TOOLCHAIN=""
CLEAN_BUILD=false
INSTALL_DEPS=false
VERBOSE=false

print_header "Linx SDK 跨平台编译脚本"
print_info "SDK 目录: $SDK_DIR"

# 显示帮助信息
show_help() {
    cat << EOF
Linx SDK 跨平台编译脚本

用法: $0 [选项] [命令]

命令:
    build           编译项目 (默认)
    clean           清理构建目录
    rebuild         清理后重新编译
    install-deps    安装依赖
    test            运行测试
    help            显示此帮助信息

选项:
    -p, --platform PLATFORM    指定目标平台 (macos, linux, windows, esp32, embedded_linux)
    -t, --toolchain TOOLCHAIN  指定工具链文件
    -b, --build-type TYPE       构建类型 (Debug, Release, MinSizeRel, RelWithDebInfo)
    -c, --clean                 清理构建目录
    -i, --install-deps          自动安装依赖
    -v, --verbose               详细输出
    -h, --help                  显示帮助信息

平台特定选项:
    ESP32:
        --idf-target TARGET     ESP32 目标芯片 (esp32, esp32s2, esp32s3, esp32c3)
    
    交叉编译:
        --sysroot PATH          指定 sysroot 路径
        --cross-prefix PREFIX   交叉编译工具链前缀

示例:
    $0                          # 自动检测平台并编译
    $0 -p macos build           # 为 macOS 平台编译
    $0 -p esp32 --idf-target esp32s3 build  # 为 ESP32-S3 编译
    $0 -p linux -i build       # 为 Linux 编译并自动安装依赖
    $0 clean                    # 清理构建目录
    $0 rebuild                  # 清理后重新编译

EOF
}

# 检测操作系统
detect_host_os() {
    case "$(uname -s)" in
        Darwin*)    HOST_OS="macos" ;;
        Linux*)     HOST_OS="linux" ;;
        CYGWIN*|MINGW*|MSYS*) HOST_OS="windows" ;;
        *)          HOST_OS="unknown" ;;
    esac
    print_info "检测到主机操作系统: $HOST_OS"
}

# 检测平台架构
detect_host_arch() {
    case "$(uname -m)" in
        x86_64|amd64)   HOST_ARCH="x64" ;;
        i386|i686)      HOST_ARCH="x86" ;;
        arm64|aarch64)  HOST_ARCH="arm64" ;;
        arm*)           HOST_ARCH="arm" ;;
        *)              HOST_ARCH="unknown" ;;
    esac
    print_info "检测到主机架构: $HOST_ARCH"
}

# 自动检测目标平台
auto_detect_platform() {
    if [ -z "$TARGET_PLATFORM" ]; then
        # 检查是否在 ESP-IDF 环境中
        if [ -n "$IDF_PATH" ]; then
            TARGET_PLATFORM="esp32"
            print_info "检测到 ESP-IDF 环境，设置目标平台为: esp32"
        else
            TARGET_PLATFORM="$HOST_OS"
            print_info "自动设置目标平台为: $TARGET_PLATFORM"
        fi
    fi
}

# 检查必要的工具
check_basic_tools() {
    print_step "检查基础工具..."
    
    local missing_tools=()
    
    # 检查 cmake
    if ! command -v cmake &> /dev/null; then
        missing_tools+=("cmake")
    fi
    
    # 检查 git
    if ! command -v git &> /dev/null; then
        missing_tools+=("git")
    fi
    
    # 根据平台检查特定工具
    case "$TARGET_PLATFORM" in
        "macos")
            if ! command -v brew &> /dev/null; then
                print_warning "Homebrew 未安装，某些依赖可能无法自动安装"
            fi
            ;;
        "linux")
            if ! command -v make &> /dev/null; then
                missing_tools+=("make")
            fi
            ;;
        "windows")
            # Windows 特定检查
            ;;
        "esp32")
            if [ -z "$IDF_PATH" ]; then
                print_error "ESP-IDF 环境未设置，请先运行 'source \$IDF_PATH/export.sh'"
                exit 1
            fi
            ;;
    esac
    
    if [ ${#missing_tools[@]} -gt 0 ]; then
        print_error "缺少必要工具: ${missing_tools[*]}"
        print_info "请安装缺少的工具后重试"
        exit 1
    fi
    
    print_success "基础工具检查完成"
}

# 安装 macOS 依赖
install_macos_deps() {
    print_step "安装 macOS 依赖..."
    
    if ! command -v brew &> /dev/null; then
        print_error "Homebrew 未安装，请先安装 Homebrew"
        print_info "安装命令: /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
        exit 1
    fi
    
    local packages=("cmake" "pkg-config" "portaudio" "opus")
    
    for package in "${packages[@]}"; do
        if brew list "$package" &> /dev/null; then
            print_info "✓ $package 已安装"
        else
            print_info "安装 $package..."
            brew install "$package"
        fi
    done
    
    print_success "macOS 依赖安装完成"
}

# 安装 Linux 依赖
install_linux_deps() {
    print_step "安装 Linux 依赖..."
    
    # 检测 Linux 发行版
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        DISTRO=$ID
    else
        print_warning "无法检测 Linux 发行版，使用通用方法"
        DISTRO="unknown"
    fi
    
    case "$DISTRO" in
        "ubuntu"|"debian")
            sudo apt-get update
            sudo apt-get install -y build-essential cmake pkg-config \
                libportaudio2 libportaudio-dev libopus0 libopus-dev git
            ;;
        "fedora"|"rhel"|"centos")
            if command -v dnf &> /dev/null; then
                sudo dnf install -y gcc gcc-c++ cmake pkgconfig \
                    portaudio-devel opus-devel git
            else
                sudo yum install -y gcc gcc-c++ cmake pkgconfig \
                    portaudio-devel opus-devel git
            fi
            ;;
        "arch"|"manjaro")
            sudo pacman -S --noconfirm base-devel cmake pkgconf \
                portaudio opus git
            ;;
        *)
            print_warning "未知的 Linux 发行版: $DISTRO"
            print_info "请手动安装以下依赖: cmake, pkg-config, portaudio, opus"
            ;;
    esac
    
    print_success "Linux 依赖安装完成"
}

# 安装 ESP32 依赖
install_esp32_deps() {
    print_step "检查 ESP32 依赖..."
    
    if [ -z "$IDF_PATH" ]; then
        print_error "ESP-IDF 环境未设置"
        print_info "请按照以下步骤安装 ESP-IDF:"
        print_info "1. git clone --recursive https://github.com/espressif/esp-idf.git"
        print_info "2. cd esp-idf && ./install.sh"
        print_info "3. source ./export.sh"
        exit 1
    fi
    
    print_info "ESP-IDF 路径: $IDF_PATH"
    
    # 检查 ESP-IDF 版本
    if [ -f "$IDF_PATH/version.txt" ]; then
        IDF_VERSION=$(cat "$IDF_PATH/version.txt")
        print_info "ESP-IDF 版本: $IDF_VERSION"
    fi
    
    # 检查工具链
    if [ -n "$IDF_TARGET" ]; then
        print_info "ESP32 目标: $IDF_TARGET"
    else
        export IDF_TARGET="esp32"
        print_info "使用默认 ESP32 目标: $IDF_TARGET"
    fi
    
    print_success "ESP32 环境检查完成"
}

# 安装依赖
install_dependencies() {
    print_header "安装平台依赖"
    
    case "$TARGET_PLATFORM" in
        "macos")
            install_macos_deps
            ;;
        "linux"|"embedded_linux")
            install_linux_deps
            ;;
        "windows")
            print_warning "Windows 依赖需要手动安装 vcpkg 或使用预编译库"
            ;;
        "esp32")
            install_esp32_deps
            ;;
        *)
            print_warning "未知平台: $TARGET_PLATFORM，跳过依赖安装"
            ;;
    esac
}

# 设置构建参数
setup_build_params() {
    print_step "设置构建参数..."
    
    # 设置构建目录
    BUILD_DIR="$SDK_DIR/build_${TARGET_PLATFORM}"
    print_info "构建目录: $BUILD_DIR"
    
    # 设置 CMake 参数
    CMAKE_ARGS=()
    CMAKE_ARGS+=("-DCMAKE_BUILD_TYPE=$BUILD_TYPE")
    
    # 平台特定参数
    case "$TARGET_PLATFORM" in
        "esp32")
            if [ -n "$TOOLCHAIN_FILE" ]; then
                CMAKE_ARGS+=("-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE")
            else
                CMAKE_ARGS+=("-DCMAKE_TOOLCHAIN_FILE=$SDK_DIR/cmake/toolchains/esp32.cmake")
            fi
            CMAKE_ARGS+=("-DLINX_TARGET_PLATFORM=esp32")
            if [ -n "$IDF_TARGET" ]; then
                CMAKE_ARGS+=("-DIDF_TARGET=$IDF_TARGET")
            fi
            ;;
        "embedded_linux")
            if [ -n "$TOOLCHAIN_FILE" ]; then
                CMAKE_ARGS+=("-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE")
            fi
            CMAKE_ARGS+=("-DLINX_TARGET_PLATFORM=embedded_linux")
            if [ -n "$SYSROOT" ]; then
                CMAKE_ARGS+=("-DCMAKE_SYSROOT=$SYSROOT")
            fi
            ;;
        *)
            CMAKE_ARGS+=("-DLINX_TARGET_PLATFORM=$TARGET_PLATFORM")
            ;;
    esac
    
    if [ "$VERBOSE" = true ]; then
        CMAKE_ARGS+=("-DCMAKE_VERBOSE_MAKEFILE=ON")
    fi
    
    print_info "CMake 参数: ${CMAKE_ARGS[*]}"
}

# 清理构建目录
clean_build_dir() {
    print_step "清理构建目录..."
    
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        print_success "构建目录已清理: $BUILD_DIR"
    else
        print_info "构建目录不存在，无需清理"
    fi
}

# 创建构建目录
create_build_dir() {
    if [ ! -d "$BUILD_DIR" ]; then
        print_step "创建构建目录: $BUILD_DIR"
        mkdir -p "$BUILD_DIR"
    fi
}

# 配置项目
configure_project() {
    print_step "配置项目..."
    
    cd "$BUILD_DIR"
    
    if [ "$VERBOSE" = true ]; then
        cmake "${CMAKE_ARGS[@]}" "$SDK_DIR"
    else
        cmake "${CMAKE_ARGS[@]}" "$SDK_DIR" > /dev/null
    fi
    
    print_success "项目配置完成"
}

# 编译项目
build_project() {
    print_step "编译项目..."
    
    cd "$BUILD_DIR"
    
    # 获取 CPU 核心数用于并行编译
    if command -v nproc &> /dev/null; then
        JOBS=$(nproc)
    elif command -v sysctl &> /dev/null; then
        JOBS=$(sysctl -n hw.ncpu)
    else
        JOBS=4
    fi
    
    print_info "使用 $JOBS 个并行任务编译"
    
    if [ "$VERBOSE" = true ]; then
        cmake --build . --parallel "$JOBS"
    else
        cmake --build . --parallel "$JOBS" > /dev/null
    fi
    
    print_success "项目编译完成"
}

# 运行测试
run_tests() {
    print_step "运行测试..."
    
    cd "$BUILD_DIR"
    
    if [ -f "CTestTestfile.cmake" ]; then
        ctest --output-on-failure
        print_success "测试完成"
    else
        print_warning "未找到测试，跳过测试阶段"
    fi
}

# 解析命令行参数
parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -p|--platform)
                TARGET_PLATFORM="$2"
                shift 2
                ;;
            -t|--toolchain)
                TOOLCHAIN_FILE="$2"
                shift 2
                ;;
            -b|--build-type)
                BUILD_TYPE="$2"
                shift 2
                ;;
            -c|--clean)
                CLEAN_BUILD=true
                shift
                ;;
            -i|--install-deps)
                INSTALL_DEPS=true
                shift
                ;;
            -v|--verbose)
                VERBOSE=true
                shift
                ;;
            --idf-target)
                export IDF_TARGET="$2"
                shift 2
                ;;
            --sysroot)
                SYSROOT="$2"
                shift 2
                ;;
            --cross-prefix)
                CROSS_PREFIX="$2"
                shift 2
                ;;
            -h|--help)
                show_help
                exit 0
                ;;
            build|clean|rebuild|install-deps|test|help)
                COMMAND="$1"
                shift
                ;;
            *)
                print_error "未知参数: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    # 设置默认值
    BUILD_TYPE="${BUILD_TYPE:-$DEFAULT_BUILD_TYPE}"
    COMMAND="${COMMAND:-build}"
}

# 主函数
main() {
    # 解析参数
    parse_arguments "$@"
    
    # 检测主机环境
    detect_host_os
    detect_host_arch
    
    # 处理命令
    case "$COMMAND" in
        "help")
            show_help
            exit 0
            ;;
        "clean")
            auto_detect_platform
            setup_build_params
            clean_build_dir
            exit 0
            ;;
        "install-deps")
            auto_detect_platform
            install_dependencies
            exit 0
            ;;
    esac
    
    # 自动检测平台
    auto_detect_platform
    
    # 检查基础工具
    check_basic_tools
    
    # 安装依赖（如果需要）
    if [ "$INSTALL_DEPS" = true ]; then
        install_dependencies
    fi
    
    # 设置构建参数
    setup_build_params
    
    # 处理构建命令
    case "$COMMAND" in
        "rebuild")
            clean_build_dir
            create_build_dir
            configure_project
            build_project
            ;;
        "build")
            if [ "$CLEAN_BUILD" = true ]; then
                clean_build_dir
            fi
            create_build_dir
            configure_project
            build_project
            ;;
        "test")
            if [ ! -d "$BUILD_DIR" ]; then
                print_error "构建目录不存在，请先运行 build 命令"
                exit 1
            fi
            run_tests
            ;;
        *)
            print_error "未知命令: $COMMAND"
            show_help
            exit 1
            ;;
    esac
    
    print_success "操作完成！"
    print_info "构建输出位于: $BUILD_DIR"
}

# 运行主函数
main "$@"