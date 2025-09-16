# LINX WebSocket SDK

基于Mongoose的C99标准WebSocket客户端SDK，专为音频数据传输设计。SDK不进行音频编解码处理，只负责通过回调函数将接收到的原始音频数据传递给上层应用。

## 特性

- ✅ **C99标准兼容** - 支持所有符合C99标准的编译器
- ✅ **原始音频数据回调** - 不进行OPUS编解码，直接传递原始音频数据
- ✅ **WebSocket客户端** - 基于Mongoose库的高性能WebSocket实现
- ✅ **自动重连机制** - 支持指数退避的自动重连
- ✅ **心跳检测** - 内置心跳机制确保连接稳定性
- ✅ **错误处理** - 完善的错误处理和日志系统
- ✅ **线程安全** - 支持多线程环境使用
- ✅ **内存管理** - 静态内存分配，适合嵌入式环境

## 系统要求

- C99兼容的编译器（GCC 4.9+, Clang 3.5+）
- Mongoose WebSocket库
- POSIX兼容系统（Linux, macOS, Unix）
- 最小内存要求：512KB

## 快速开始

### 1. 安装依赖

```bash
# 安装Mongoose库（Ubuntu/Debian）
sudo apt-get install libmongoose-dev

# 或者从源码编译Mongoose
git clone https://github.com/cesanta/mongoose.git
cd mongoose
make
sudo make install
```

### 2. 编译SDK

```bash
# 克隆项目
git clone <repository-url>
cd linx-mongoose

# 编译SDK和示例
make all

# 或者编译调试版本
make debug
```

### 3. 运行示例

```bash
# 运行基本示例
make run-example

# 或者直接运行
./build/basic_example
```

## API 文档

### 核心函数

#### 初始化和清理

```c
/**
 * 初始化SDK
 * @return LINX_OK 成功，其他值表示错误
 */
int linx_init(void);

/**
 * 清理SDK资源
 */
void linx_cleanup(void);
```

#### 配置管理

```c
/**
 * 设置SDK配置
 * @param config 配置结构体指针
 * @return LINX_OK 成功，其他值表示错误
 */
int linx_set_config(const linx_config_t *config);

/**
 * 获取当前配置
 * @param config 用于接收配置的结构体指针
 * @return LINX_OK 成功，其他值表示错误
 */
int linx_get_config(linx_config_t *config);
```

#### 连接管理

```c
/**
 * 连接到服务器
 * @return LINX_OK 成功，其他值表示错误
 */
int linx_connect(void);

/**
 * 断开连接
 * @return LINX_OK 成功，其他值表示错误
 */
int linx_disconnect(void);

/**
 * 获取连接状态
 * @return 当前连接状态
 */
linx_connection_state_t linx_get_connection_state(void);
```

#### 音频处理

```c
/**
 * 发送原始音频数据
 * @param data 音频数据指针
 * @param len 数据长度
 * @return LINX_OK 成功，其他值表示错误
 */
int linx_send_audio(const uint8_t *data, size_t len);

/**
 * 开始音频流
 * @return LINX_OK 成功，其他值表示错误
 */
int linx_start_audio_stream(void);

/**
 * 停止音频流
 * @return LINX_OK 成功，其他值表示错误
 */
int linx_stop_audio_stream(void);
```

#### 事件处理

```c
/**
 * 轮询网络事件
 * @param timeout_ms 超时时间（毫秒）
 * @return LINX_OK 成功，LINX_ERROR_TIMEOUT 超时，其他值表示错误
 */
int linx_poll(int timeout_ms);
```

### 数据结构

#### 音频参数

```c
typedef struct {
    char format[16];           // 音频格式："pcm" 或 "raw"
    int sample_rate;           // 采样率（Hz）
    int channels;              // 声道数
    int bits_per_sample;       // 位深度
} linx_audio_params_t;
```

#### 回调函数

```c
typedef struct {
    void (*on_connected)(linx_context_t *ctx);
    void (*on_disconnected)(linx_context_t *ctx, int reason);
    void (*on_ready)(linx_context_t *ctx);
    void (*on_audio_start)(linx_context_t *ctx);
    void (*on_audio_end)(linx_context_t *ctx);
    void (*on_audio_data)(linx_context_t *ctx, const uint8_t *data, size_t len);  // 核心回调
    void (*on_text_result)(linx_context_t *ctx, const char *text);
    void (*on_audio_result)(linx_context_t *ctx, const uint8_t *data, size_t len);
    void (*on_error)(linx_context_t *ctx, int error_code, const char *error_msg);
} linx_callbacks_t;
```

#### SDK配置

```c
typedef struct {
    char server_url[256];           // 服务器URL
    char device_id[64];             // 设备ID
    char client_id[64];             // 客户端ID
    char access_token[512];         // 访问令牌（可选）
    linx_audio_params_t audio_params; // 音频参数
    linx_listen_mode_t listen_mode;   // 监听模式
    int reconnect_max_times;        // 最大重连次数
    int heartbeat_interval;         // 心跳间隔（秒）
    linx_log_level_t log_level;     // 日志级别
    linx_callbacks_t callbacks;    // 回调函数
} linx_config_t;
```

## 使用示例

### 基本使用

