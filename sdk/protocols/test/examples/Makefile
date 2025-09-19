# Makefile for linx protocols examples
# 用于编译和运行 linx 协议示例程序

# 编译器设置
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O2
LDFLAGS = -lm

# 目录设置
PROTOCOLS_DIR = ../../../protocols
EXAMPLES_DIR = .
BUILD_DIR = build
CJSON_DIR = ../../../cjson

# 源文件
PROTOCOL_SOURCES = $(PROTOCOLS_DIR)/linx_protocol.c $(PROTOCOLS_DIR)/linx_websocket.c
CJSON_SOURCES = $(CJSON_DIR)/cJSON.c $(CJSON_DIR)/cJSON_Utils.c
EXAMPLE_PROTOCOL_SRC = example_linx_protocol.c
EXAMPLE_WEBSOCKET_SRC = example_linx_websocket.c

# 目标文件
EXAMPLE_PROTOCOL_TARGET = $(BUILD_DIR)/example_linx_protocol
EXAMPLE_WEBSOCKET_TARGET = $(BUILD_DIR)/example_linx_websocket

# 包含路径
INCLUDES = -I$(PROTOCOLS_DIR) -I$(CJSON_DIR)

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
    else ifneq ($(wildcard /opt/homebrew/Cellar/mongoose/*/include/mongoose.h),)
        MONGOOSE_CFLAGS = -I$(shell find /opt/homebrew/Cellar/mongoose/*/include -name mongoose.h -exec dirname {} \; | head -1)
        MONGOOSE_LIBS = -L$(shell find /opt/homebrew/Cellar/mongoose/*/lib -name "libmongoose.*" -exec dirname {} \; | head -1) -lmongoose
    else ifneq ($(wildcard /usr/include/mongoose.h),)
        MONGOOSE_CFLAGS = -I/usr/include
        MONGOOSE_LIBS = -lmongoose
    endif
endif

# 默认目标
.PHONY: all clean help run-protocol run-websocket run-all check-deps

all: check-deps $(EXAMPLE_PROTOCOL_TARGET) $(EXAMPLE_WEBSOCKET_TARGET)

# 检查依赖
check-deps:
	@echo "🔍 检查编译依赖..."
	@which $(CC) > /dev/null || (echo "❌ 错误: 未找到 $(CC) 编译器" && exit 1)
	@echo "✅ $(CC) 编译器已安装"
	@which make > /dev/null || (echo "❌ 错误: 未找到 make 工具" && exit 1)
	@echo "✅ make 工具已安装"
	@if [ "$(MONGOOSE_CFLAGS)" = "-DMISSING_MONGOOSE" ]; then \
		echo "⚠️  警告: 未找到 mongoose 库，WebSocket 示例将无法编译"; \
		echo "   安装方法:"; \
		echo "   macOS: brew install mongoose"; \
		echo "   Ubuntu/Debian: sudo apt-get install libmongoose-dev"; \
	else \
		echo "✅ mongoose 库已找到"; \
	fi
	@echo "✅ 依赖检查完成"

# 创建构建目录
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# 编译 linx_protocol 示例
$(EXAMPLE_PROTOCOL_TARGET): $(EXAMPLE_PROTOCOL_SRC) $(PROTOCOL_SOURCES) $(CJSON_SOURCES) | $(BUILD_DIR)
	@echo "🔨 编译 linx_protocol 示例..."
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(PROTOCOL_SOURCES) $(CJSON_SOURCES) $(LDFLAGS)
	@echo "✅ linx_protocol 示例编译完成: $@"

# 编译 linx_websocket 示例
$(EXAMPLE_WEBSOCKET_TARGET): $(EXAMPLE_WEBSOCKET_SRC) $(PROTOCOL_SOURCES) $(CJSON_SOURCES) | $(BUILD_DIR)
	@echo "🔨 编译 linx_websocket 示例..."
	@if [ "$(MONGOOSE_CFLAGS)" = "-DMISSING_MONGOOSE" ]; then \
		echo "❌ 错误: 未找到 mongoose 库"; \
		echo "请安装 mongoose 库以编译 WebSocket 示例"; \
		echo "macOS: brew install mongoose"; \
		echo "Ubuntu/Debian: sudo apt-get install libmongoose-dev"; \
		exit 1; \
	else \
		$(CC) $(CFLAGS) $(INCLUDES) $(MONGOOSE_CFLAGS) -o $@ $< $(PROTOCOL_SOURCES) $(CJSON_SOURCES) $(LDFLAGS) $(MONGOOSE_LIBS); \
		echo "✅ linx_websocket 示例编译完成: $@"; \
	fi

# 运行 linx_protocol 示例
run-protocol: $(EXAMPLE_PROTOCOL_TARGET)
	@echo "🚀 运行 linx_protocol 示例..."
	@echo "================================"
	@$(EXAMPLE_PROTOCOL_TARGET)

# 运行 linx_websocket 示例
run-websocket: $(EXAMPLE_WEBSOCKET_TARGET)
	@echo "🚀 运行 linx_websocket 示例..."
	@echo "================================"
	@$(EXAMPLE_WEBSOCKET_TARGET)

# 运行所有示例
run-all: run-protocol run-websocket
	@echo ""
	@echo "🎉 所有示例运行完成！"

# 清理构建文件
clean:
	@echo "🧹 清理构建文件..."
	@rm -rf $(BUILD_DIR)
	@echo "✅ 清理完成"

# 显示帮助信息
help:
	@echo "📖 linx 协议示例 Makefile 帮助"
	@echo "================================"
	@echo ""
	@echo "可用目标:"
	@echo "  all              - 编译所有示例程序"
	@echo "  check-deps       - 检查编译依赖"
	@echo "  run-protocol     - 编译并运行 linx_protocol 示例"
	@echo "  run-websocket    - 编译并运行 linx_websocket 示例"
	@echo "  run-all          - 运行所有示例"
	@echo "  clean            - 清理构建文件"
	@echo "  help             - 显示此帮助信息"
	@echo ""
	@echo "示例用法:"
	@echo "  make                    # 编译所有示例"
	@echo "  make run-protocol       # 运行协议示例"
	@echo "  make run-websocket      # 运行 WebSocket 示例"
	@echo "  make run-all            # 运行所有示例"
	@echo "  make clean              # 清理构建文件"
	@echo ""
	@echo "依赖要求:"
	@echo "  - gcc 编译器"
	@echo "  - mongoose 库"
	@echo "  - cjson 库"

# 防止文件名冲突
.SUFFIXES:
.SECONDARY: