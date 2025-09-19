#!/bin/bash

# linx protocols 单元测试运行脚本
# 此脚本用于编译和运行 linx protocols 的单元测试

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

# 显示帮助信息
show_help() {
    echo "linx protocols 单元测试运行脚本"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help          显示此帮助信息"
    echo "  -c, --clean         清理构建文件"
    echo "  -d, --check-deps    检查依赖"
    echo "  -p, --protocol-only 仅运行 protocol 测试"
    echo "  -w, --websocket-only 仅运行 websocket 测试"
    echo "  -v, --verbose       详细输出"
    echo "  -q, --quiet         静默模式"
    echo ""
    echo "示例:"
    echo "  $0                  # 运行所有测试"
    echo "  $0 -p               # 仅运行 protocol 测试"
    echo "  $0 -w               # 仅运行 websocket 测试"
    echo "  $0 -c               # 清理构建文件"
    echo "  $0 -d               # 检查依赖"
}

# 检查是否在正确的目录
check_directory() {
    if [ ! -f "Makefile" ] || [ ! -f "test_linx_protocol.c" ]; then
        print_error "请在 sdk/protocols/test 目录下运行此脚本"
        exit 1
    fi
}

# 检查依赖
check_dependencies() {
    print_info "检查依赖..."
    
    # 检查 gcc
    if ! command -v gcc &> /dev/null; then
        print_error "gcc 编译器未安装"
        print_info "请安装 gcc 编译器"
        exit 1
    fi
    print_success "gcc 编译器已安装"
    
    # 检查 make
    if ! command -v make &> /dev/null; then
        print_error "make 工具未安装"
        print_info "请安装 make 工具"
        exit 1
    fi
    print_success "make 工具已安装"
    
    # 检查 mongoose 库
    if pkg-config --exists libmongoose 2>/dev/null; then
        print_success "mongoose 库已安装 (通过 pkg-config)"
    elif [ -f "/usr/local/include/mongoose.h" ] || [ -f "/opt/homebrew/include/mongoose.h" ] || [ -f "/usr/include/mongoose.h" ]; then
        print_success "mongoose 库已安装"
    else
        print_warning "mongoose 库未安装，WebSocket 测试将被跳过"
        print_info "在 macOS 上安装: brew install mongoose"
        print_info "在 Ubuntu/Debian 上安装: sudo apt-get install libmongoose-dev"
    fi
    
    print_success "依赖检查完成"
}

# 清理构建文件
clean_build() {
    print_info "清理构建文件..."
    make clean
    print_success "构建文件已清理"
}

# 编译测试
compile_tests() {
    print_info "编译测试..."
    if [ "$VERBOSE" = true ]; then
        make all
    else
        make all > /dev/null 2>&1
    fi
    print_success "测试编译完成"
}

# 运行 protocol 测试
run_protocol_test() {
    print_info "运行 linx_protocol 测试..."
    echo "----------------------------------------"
    if make test-protocol; then
        print_success "linx_protocol 测试通过"
        return 0
    else
        print_error "linx_protocol 测试失败"
        return 1
    fi
}

# 运行 websocket 测试
run_websocket_test() {
    print_info "运行 linx_websocket 测试..."
    echo "----------------------------------------"
    if make test-websocket; then
        print_success "linx_websocket 测试通过"
        return 0
    else
        print_error "linx_websocket 测试失败"
        return 1
    fi
}

# 运行所有测试
run_all_tests() {
    local protocol_result=0
    local websocket_result=0
    
    run_protocol_test || protocol_result=1
    echo ""
    run_websocket_test || websocket_result=1
    
    echo ""
    echo "========================================"
    if [ $protocol_result -eq 0 ] && [ $websocket_result -eq 0 ]; then
        print_success "所有测试通过!"
        return 0
    else
        if [ $protocol_result -ne 0 ]; then
            print_error "linx_protocol 测试失败"
        fi
        if [ $websocket_result -ne 0 ]; then
            print_error "linx_websocket 测试失败"
        fi
        return 1
    fi
}

# 主函数
main() {
    local CLEAN=false
    local CHECK_DEPS=false
    local PROTOCOL_ONLY=false
    local WEBSOCKET_ONLY=false
    local VERBOSE=false
    local QUIET=false
    
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -c|--clean)
                CLEAN=true
                shift
                ;;
            -d|--check-deps)
                CHECK_DEPS=true
                shift
                ;;
            -p|--protocol-only)
                PROTOCOL_ONLY=true
                shift
                ;;
            -w|--websocket-only)
                WEBSOCKET_ONLY=true
                shift
                ;;
            -v|--verbose)
                VERBOSE=true
                shift
                ;;
            -q|--quiet)
                QUIET=true
                shift
                ;;
            *)
                print_error "未知选项: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    # 检查目录
    check_directory
    
    # 静默模式处理
    if [ "$QUIET" = true ]; then
        exec > /dev/null 2>&1
    fi
    
    print_info "linx protocols 单元测试运行脚本"
    echo ""
    
    # 执行操作
    if [ "$CLEAN" = true ]; then
        clean_build
        exit 0
    fi
    
    if [ "$CHECK_DEPS" = true ]; then
        check_dependencies
        exit 0
    fi
    
    # 检查依赖
    check_dependencies
    echo ""
    
    # 编译测试
    compile_tests
    echo ""
    
    # 运行测试
    if [ "$PROTOCOL_ONLY" = true ]; then
        run_protocol_test
        exit $?
    elif [ "$WEBSOCKET_ONLY" = true ]; then
        run_websocket_test
        exit $?
    else
        run_all_tests
        exit $?
    fi
}

# 运行主函数
main "$@"