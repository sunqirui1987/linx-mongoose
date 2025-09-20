# Linx SDK Complete Voice Demo Makefile
# 用于编译完整的语音对话演示程序

# 编译器设置
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g -D_GNU_SOURCE
LDFLAGS = -lpthread -lm -ldl

# 目录设置
SDK_DIR = ../sdk
DEMO_DIR = .
BUILD_DIR = build
BIN_DIR = bin

# 包含路径
INCLUDES = -I$(SDK_DIR) \
           -I$(SDK_DIR)/protocols \
           -I$(SDK_DIR)/audio \
           -I$(SDK_DIR)/codecs \
           -I$(SDK_DIR)/mcp \
           -I$(SDK_DIR)/log \
           -I$(SDK_DIR)/utils \
           -I$(SDK_DIR)/websocket \
           -I/usr/local/include \
           -I/opt/homebrew/include \
           $(shell pkg-config --cflags opus json-c libcurl)

# 库搜索路径
LIB_PATHS = -L$(SDK_DIR)/lib \
            -L/usr/local/lib \
            -L/opt/homebrew/lib

# 库链接 (按依赖顺序)
LIBS = $(LIB_PATHS) \
       -llinx_sdk \
       $(shell pkg-config --libs opus json-c libcurl) \
       -lportaudio \
       -lsndfile \
       -lfftw3 \
       -lssl \
       -lcrypto

# 源文件
SOURCES = linx_demo.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = linx_demo

# 默认目标
all: check-deps $(TARGET)

# 检查依赖
check-deps:
	@echo "检查依赖库..."
	@pkg-config --exists opus || (echo "❌ 缺少 opus 库" && exit 1)
	@pkg-config --exists json-c || (echo "❌ 缺少 json-c 库" && exit 1)
	@pkg-config --exists libcurl || (echo "❌ 缺少 curl 库" && exit 1)
	@echo "✓ 依赖检查完成"

# 创建目录
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# 编译目标文件
%.o: %.c
	@echo "编译 $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 生成可执行文件
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	@echo "链接 $(TARGET)..."
	$(CC) $(OBJECTS) $(LIBS) $(LDFLAGS) -o $(BIN_DIR)/$(TARGET)
	@echo "✓ 编译完成: $(BIN_DIR)/$(TARGET)"
	@echo ""
	@echo "使用方法:"
	@echo "  make run              - 运行演示程序"
	@echo "  make run-help         - 显示程序帮助"
	@echo "  ./$(BIN_DIR)/$(TARGET) -h  - 显示详细帮助"

# 编译 (同 all)
build: all

# 清理编译文件
clean:
	rm -f *.o
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)
	@echo "✓ 清理完成"

# 完全清理 (包括依赖)
distclean: clean
	@echo "✓ 完全清理完成"

# 安装依赖 (macOS)
install-deps-macos:
	@echo "安装 macOS 依赖..."
	brew install opus json-c curl portaudio libsndfile fftw openssl
	@echo "✓ 依赖安装完成"

# 安装依赖 (Ubuntu/Debian)
install-deps-ubuntu:
	@echo "安装 Ubuntu/Debian 依赖..."
	sudo apt-get update
	sudo apt-get install -y libopus-dev libjson-c-dev libcurl4-openssl-dev \
	                        libportaudio2 libportaudio-dev libsndfile1-dev \
	                        libfftw3-dev libssl-dev pkg-config
	@echo "✓ 依赖安装完成"

# 安装程序到系统
install: $(TARGET)
	sudo cp $(BIN_DIR)/$(TARGET) /usr/local/bin/
	@echo "✓ 程序安装完成"

# 卸载程序
uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)
	@echo "✓ 程序卸载完成"

# 运行程序
run: $(TARGET)
	@echo "启动 Linx 语音对话演示..."
	./$(BIN_DIR)/$(TARGET)

# 显示程序帮助
run-help: $(TARGET)
	./$(BIN_DIR)/$(TARGET) --help

