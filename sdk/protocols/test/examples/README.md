# linx 协议使用示例

本目录包含 linx 协议的完整使用示例，展示如何在实际项目中集成和使用 linx_protocol 和 linx_websocket。

## 📁 目录结构

```
examples/
├── README.md                    # 本说明文档
├── Makefile                     # 示例编译配置
├── run_examples.sh             # 示例运行脚本
├── example_linx_protocol.c     # linx_protocol 使用示例
├── example_linx_websocket.c    # linx_websocket 使用示例
└── build/                      # 编译输出目录（自动创建）
```

## 🚀 快速开始

### 运行所有示例
```bash
./run_examples.sh
```

### 运行特定示例
```bash
# 仅运行 protocol 示例
./run_examples.sh -p

# 仅运行 websocket 示例  
./run_examples.sh -w

# 仅编译不运行
./run_examples.sh -b

# 检查依赖
./run_examples.sh -d
```

### 使用 Makefile
```bash
# 编译所有示例
make all

# 运行 protocol 示例
make run-protocol

# 运行 websocket 示例
make run-websocket

# 运行所有示例
make run-all

# 清理构建文件
make clean
```

## 📖 示例说明

### example_linx_protocol.c

这个示例展示了 `linx_protocol` 的完整使用流程：

#### 🔧 主要功能
- **协议初始化** - 创建和配置协议实例
- **回调设置** - 设置音频数据、事件和消息回调
- **协议信息** - 获取采样率、帧持续时间、会话ID
- **音频通道** - 打开/关闭音频通道，发送音频数据
- **数据包管理** - 创建、使用和销毁音频数据包
- **消息发送** - 发送协议消息
- **资源清理** - 正确释放所有资源

#### 🎯 学习要点
- 如何正确初始化和销毁协议
- 回调函数的设置和使用
- 音频数据的处理流程
- 错误处理的最佳实践

### example_linx_websocket.c

这个示例展示了 `linx_websocket` 的完整使用流程：

#### 🔧 主要功能
- **连接配置** - 设置服务器URL、认证令牌、设备ID等
- **回调设置** - 设置连接、断开、错误、消息和音频回调
- **连接管理** - 启动连接、状态检查、重连处理
- **Hello消息** - 生成、发送和解析Hello消息
- **音频传输** - 音频通道管理和数据传输
- **消息通信** - 发送和接收自定义消息
- **状态监控** - 连接状态和重连次数监控

#### 🎯 学习要点
- WebSocket 连接的配置和管理
- 认证和会话处理
- 实时消息和音频数据传输
- 连接状态监控和错误处理

## 🛠️ 编译要求

### 必需依赖
- **gcc** - C 编译器
- **make** - 构建工具

### 可选依赖
- **mongoose** - WebSocket 库（WebSocket 示例需要）
- **cjson** - JSON 解析库（项目内置）

### 安装依赖

#### macOS
```bash
# 安装 mongoose
brew install mongoose

# 或者使用 pkg-config
brew install pkg-config
```

#### Ubuntu/Debian
```bash
# 安装编译工具
sudo apt-get update
sudo apt-get install build-essential

# 安装 mongoose
sudo apt-get install libmongoose-dev

# 安装 pkg-config
sudo apt-get install pkg-config
```

## 🎨 示例输出

运行示例时会看到彩色的详细输出：

```
🚀 linx_protocol 使用示例
========================

1️⃣ 创建协议实例...
✅ 协议创建成功

2️⃣ 设置回调函数...
✅ 回调函数设置完成

3️⃣ 协议信息:
   📊 采样率: 16000 Hz
   ⏱️  帧持续时间: 20 ms
   🆔 会话ID: session-12345

...

🎉 示例运行完成！
========================
```

## 🔧 自定义和扩展

### 修改示例参数

你可以轻松修改示例中的参数来适应你的需求：

```c
// 修改 WebSocket URL
linx_websocket_set_url(ws, "wss://your-server.com/websocket");

// 修改认证令牌
linx_websocket_set_auth_token(ws, "your-auth-token");

// 修改设备信息
linx_websocket_set_device_id(ws, "your-device-id");
```

### 添加自定义回调

```c
// 自定义音频数据处理
static void my_audio_callback(const char* data, size_t size, void* user_data) {
    // 在这里添加你的音频处理逻辑
    printf("处理音频数据: %zu 字节\n", size);
}

// 设置自定义回调
linx_protocol_set_audio_callback(protocol, my_audio_callback, NULL);
```

### 集成到项目

这些示例可以作为模板集成到你的项目中：

1. 复制相关的示例代码
2. 根据项目需求修改配置参数
3. 添加项目特定的业务逻辑
4. 集成到项目的构建系统

## 🐛 故障排除

### 编译问题

1. **找不到头文件**
   ```bash
   # 确保在正确目录运行
   pwd  # 应该在 examples/ 目录
   ```

2. **mongoose 库未找到**
   ```bash
   # 检查库是否安装
   pkg-config --exists libmongoose
   
   # 或手动指定库路径
   export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
   ```

### 运行问题

1. **权限错误**
   ```bash
   # 确保脚本有执行权限
   chmod +x run_examples.sh
   ```

2. **段错误**
   ```bash
   # 使用调试模式编译
   make clean && make CFLAGS="-g -O0"
   
   # 使用 gdb 调试
   gdb ./build/example_linx_protocol
   ```

## 📚 进一步学习

- 查看 `../test_*.c` 文件了解单元测试
- 阅读 `../../../protocols/` 目录中的源代码
- 参考项目根目录的文档

## 🤝 贡献

欢迎提交改进建议和新的示例：

1. 确保示例代码清晰易懂
2. 添加充分的注释说明
3. 测试示例在不同环境下的运行
4. 更新相关文档

---

💡 **提示**: 这些示例是学习 linx 协议的最佳起点，建议从 `example_linx_protocol.c` 开始！