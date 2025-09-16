# LINX WebSocket SDK Makefile
# 基于Mongoose的C99标准WebSocket客户端SDK

# 编译器设置
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2 -fPIC
DEBUG_CFLAGS = -std=c99 -Wall -Wextra -g -O0 -fPIC -DDEBUG
LDFLAGS = -lm -lpthread

# 目录设置
SRC_DIR = src
INCLUDE_DIR = include
EXAMPLES_DIR = examples
BUILD_DIR = build
LIB_DIR = lib

# Mongoose库设置（假设已安装或在第三方目录中）
MONGOOSE_INCLUDE = -I/opt/homebrew/opt/mongoose/include
MONGOOSE_LIB = -L/opt/homebrew/opt/mongoose/lib -lmongoose

# 或者使用本地Mongoose源码
# MONGOOSE_INCLUDE = -Ithird_party/mongoose
# MONGOOSE_SRC = third_party/mongoose/mongoose.c

# 源文件
SDK_SOURCES = $(SRC_DIR)/linx_websocket_sdk.c \
              $(SRC_DIR)/linx_connection.c \
              $(SRC_DIR)/linx_json.c \
              $(SRC_DIR)/linx_audio.c \
              $(SRC_DIR)/linx_utils.c \
              $(SRC_DIR)/linx_log.c

# 头文件
SDK_HEADERS = $(INCLUDE_DIR)/linx_websocket_sdk.h \
              $(SRC_DIR)/linx_internal.h

# 目标文件
SDK_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SDK_SOURCES))

# 库文件
STATIC_LIB = $(LIB_DIR)/liblinx_websocket_sdk.a
SHARED_LIB = $(LIB_DIR)/liblinx_websocket_sdk.so

# 示例程序
EXAMPLE_BASIC = $(BUILD_DIR)/basic_example

# 默认目标
all: directories $(STATIC_LIB) $(SHARED_LIB) examples

# 创建目录
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(LIB_DIR)

# 编译目标文件
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(SDK_HEADERS)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(MONGOOSE_INCLUDE) -I$(INCLUDE_DIR) -I$(SRC_DIR) -c $< -o $@

# 创建静态库
$(STATIC_LIB): $(SDK_OBJECTS)
	@echo "Creating static library..."
	ar rcs $@ $^
	@echo "Static library created: $@"

# 创建动态库
$(SHARED_LIB): $(SDK_OBJECTS)
	@echo "Creating shared library..."
	$(CC) -shared -o $@ $^ $(MONGOOSE_LIB) $(LDFLAGS)
	@echo "Shared library created: $@"

# 编译示例程序
examples: $(EXAMPLE_BASIC)

$(EXAMPLE_BASIC): $(EXAMPLES_DIR)/basic_example.c $(STATIC_LIB)
	@echo "Compiling basic example..."
	$(CC) $(CFLAGS) $(MONGOOSE_INCLUDE) -I$(INCLUDE_DIR) -o $@ $< -L$(LIB_DIR) -llinx_websocket_sdk $(MONGOOSE_LIB) $(LDFLAGS)
	@echo "Basic example compiled: $@"

# 调试版本
debug: CFLAGS = $(DEBUG_CFLAGS)
debug: clean all

# 安装库文件和头文件
install: all
	@echo "Installing SDK..."
	sudo cp $(STATIC_LIB) /usr/local/lib/
	sudo cp $(SHARED_LIB) /usr/local/lib/
	sudo cp $(INCLUDE_DIR)/linx_websocket_sdk.h /usr/local/include/
	sudo ldconfig
	@echo "SDK installed successfully"

# 卸载
uninstall:
	@echo "Uninstalling SDK..."
	sudo rm -f /usr/local/lib/liblinx_websocket_sdk.a
	sudo rm -f /usr/local/lib/liblinx_websocket_sdk.so
	sudo rm -f /usr/local/include/linx_websocket_sdk.h
	sudo ldconfig
	@echo "SDK uninstalled"

# 运行示例
run-example: $(EXAMPLE_BASIC)
	@echo "Running basic example..."
	./$(EXAMPLE_BASIC)