```c
#include "linx_websocket_sdk.h"

// 音频数据回调函数
void on_audio_data(linx_context_t *ctx, const uint8_t *data, size_t len) {
    printf("Received audio data: %zu bytes\n", len);
    // 在这里处理接收到的原始音频数据
    // 例如：播放音频、保存到文件、进行音频处理等
}

int main() {
    // 初始化SDK
    linx_init();
    
    // 配置SDK
    linx_config_t config = {0};
    strcpy(config.server_url, "wss://your-server.com/ws");
    strcpy(config.device_id, "device_001");
    strcpy(config.client_id, "client_001");
    
    // 音频参数（PCM格式）
    strcpy(config.audio_params.format, "pcm");
    config.audio_params.sample_rate = 16000;
    config.audio_params.channels = 1;
    config.audio_params.bits_per_sample = 16;
    
    // 设置回调函数
    config.callbacks.on_audio_data = on_audio_data;
    
    linx_set_config(&config);
    
    // 连接服务器
    linx_connect();
    
    // 主循环
    while (running) {
        linx_poll(100);
        
        // 发送音频数据
        uint8_t audio_data[1024];
        // ... 填充音频数据 ...
        linx_send_audio(audio_data, sizeof(audio_data));
    }
    
    // 清理
    linx_disconnect();
    linx_cleanup();
    
    return 0;
}
```

### 高级使用

```c
// 完整的回调函数设置
void setup_callbacks(linx_config_t *config) {
    config->callbacks.on_connected = [](linx_context_t *ctx) {
        printf("Connected to server\n");
    };
    
    config->callbacks.on_ready = [](linx_context_t *ctx) {
        printf("SDK ready, starting audio stream\n");
        linx_start_audio_stream();
    };
    
    config->callbacks.on_audio_data = [](linx_context_t *ctx, const uint8_t *data, size_t len) {
        // 处理接收到的原始音频数据
        process_received_audio(data, len);
    };
    
    config->callbacks.on_error = [](linx_context_t *ctx, int code, const char *msg) {
        printf("Error: %d - %s\n", code, msg);
    };
}
```

## 错误处理

### 错误码

```c
#define LINX_OK                     0
#define LINX_ERROR_INVALID_PARAM   -1
#define LINX_ERROR_MEMORY          -2
#define LINX_ERROR_NETWORK         -3
#define LINX_ERROR_NOT_CONNECTED   -4
#define LINX_ERROR_NOT_READY       -5
#define LINX_ERROR_HANDSHAKE       -6
#define LINX_ERROR_AUDIO           -7
// ... 更多错误码
```

### 错误处理示例

```c
int ret = linx_connect();
if (ret != LINX_OK) {
    linx_error_info_t error;
    linx_get_last_error(&error);
    printf("Connection failed: %s\n", error.message);
    return -1;
}
```

## 编译选项

### Makefile目标

```bash
make all          # 编译所有目标
make debug        # 编译调试版本
make examples     # 编译示例程序
make install      # 安装到系统
make clean        # 清理编译文件
make test         # 运行测试
make docs         # 生成文档
```

### 编译器选项

```bash
# 基本编译
gcc -std=c99 -Wall -Wextra -O2 -I./include -L./lib -llinx_websocket_sdk -lmongoose -lm -lpthread

# 调试编译
gcc -std=c99 -Wall -Wextra -g -O0 -DDEBUG -I./include -L./lib -llinx_websocket_sdk -lmongoose -lm -lpthread
```

## 性能优化

### 内存使用

- 静态内存分配，避免频繁的malloc/free
- 音频缓冲区大小可配置
- 消息队列大小限制，防止内存泄漏

### 网络优化

- 支持WebSocket压缩（如果服务器支持）
- 自适应重连间隔
- 心跳机制减少不必要的网络检测

### 音频处理

- 零拷贝音频数据传递
- 支持多种音频格式（PCM、RAW）
- 可配置的音频参数

## 故障排除

### 常见问题

1. **连接失败**
   - 检查服务器URL是否正确
   - 确认网络连接正常
   - 验证访问令牌（如果需要）

2. **音频数据丢失**
   - 检查网络带宽
   - 调整音频缓冲区大小
   - 确认回调函数处理速度

3. **内存泄漏**
   - 确保调用linx_cleanup()
   - 检查回调函数中的内存管理
   - 使用valgrind等工具检测

### 调试技巧

```c
// 启用调试日志
config.log_level = LINX_LOG_DEBUG;

// 检查错误信息
linx_error_info_t error;
if (linx_get_last_error(&error) == LINX_OK) {
    printf("Last error: %s\n", error.message);
}

// 获取连接状态
linx_connection_state_t state = linx_get_connection_state();
printf("Connection state: %s\n", linx_get_state_string(state));
```

## 许可证

MIT License - 详见 [LICENSE](LICENSE) 文件

## 贡献

欢迎提交Issue和Pull Request！

## 更新日志

### v1.0.0
- 初始版本发布
- 支持原始音频数据回调
- 基于Mongoose的WebSocket实现
- 完整的错误处理和日志系统

## 联系方式

- 项目主页：<repository-url>
- 问题反馈：<repository-url>/issues
- 邮箱：<contact-email>