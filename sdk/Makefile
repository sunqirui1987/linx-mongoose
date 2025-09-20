# LinxSdk Makefile
# 用于编译 LinxSdk 库和测试程序

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O2
INCLUDES = -I.
LDFLAGS = -lm

# 源文件
SDK_SOURCES = linx_sdk.c
TEST_SOURCES = test_linx_sdk.c

# 目标文件
SDK_OBJECTS = $(SDK_SOURCES:.c=.o)
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)

# 库文件
STATIC_LIB = liblinx_sdk.a
SHARED_LIB = liblinx_sdk.so

# 测试程序
TEST_PROGRAM = test_linx_sdk

# 默认目标
all: $(STATIC_LIB) $(SHARED_LIB) $(TEST_PROGRAM)

# 静态库
$(STATIC_LIB): $(SDK_OBJECTS)
	@echo "创建静态库 $@"
	ar rcs $@ $^

# 动态库
$(SHARED_LIB): $(SDK_OBJECTS)
	@echo "创建动态库 $@"
	$(CC) -shared -fPIC -o $@ $^ $(LDFLAGS)

# 测试程序
$(TEST_PROGRAM): $(TEST_OBJECTS) $(STATIC_LIB)
	@echo "编译测试程序 $@"
	$(CC) -o $@ $^ $(LDFLAGS)

# 编译规则
%.o: %.c
	@echo "编译 $<"
	$(CC) $(CFLAGS) $(INCLUDES) -fPIC -c $< -o $@

# 清理
clean:
	@echo "清理编译文件"
	rm -f $(SDK_OBJECTS) $(TEST_OBJECTS) $(STATIC_LIB) $(SHARED_LIB) $(TEST_PROGRAM)

# 安装
install: $(STATIC_LIB) $(SHARED_LIB)
	@echo "安装库文件"
	mkdir -p /usr/local/lib
	mkdir -p /usr/local/include
	cp $(STATIC_LIB) $(SHARED_LIB) /usr/local/lib/
	cp linx_sdk.h /usr/local/include/

# 运行测试
test: $(TEST_PROGRAM)
	@echo "运行测试程序"
	./$(TEST_PROGRAM)

# 帮助
help:
	@echo "可用目标:"
	@echo "  all       - 编译所有目标 (默认)"
	@echo "  static    - 编译静态库"
	@echo "  shared    - 编译动态库"
	@echo "  test      - 编译并运行测试程序"
	@echo "  clean     - 清理编译文件"
	@echo "  install   - 安装库文件到系统"
	@echo "  help      - 显示此帮助信息"

# 别名
static: $(STATIC_LIB)
shared: $(SHARED_LIB)

.PHONY: all clean install test help static shared