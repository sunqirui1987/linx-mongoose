#!/bin/bash

# linx 协议示例运行脚本
# 提供友好的命令行界面来编译和运行示例程序

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# 脚本信息
SCRIPT_NAME="linx 协议示例运行脚本"
VERSION="1.0.0"

# 显示帮助信息
show_help() {
    echo -e "${CYAN}${SCRIPT_NAME} v${VERSION}${NC}"
    echo -e "${CYAN}================================${NC}"
    echo ""
    echo -e "${YELLOW}用法:${NC}"
    echo "  $0 [选项]"
    echo ""
    echo -e "${YELLOW}选项:${NC}"
    echo "  -p, --protocol     仅运行 linx_protocol 示例"
    echo "  -w, --websocket    仅运行 linx_websocket 示例"
    echo "  -a, --all          运行所有示例 (默认)"
    echo "  -c, --clean        清理构建文件"
    echo "  -d, --deps         检查依赖"
    echo "  -b, --build        仅编译，不运行"
    echo "  -h, --help         显示此帮助信息"
    echo "  -v, --verbose      详细输出模式"
    echo ""
    echo -e "${YELLOW}示例:${NC}"
    echo "  $0                 # 运行所有示例"
    echo "  $0 -p              # 仅运行协议示例"
    echo "  $0 -w              # 仅运行 WebSocket 示例"
    echo "  $0 -c              # 清理构建文件"
    echo "  $0 -d              # 检查依赖"
    echo ""
}

# 日志函数
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_step() {
    echo -e "${PURPLE}[STEP]${NC} $1"
}

# 检查是否在正确的目录
check_directory() {
    if [[ ! -f "Makefile" ]] || [[ ! -f "example_linx_protocol.c" ]] || [[ ! -f "example_linx_websocket.c" ]]; then
        log_error "请在 examples 目录中运行此脚本"
        log_info "当前目录: $(pwd)"
        log_info "期望文件: Makefile, example_linx_protocol.c, example_linx_websocket.c"
        exit 1
    fi
}

# 检查依赖
check_dependencies() {
    log_step "检查编译依赖..."
    
    # 检查 make
    if ! command -v make &> /dev/null; then
        log_error "未找到 make 工具"
        return 1
    fi
    log_success "make 工具已安装"
    
    # 检查 gcc
    if ! command -v gcc &> /dev/null; then
        log_error "未找到 gcc 编译器"
        return 1
    fi
    log_success "gcc 编译器已安装"
    
    # 检查 mongoose 库 (可选)
    if pkg-config --exists libmongoose 2>/dev/null; then
        log_success "mongoose 库已安装"
    else
        log_warning "未找到 mongoose 库，可能需要手动配置"
    fi
    
    log_success "依赖检查完成"
    return 0
}

# 清理构建文件
clean_build() {
    log_step "清理构建文件..."
    if make clean; then
        log_success "构建文件清理完成"
    else
        log_error "清理构建文件失败"
        return 1
    fi
}

# 编译示例
build_examples() {
    local target="$1"
    
    if [[ -n "$target" ]]; then
        log_step "编译 $target 示例..."
        if make "$target"; then
            log_success "$target 示例编译完成"
        else
            log_error "$target 示例编译失败"
            return 1
        fi
    else
        log_step "编译所有示例..."
        if make all; then
            log_success "所有示例编译完成"
        else
            log_error "示例编译失败"
            return 1
        fi
    fi
}

# 运行示例
run_example() {
    local example_type="$1"
    
    case "$example_type" in
        "protocol")
            log_step "运行 linx_protocol 示例..."
            echo -e "${CYAN}================================${NC}"
            if make run-protocol; then
                log_success "linx_protocol 示例运行完成"
            else
                log_error "linx_protocol 示例运行失败"
                return 1
            fi
            ;;
        "websocket")
            log_step "运行 linx_websocket 示例..."
            echo -e "${CYAN}================================${NC}"
            if make run-websocket; then
                log_success "linx_websocket 示例运行完成"
            else
                log_error "linx_websocket 示例运行失败"
                return 1
            fi
            ;;
        "all")
            log_step "运行所有示例..."
            echo -e "${CYAN}================================${NC}"
            if make run-all; then
                log_success "所有示例运行完成"
            else
                log_error "示例运行失败"
                return 1
            fi
            ;;
        *)
            log_error "未知的示例类型: $example_type"
            return 1
            ;;
    esac
}

# 主函数
main() {
    local run_type="all"
    local build_only=false
    local verbose=false
    
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            -p|--protocol)
                run_type="protocol"
                shift
                ;;
            -w|--websocket)
                run_type="websocket"
                shift
                ;;
            -a|--all)
                run_type="all"
                shift
                ;;
            -c|--clean)
                check_directory
                clean_build
                exit $?
                ;;
            -d|--deps)
                check_dependencies
                exit $?
                ;;
            -b|--build)
                build_only=true
                shift
                ;;
            -h|--help)
                show_help
                exit 0
                ;;
            -v|--verbose)
                verbose=true
                shift
                ;;
            *)
                log_error "未知选项: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    # 显示脚本信息
    echo -e "${CYAN}${SCRIPT_NAME}${NC}"
    echo ""
    
    # 检查目录
    check_directory
    
    # 检查依赖
    if ! check_dependencies; then
        log_error "依赖检查失败，请安装必要的依赖后重试"
        exit 1
    fi
    echo ""
    
    # 清理之前的构建
    if [[ "$verbose" == true ]]; then
        log_info "清理之前的构建文件..."
        clean_build
        echo ""
    fi
    
    # 编译示例
    if ! build_examples; then
        log_error "编译失败"
        exit 1
    fi
    echo ""
    
    # 如果只是编译，则退出
    if [[ "$build_only" == true ]]; then
        log_success "编译完成，使用 -h 查看运行选项"
        exit 0
    fi
    
    # 运行示例
    if ! run_example "$run_type"; then
        log_error "示例运行失败"
        exit 1
    fi
    
    echo ""
    log_success "🎉 示例程序执行完成！"
}

# 运行主函数
main "$@"