# 测试
test: examples
	@echo "Running tests..."
	# 这里可以添加测试脚本
	@echo "Tests completed"

# 清理
clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD_DIR)
	rm -rf $(LIB_DIR)
	@echo "Clean completed"

# 完全清理（包括生成的文件）
clean-all: clean
	rm -f received_audio.raw
	rm -f *.log

# 显示帮助
help:
	@echo "LINX WebSocket SDK Makefile"
	@echo "==========================="
	@echo ""
	@echo "Available targets:"
	@echo "  all          - Build static library, shared library and examples"
	@echo "  debug        - Build debug version"
	@echo "  examples     - Build example programs"
	@echo "  install      - Install SDK to system"
	@echo "  uninstall    - Remove SDK from system"
	@echo "  run-example  - Run basic example"
	@echo "  test         - Run tests"
	@echo "  clean        - Remove build files"
	@echo "  clean-all    - Remove all generated files"
	@echo "  help         - Show this help"
	@echo ""
	@echo "Configuration:"
	@echo "  CC           = $(CC)"
	@echo "  CFLAGS       = $(CFLAGS)"
	@echo "  LDFLAGS      = $(LDFLAGS)"
	@echo "  MONGOOSE_LIB = $(MONGOOSE_LIB)"
	@echo ""

# 显示SDK信息
info:
	@echo "LINX WebSocket SDK Information"
	@echo "============================="
	@echo "Version: 1.0.0"
	@echo "Standard: C99"
	@echo "Dependencies: Mongoose WebSocket library"
	@echo "Features:"
	@echo "  - WebSocket client implementation"
	@echo "  - Raw audio data callback (no OPUS encoding/decoding)"
	@echo "  - Automatic reconnection"
	@echo "  - Heartbeat mechanism"
	@echo "  - Error handling and logging"
	@echo "  - Thread-safe design"
	@echo ""
	@echo "Source files:"
	@for file in $(SDK_SOURCES); do echo "  $$file"; done
	@echo ""
	@echo "Header files:"
	@for file in $(SDK_HEADERS); do echo "  $$file"; done
	@echo ""

# 代码统计
stats:
	@echo "Code Statistics"
	@echo "==============="
	@echo "Source files:"
	@wc -l $(SDK_SOURCES) | tail -1
	@echo "Header files:"
	@wc -l $(SDK_HEADERS) | tail -1
	@echo "Example files:"
	@wc -l $(EXAMPLES_DIR)/*.c | tail -1
	@echo "Total lines:"
	@wc -l $(SDK_SOURCES) $(SDK_HEADERS) $(EXAMPLES_DIR)/*.c | tail -1

# 格式化代码（需要安装clang-format）
format:
	@echo "Formatting code..."
	@if command -v clang-format >/dev/null 2>&1; then \
		clang-format -i $(SDK_SOURCES) $(SDK_HEADERS) $(EXAMPLES_DIR)/*.c; \
		echo "Code formatted"; \
	else \
		echo "clang-format not found, skipping formatting"; \
	fi

# 检查代码（需要安装cppcheck）
check:
	@echo "Checking code..."
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=all --std=c99 $(SRC_DIR) $(EXAMPLES_DIR); \
	else \
		echo "cppcheck not found, skipping code check"; \
	fi

# 生成文档（需要安装doxygen）
docs:
	@echo "Generating documentation..."
	@if command -v doxygen >/dev/null 2>&1; then \
		doxygen Doxyfile; \
		echo "Documentation generated in docs/html/"; \
	else \
		echo "doxygen not found, skipping documentation generation"; \
	fi

# 打包发布
package: clean all
	@echo "Creating package..."
	@VERSION=$$(grep "#define LINX_VERSION" $(INCLUDE_DIR)/linx_websocket_sdk.h | cut -d'"' -f2); \
	tar -czf linx-websocket-sdk-$$VERSION.tar.gz \
		$(SRC_DIR) $(INCLUDE_DIR) $(EXAMPLES_DIR) Makefile README.md LICENSE
	@echo "Package created: linx-websocket-sdk-*.tar.gz"

.PHONY: all directories examples debug install uninstall run-example test clean clean-all help info stats format check docs package