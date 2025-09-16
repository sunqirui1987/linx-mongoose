# LINX WebSocket SDK 使用指南

## 概述

LINX WebSocket SDK 是一个基于 Mongoose 的 C99 标准 WebSocket 客户端库，专为实时音频数据传输和处理而设计。

## 主要特性

- **C99 标准兼容**：确保跨平台兼容性
- **基于 Mongoose**：稳定可靠的网络库
- **原始音频数据处理**：支持 PCM 格式音频数据
- **异步回调机制**：非阻塞的事件驱动架构
- **自动重连**：网络断开时自动重连
- **灵活配置**：支持多种音频参数和连接选项

## 快速开始

### 1. 编译项目

```bash
# 安装依赖
brew install mongoose

# 编译项目
make
```

### 2. 基本使用

```c
#include "linx_websocket_sdk.h"

// 回调函数实现
void on_connected(linx_context_t *ctx) {
    printf("Connected to server\n");
}

void on_audio_data(linx_context_t *ctx, const uint8_t *data, size_t len) {
    // 处理接收到的音频数据
    printf("Received audio data: %zu bytes\n", len);
}

int main() {
    // 配置SDK
    linx_config_t config = {0};
    strcpy(config.server_url, "ws://your-server.com:8080/ws");
    strcpy(config.device_id, "device_001");
    strcpy(config.token, "your_auth_token");
    
    // 音频配置
    config.audio = true;
    strcpy(config.audio_params.format, "pcm");
    config.audio_params.sample_rate = 16000;
    config.audio_params.channels = 1;
    config.audio_params.bits_per_sample = 16;
    
    // 设置回调
    config.callbacks.on_connected = on_connected;
    config.callbacks.on_audio_data = on_audio_data;
    
    // 初始化并连接
    if (linx_sdk_init(&config) == LINX_OK) {
        linx_start_listen(config.listen_mode);
        
        // 主循环
        while (running) {
            linx_poll_events(100);
        }
        
        linx_sdk_cleanup();
    }
    
    return 0;
}
```

## API 参考

### 核心函数

- `linx_sdk_init(const linx_config_t *config)` - 初始化SDK
- `linx_start_listen(linx_listen_mode_t mode)` - 开始监听
- `linx_poll_events(int timeout_ms)` - 轮询事件
- `linx_send_audio_data(const uint8_t *data, size_t len)` - 发送音频数据
- `linx_sdk_cleanup()` - 清理资源

### 回调函数

- `on_connected` - 连接成功
- `on_disconnected` - 连接断开
- `on_audio_data` - 接收音频数据
- `on_text_result` - 接收文本结果
- `on_audio_result` - 接收音频结果
- `on_error` - 错误处理

### 配置选项

```c
typedef struct {
    char server_url[256];           // 服务器URL
    char device_id[64];             // 设备ID
    char token[256];                // 认证令牌
    bool audio;                     // 启用音频
    linx_audio_params_t audio_params; // 音频参数
    linx_listen_mode_t listen_mode; // 监听模式
    linx_log_level_t log_level;     // 日志级别
    int connect_timeout_ms;         // 连接超时
    int heartbeat_interval_ms;      // 心跳间隔
    int max_reconnect_attempts;     // 最大重连次数
    linx_callbacks_t callbacks;    // 回调函数
} linx_config_t;
```

## 示例程序

项目包含完整的示例程序 `examples/basic_example.c`，演示了SDK的基本使用方法。

```bash
# 运行示例
./build/basic_example
```

## 注意事项

1. 确保服务器URL和认证信息正确
2. 音频数据格式必须与服务器端匹配
3. 在多线程环境中使用时需要注意线程安全
4. 及时处理错误回调，实现适当的错误恢复机制

## 故障排除

### 编译错误

- 确保安装了 Mongoose 库：`brew install mongoose`
- 检查编译器是否支持 C99 标准

### 连接失败

- 检查服务器URL是否正确
- 验证网络连接
- 确认认证令牌有效

### 音频数据问题

- 检查音频参数配置
- 确认数据格式匹配
- 验证采样率和声道数

## 更多信息

详细的技术文档请参考：
- [架构设计文档](../docs/linx_websocket_sdk_architecture.md)
- [需求规格说明](../docs/linx_websocket_sdk_requirements.md)
- [WebSocket接入文档](../docs/websocket接入.md)