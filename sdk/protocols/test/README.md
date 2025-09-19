# linx protocols 单元测试

本目录包含 linx protocols 模块的单元测试，用于验证协议实现的正确性和稳定性。

## 文件结构

```
test/
├── README.md                   # 本说明文档
├── Makefile                    # 编译配置文件
├── run_tests.sh               # 测试运行脚本
├── test_linx_protocol.c       # linx_protocol.c 的单元测试
├── test_linx_websocket.c      # linx_websocket.c 的单元测试
├── examples/                  # 使用示例目录
│   ├── README.md              # 示例说明文档
│   ├── Makefile               # 示例编译配置
│   ├── run_examples.sh        # 示例运行脚本
│   ├── example_linx_protocol.c    # linx_protocol 使用示例
│   ├── example_linx_websocket.c   # linx_websocket 使用示例
│   └── build/                 # 示例编译输出目录
└── build/                     # 测试编译输出目录（自动创建）
```

## 测试内容

### test_linx_protocol.c
测试 `linx_protocol.c` 中的基础协议功能：
- 协议初始化和销毁
- 获取器函数（采样率、帧持续时间、会话ID等）
- 回调函数设置
- 协议操作（启动、音频通道管理、数据发送等）
- 音频数据包管理
- 错误处理
- 协议消息发送

### test_linx_websocket.c
测试 `linx_websocket.c` 中的 WebSocket 协议功能：
- WebSocket 协议创建和销毁
- 配置函数（URL、认证令牌、设备ID等）
- 状态查询（连接状态、重连次数等）
- 协议操作（事件处理、ping发送等）
- 虚函数表调用
- Hello 消息处理
- 回调设置

## 依赖要求

### 必需依赖
- **gcc** - C 编译器
- **make** - 构建工具
- **cJSON** - JSON 解析库（项目内置）

### 可选依赖
- **mongoose** - WebSocket 库（用于 WebSocket 测试）
  - macOS: `brew install mongoose`
  - Ubuntu/Debian: `sudo apt-get install libmongoose-dev`

## 使用示例

### 快速开始

在 `examples/` 目录中提供了完整的使用示例，展示如何在实际项目中使用 linx 协议：

```bash
# 进入示例目录
cd examples/

# 运行所有示例
./run_examples.sh

# 仅运行 protocol 示例
./run_examples.sh -p

# 仅运行 websocket 示例
./run_examples.sh -w

# 查看帮助
./run_examples.sh -h
```

### 示例内容

#### example_linx_protocol.c
展示 `linx_protocol` 的基本使用方法：
- 协议实例创建和配置
- 回调函数设置
- 音频数据处理流程
- 协议消息发送
- 资源清理

#### example_linx_websocket.c
展示 `linx_websocket` 的完整使用流程：
- WebSocket 连接配置
- 认证和会话管理
- 实时消息处理
- 音频数据传输
- 连接状态监控

### 示例特点

- 🎯 **实用性** - 基于真实使用场景设计
- 📝 **详细注释** - 每个步骤都有清晰说明
- 🔧 **易于修改** - 可作为项目模板使用
- 🎨 **友好输出** - 彩色输出和进度提示
- 🛠️ **完整工具链** - 包含编译、运行、清理脚本

## 运行测试

### 使用测试脚本（推荐）

```bash
# 运行所有测试
./run_tests.sh

# 仅运行 protocol 测试
./run_tests.sh -p

# 仅运行 websocket 测试
./run_tests.sh -w

# 检查依赖
./run_tests.sh -d

# 清理构建文件
./run_tests.sh -c

# 显示帮助
./run_tests.sh -h
```

### 使用 Makefile

```bash
# 编译所有测试
make all

# 运行所有测试
make test

# 运行单个测试
make test-protocol
make test-websocket

# 清理构建文件
make clean

# 检查依赖
make check-deps

# 显示帮助
make help
```

### 手动编译和运行

```bash
# 创建构建目录
mkdir -p build

# 编译 protocol 测试
gcc -Wall -Wextra -std=c99 -g -O0 -I.. -I../../cjson -o build/test_linx_protocol \
    test_linx_protocol.c ../linx_protocol.c ../../cjson/cJSON.c ../../cjson/cJSON_Utils.c -lm

# 编译 websocket 测试（需要 mongoose）
gcc -Wall -Wextra -std=c99 -g -O0 -I.. -I../../cjson -o build/test_linx_websocket \
    test_linx_websocket.c ../linx_websocket.c ../linx_protocol.c \
    ../../cjson/cJSON.c ../../cjson/cJSON_Utils.c -lm -lmongoose

# 运行测试
./build/test_linx_protocol
./build/test_linx_websocket
```

## 测试输出

测试运行时会显示详细的测试结果：

```
开始运行 linx_protocol 单元测试...

=== 测试协议初始化和销毁 ===
✓ 协议虚函数表设置正确
✓ 默认采样率设置正确
✓ 默认帧持续时间设置正确
...

=== 测试结果 ===
总测试数: 45
通过测试: 45
失败测试: 0
✓ 所有测试通过!
```

## 故障排除

### 编译错误

1. **找不到头文件**
   - 确保在正确的目录下运行测试
   - 检查相对路径是否正确

2. **mongoose 库未找到**
   - 安装 mongoose 库或跳过 WebSocket 测试
   - 检查库的安装路径

3. **cJSON 相关错误**
   - 确保 cJSON 源文件存在于 `../../cjson/` 目录

### 运行时错误

1. **段错误（Segmentation fault）**
   - 检查空指针处理
   - 使用 gdb 调试：`gdb ./build/test_linx_protocol`

2. **测试失败**
   - 查看具体的失败信息
   - 检查被测试的代码是否有问题

## 添加新测试

要添加新的测试用例：

1. 在相应的测试文件中添加新的测试函数
2. 在 `main()` 函数中调用新的测试函数
3. 使用 `TEST_ASSERT` 宏进行断言检查
4. 更新本文档说明新增的测试内容

## 测试覆盖率

当前测试覆盖了以下功能：
- ✅ 协议基础功能
- ✅ 回调机制
- ✅ 错误处理
- ✅ 内存管理
- ✅ WebSocket 配置
- ✅ 消息处理
- ⚠️ 网络连接（模拟测试）
- ⚠️ 并发场景（待添加）

## 贡献指南

1. 确保新增的测试用例有清晰的描述
2. 测试应该覆盖正常情况和边界情况
3. 添加适当的错误处理测试
4. 保持测试的独立性，避免测试间的依赖
5. 更新相关文档