# 运行程序 (指定服务器)
run-server: $(TARGET)
	@echo "连接到指定服务器..."
	./$(BIN_DIR)/$(TARGET) --server ws://localhost:8080/ws

# 运行程序 (本地测试服务器)
run-local: $(TARGET)
	@echo "连接到本地测试服务器..."
	./$(BIN_DIR)/$(TARGET) --server ws://127.0.0.1:8080/ws

# 调试运行
debug: $(TARGET)
	@echo "启动调试模式..."
	gdb --args ./$(BIN_DIR)/$(TARGET)

# 内存检查
valgrind: $(TARGET)
	@echo "启动内存检查..."
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
	         ./$(BIN_DIR)/$(TARGET)

# 性能分析
profile: $(TARGET)
	@echo "启动性能分析..."
	perf record -g ./$(BIN_DIR)/$(TARGET)
	perf report

# 代码格式化
format:
	@echo "格式化代码..."
	clang-format -i *.c *.h
	@echo "✓ 代码格式化完成"

# 静态分析
analyze:
	@echo "执行静态分析..."
	clang-static-analyzer *.c
	@echo "✓ 静态分析完成"

# 代码检查
lint:
	@echo "执行代码检查..."
	cppcheck --enable=all --std=c99 *.c
	@echo "✓ 代码检查完成"

# 测试编译 (不链接)
test-compile:
	@echo "测试编译..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $(SOURCES)
	@echo "✓ 编译测试通过"
	@rm -f *.o

# 显示编译信息
info:
	@echo "=== 编译信息 ==="
	@echo "编译器: $(CC)"
	@echo "编译选项: $(CFLAGS)"
	@echo "包含路径: $(INCLUDES)"
	@echo "库路径: $(LIB_PATHS)"
	@echo "链接库: $(LIBS)"
	@echo "源文件: $(SOURCES)"
	@echo "目标文件: $(TARGET)"
	@echo "================"

# 显示帮助
help:
	@echo "Linx SDK 完整语音对话演示程序 Makefile"
	@echo ""
	@echo "编译目标:"
	@echo "  all              - 编译程序 (默认)"
	@echo "  build            - 编译程序"
	@echo "  clean            - 清理编译文件"
	@echo "  distclean        - 完全清理"
	@echo "  test-compile     - 测试编译"
	@echo ""
	@echo "依赖管理:"
	@echo "  check-deps       - 检查依赖"
	@echo "  install-deps-macos   - 安装 macOS 依赖"
	@echo "  install-deps-ubuntu  - 安装 Ubuntu 依赖"
	@echo ""
	@echo "安装/卸载:"
	@echo "  install          - 安装到系统"
	@echo "  uninstall        - 从系统卸载"
	@echo ""
	@echo "运行程序:"
	@echo "  run              - 运行程序"
	@echo "  run-help         - 显示程序帮助"
	@echo "  run-server       - 指定服务器运行"
	@echo "  run-local        - 连接本地服务器"
	@echo ""
	@echo "调试/分析:"
	@echo "  debug            - 调试运行"
	@echo "  valgrind         - 内存检查"
	@echo "  profile          - 性能分析"
	@echo ""
	@echo "代码质量:"
	@echo "  format           - 代码格式化"
	@echo "  analyze          - 静态分析"
	@echo "  lint             - 代码检查"
	@echo ""
	@echo "信息:"
	@echo "  info             - 显示编译信息"
	@echo "  help             - 显示此帮助"
	@echo ""
	@echo "功能特性:"
	@echo "  • 实时音频录制和播放"
	@echo "  • Opus 音频编解码"
	@echo "  • WebSocket 通信"
	@echo "  • TTS 语音合成"
	@echo "  • MCP 工具调用支持"
	@echo "  • 多线程音频处理"

# 声明伪目标
.PHONY: all build clean distclean check-deps install-deps-macos install-deps-ubuntu \
        install uninstall run run-help run-server run-local debug valgrind profile \
        format analyze lint test-compile info help

# 依赖关系
linx_demo.o: linx_demo.c $(SDK_DIR)/linx_sdk.h