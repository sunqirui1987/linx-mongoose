# Makefile for linx protocols unit tests

# 编译器设置
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O0
LDFLAGS = -lm

# 目录设置
SRC_DIR = ..
CJSON_DIR = ../../cjson
TEST_DIR = .
BUILD_DIR = build

# 源文件
PROTOCOL_SOURCES = $(SRC_DIR)/linx_protocol.c
WEBSOCKET_SOURCES = $(SRC_DIR)/linx_websocket.c
CJSON_SOURCES = $(CJSON_DIR)/cJSON.c $(CJSON_DIR)/cJSON_Utils.c

# 测试文件
TEST_PROTOCOL_SOURCES = test_linx_protocol.c
TEST_WEBSOCKET_SOURCES = test_linx_websocket.c

# 目标文件
TEST_PROTOCOL_TARGET = $(BUILD_DIR)/test_linx_protocol
TEST_WEBSOCKET_TARGET = $(BUILD_DIR)/test_linx_websocket

# 包含路径
INCLUDES = -I$(SRC_DIR) -I$(CJSON_DIR) -I.

# 检查是否安装了 mongoose
MONGOOSE_CFLAGS = $(shell pkg-config --cflags libmongoose 2>/dev/null || echo "-DMISSING_MONGOOSE")
MONGOOSE_LIBS = $(shell pkg-config --libs libmongoose 2>/dev/null || echo "")

# 如果没有安装 mongoose，尝试查找本地安装
ifeq ($(MONGOOSE_CFLAGS),-DMISSING_MONGOOSE)
    # 检查常见的 mongoose 安装位置
    ifneq ($(wildcard /usr/local/include/mongoose.h),)
        MONGOOSE_CFLAGS = -I/usr/local/include
        MONGOOSE_LIBS = -L/usr/local/lib -lmongoose
    else ifneq ($(wildcard /opt/homebrew/include/mongoose.h),)
        MONGOOSE_CFLAGS = -I/opt/homebrew/include
        MONGOOSE_LIBS = -L/opt/homebrew/lib -lmongoose
    else ifneq ($(wildcard /usr/include/mongoose.h),)
        MONGOOSE_CFLAGS = -I/usr/include
        MONGOOSE_LIBS = -lmongoose
    endif
endif

# 默认目标
all: $(BUILD_DIR) $(TEST_PROTOCOL_TARGET) $(TEST_WEBSOCKET_TARGET)

# 创建构建目录
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 编译协议测试
$(TEST_PROTOCOL_TARGET): $(TEST_PROTOCOL_SOURCES) $(PROTOCOL_SOURCES) $(CJSON_SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

# 编译 WebSocket 测试
$(TEST_WEBSOCKET_TARGET): $(TEST_WEBSOCKET_SOURCES) $(WEBSOCKET_SOURCES) $(PROTOCOL_SOURCES) $(CJSON_SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) $(MONGOOSE_CFLAGS) -o $@ $^ $(LDFLAGS) $(MONGOOSE_LIBS)

# 运行所有测试
test: all
	@echo "运行 linx_protocol 测试..."
	@$(TEST_PROTOCOL_TARGET)
	@echo ""
	@echo "运行 linx_websocket 测试..."
	@if [ "$(MONGOOSE_CFLAGS)" = "-DMISSING_MONGOOSE" ]; then \
		echo "警告: 未找到 mongoose 库，跳过 WebSocket 测试"; \
		echo "请安装 mongoose 库以运行完整测试"; \
	else \
		$(TEST_WEBSOCKET_TARGET); \
	fi

# 运行单个测试
test-protocol: $(TEST_PROTOCOL_TARGET)
	@echo "运行 linx_protocol 测试..."
	@$(TEST_PROTOCOL_TARGET)

test-websocket: $(TEST_WEBSOCKET_TARGET)
	@echo "运行 linx_websocket 测试..."
	@if [ "$(MONGOOSE_CFLAGS)" = "-DMISSING_MONGOOSE" ]; then \
		echo "错误: 未找到 mongoose 库"; \
		echo "请安装 mongoose 库以运行 WebSocket 测试"; \
		exit 1; \
	else \
		$(TEST_WEBSOCKET_TARGET); \
	fi

# 清理
clean:
	rm -rf $(BUILD_DIR)

# 显示帮助信息
help:
	@echo "可用的目标:"
	@echo "  all           - 编译所有测试"
	@echo "  test          - 运行所有测试"
	@echo "  test-protocol - 运行 linx_protocol 测试"
	@echo "  test-websocket- 运行 linx_websocket 测试"
	@echo "  clean         - 清理构建文件"
	@echo "  help          - 显示此帮助信息"
	@echo ""
	@echo "依赖要求:"
	@echo "  - gcc 编译器"
	@echo "  - mongoose 库 (用于 WebSocket 测试)"
	@echo ""
	@echo "安装 mongoose 库 (macOS):"
	@echo "  brew install mongoose"
	@echo ""
	@echo "安装 mongoose 库 (Ubuntu/Debian):"
	@echo "  sudo apt-get install libmongoose-dev"

# 检查依赖
check-deps:
	@echo "检查依赖..."
	@which gcc > /dev/null || (echo "错误: 未找到 gcc 编译器" && exit 1)
	@echo "✓ gcc 编译器已安装"
	@if [ "$(MONGOOSE_CFLAGS)" = "-DMISSING_MONGOOSE" ]; then \
		echo "⚠ mongoose 库未安装 (WebSocket 测试将被跳过)"; \
	else \
		echo "✓ mongoose 库已安装"; \
	fi
	@echo "依赖检查完成"

.PHONY: all test test-protocol test-websocket clean help check-deps