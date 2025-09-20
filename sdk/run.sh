#!/bin/bash

# Linx SDK 编译脚本
# 用于编译整个 Linx SDK 库

set -e  # 遇到错误时退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
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

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDK_DIR="$SCRIPT_DIR"
BUILD_DIR="$SDK_DIR/build"

print_info "Linx SDK 编译脚本启动"
print_info "SDK 目录: $SDK_DIR"
print_info "构建目录: $BUILD_DIR"

# 检查必要的工具
check_dependencies() {
    print_info "检查编译依赖..."
    
    # 检查 cmake
    if ! command -v cmake &> /dev/null; then
        print_error "cmake 未安装，请先安装 cmake"
        exit 1
    fi
    
    # 检查 make
    if ! command -v make &> /dev/null; then
        print_error "make 未安装，请先安装 make"
        exit 1
    fi
    
    # 检查 pkg-config
    if ! command -v pkg-config &> /dev/null; then
        print_warning "pkg-config 未安装，某些依赖可能无法正确检测"
    fi
    
    print_success "依赖检查完成"
}

# 检查系统依赖库
check_system_libs() {
    print_info "检查系统依赖库..."
    
    # 检查 PortAudio
    if pkg-config --exists portaudio-2.0; then
        print_success "PortAudio 已安装"
    else
        print_warning "PortAudio 未找到，请确保已安装 PortAudio"
        print_info "macOS 安装命令: brew install portaudio"
    fi
    
    # 检查 Opus
    if pkg-config --exists opus; then
        print_success "Opus 已安装"
    else
        print_warning "Opus 未找到，请确保已安装 Opus"
        print_info "macOS 安装命令: brew install opus"
    fi
}

# 清理构建目录
clean_build() {
    if [ "$1" = "clean" ] || [ "$1" = "rebuild" ]; then
        print_info "清理构建目录..."
        if [ -d "$BUILD_DIR" ]; then
            rm -rf "$BUILD_DIR"
            print_success "构建目录已清理"
        fi
    fi
}

# 创建构建目录
create_build_dir() {
    if [ ! -d "$BUILD_DIR" ]; then
        print_info "创建构建目录..."
        mkdir -p "$BUILD_DIR"
        print_success "构建目录已创建"
    fi
}

# 配置构建
configure_build() {
    print_info "配置构建..."
    cd "$BUILD_DIR"
    
    # 设置构建类型
    BUILD_TYPE="Release"
    if [ "$1" = "debug" ]; then
        BUILD_TYPE="Debug"
        print_info "使用 Debug 构建模式"
    else
        print_info "使用 Release 构建模式"
    fi
    
    # 运行 cmake 配置
    cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
          -DCMAKE_INSTALL_PREFIX="$SDK_DIR/install" \
          "$SDK_DIR"
    
    if [ $? -eq 0 ]; then
        print_success "构建配置完成"
    else
        print_error "构建配置失败"
        exit 1
    fi
}

# 编译
build_sdk() {
    print_info "开始编译 SDK..."
    cd "$BUILD_DIR"
    
    # 获取 CPU 核心数用于并行编译
    if command -v nproc &> /dev/null; then
        JOBS=$(nproc)
    elif command -v sysctl &> /dev/null; then
        JOBS=$(sysctl -n hw.ncpu)
    else
        JOBS=4
    fi
    
    print_info "使用 $JOBS 个并行任务进行编译"
    
    # 编译
    make -j"$JOBS"
    
    if [ $? -eq 0 ]; then
        print_success "SDK 编译完成"
    else
        print_error "SDK 编译失败"
        exit 1
    fi
}

# 安装
install_sdk() {
    if [ "$1" = "install" ]; then
        print_info "安装 SDK..."
        cd "$BUILD_DIR"
        make install
        
        if [ $? -eq 0 ]; then
            print_success "SDK 安装完成"
            print_info "安装路径: $SDK_DIR/install"
        else
            print_error "SDK 安装失败"
            exit 1
        fi
    fi
}

# 运行测试
run_tests() {
    if [ "$1" = "test" ]; then
        print_info "运行测试..."
        cd "$BUILD_DIR"
        
        # 检查是否有测试目标
        if make help | grep -q test; then
            make test
            if [ $? -eq 0 ]; then
                print_success "测试通过"
            else
                print_error "测试失败"
                exit 1
            fi
        else
            print_warning "未找到测试目标"
        fi
    fi
}

# 显示帮助信息
show_help() {
    echo "Linx SDK 编译脚本"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  clean      清理构建目录"
    echo "  rebuild    清理并重新构建"
    echo "  debug      使用 Debug 模式构建"
    echo "  install    构建并安装"
    echo "  test       构建并运行测试"
    echo "  help       显示此帮助信息"
    echo ""
    echo "示例:"
    echo "  $0              # 默认构建 (Release 模式)"
    echo "  $0 debug        # Debug 模式构建"
    echo "  $0 clean        # 清理构建目录"
    echo "  $0 rebuild      # 清理并重新构建"
    echo "  $0 install      # 构建并安装"
    echo "  $0 test         # 构建并运行测试"
}

# 主函数
main() {
    # 解析命令行参数
    case "$1" in
        help|--help|-h)
            show_help
            exit 0
            ;;
        clean)
            clean_build clean
            print_success "清理完成"
            exit 0
            ;;
    esac
    
    # 检查依赖
    check_dependencies
    check_system_libs
    
    # 清理（如果需要）
    clean_build "$1"
    
    # 创建构建目录
    create_build_dir
    
    # 配置构建
    configure_build "$1"
    
    # 编译
    build_sdk
    
    # 安装（如果需要）
    install_sdk "$1"
    
    # 运行测试（如果需要）
    run_tests "$1"
    
    print_success "所有操作完成！"
    print_info "构建产物位于: $BUILD_DIR"
    
    # 显示生成的库文件
    if [ -d "$BUILD_DIR" ]; then
        print_info "生成的库文件:"
        find "$BUILD_DIR" -name "*.a" -o -name "*.so" -o -name "*.dylib" | head -10
    fi
}

# 运行主函数
main "$@"