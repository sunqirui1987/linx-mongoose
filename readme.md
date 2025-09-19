# Linx SDK

一个简单易用的 C 语言 SDK，用于与 Linx 服务器进行通信。

## 特性

- 🚀 **简单易用**: 只需几行代码即可开始使用
- 🔌 **多协议支持**: 支持 WebSocket 和 MQTT 协议
- 🎤 **音频处理**: 内置音频通道管理和音频数据传输
- 📨 **消息处理**: 支持 TTS、STT、LLM、MCP 等多种消息类型
- 🔧 **回调机制**: 灵活的事件回调系统
- 📦 **轻量级**: 头文件管理简洁，依赖最少

## 快速开始

### 1. 基本使用

```c
#include "linx_sdk.h"

// 回调函数
void on_connected(void* user_data) {
    printf("连接成功!\n");
}

void on_stt_result(const char* text, void* user_data) {
    printf("识别结果: %s\n", text);
}

int main() {
    // 创建配置
    linx_config_t config = linx_sdk_create_default_config(
        LINX_PROTOCOL_WEBSOCKET, 
        "ws://localhost:8080/ws"
    );

    // 创建 SDK 实例
    linx_sdk_t* sdk = linx_sdk_create(&config);

    // 设置回调
    linx_callbacks_t callbacks = linx_sdk_create_empty_callbacks();
    callbacks.on_connected = on_connected;
    callbacks.on_stt_result = on_stt_result;
    linx_sdk_set_callbacks(sdk, &callbacks);

    // 启动
    linx_sdk_start(sdk);

    // 开启音频通道
    linx_sdk_open_audio_channel(sdk);

    // 开始监听
    linx_sdk_send_start_listening(sdk, LINX_LISTENING_MODE_AUTO_STOP);

    // ... 你的业务逻辑 ...

    // 清理
    linx_sdk_destroy(sdk);
    return 0;
}
```

### 2. 编译和运行

```bash
# 编译 SDK 库
make all

# 编译示例程序
make example

# 运行示例
./example_usage
```

## API 参考

### 核心函数

| 函数 | 描述 |
|------|------|
| `linx_sdk_create()` | 创建 SDK 实例 |
| `linx_sdk_destroy()` | 销毁 SDK 实例 |
| `linx_sdk_set_callbacks()` | 设置回调函数 |
| `linx_sdk_start()` | 启动 SDK |
| `linx_sdk_stop()` | 停止 SDK |

### 音频管理

| 函数 | 描述 |
|------|------|
| `linx_sdk_open_audio_channel()` | 开启音频通道 |
| `linx_sdk_close_audio_channel()` | 关闭音频通道 |
| `linx_sdk_send_audio()` | 发送音频数据 |
| `linx_sdk_is_audio_channel_opened()` | 检查音频通道状态 |

### 消息发送

| 函数 | 描述 |
|------|------|
| `linx_sdk_send_wake_word_detected()` | 发送唤醒词检测 |
| `linx_sdk_send_start_listening()` | 开始监听 |
| `linx_sdk_send_stop_listening()` | 停止监听 |
| `linx_sdk_send_abort_speaking()` | 中止播放 |
| `linx_sdk_send_text_message()` | 发送文本消息 |
| `linx_sdk_send_mcp_message()` | 发送 MCP 消息 |

### 状态查询

| 函数 | 描述 |
|------|------|
| `linx_sdk_get_server_sample_rate()` | 获取服务器采样率 |
| `linx_sdk_get_server_frame_duration()` | 获取服务器帧时长 |
| `linx_sdk_get_session_id()` | 获取会话 ID |
| `linx_sdk_has_error()` | 检查是否有错误 |

## 回调事件

SDK 支持以下回调事件：

- **连接事件**: `on_connected`, `on_disconnected`, `on_network_error`
- **音频事件**: `on_audio_channel_opened`, `on_audio_channel_closed`, `on_incoming_audio`
- **TTS 事件**: `on_tts_start`, `on_tts_stop`, `on_tts_sentence`
- **STT 事件**: `on_stt_result`
- **LLM 事件**: `on_llm_emotion`
- **MCP 事件**: `on_mcp_message`
- **系统事件**: `on_system_command`, `on_alert`
- **自定义事件**: `on_custom_message`

## 协议支持

- **WebSocket**: 实时双向通信
- **MQTT**: 轻量级消息传输（开发中）

## 依赖

- `libcjson`: JSON 解析库
- `pthread`: 线程支持

## 编译选项

```bash
# 调试版本
make CFLAGS="-g -DDEBUG"

# 发布版本
make CFLAGS="-O2 -DNDEBUG"

# 清理
make clean
```

## 许可证

MIT License

## 贡献

欢迎提交 Issue 和 Pull Request！