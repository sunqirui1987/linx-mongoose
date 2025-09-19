#ifndef LINX_WEBSOCKET_H
#define LINX_WEBSOCKET_H

#include "linx_protocol.h"
#include <stdbool.h>
#include <mongoose.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 音频参数配置常量 */
#define LINX_WEBSOCKET_AUDIO_FORMAT         "opus"
#define LINX_WEBSOCKET_AUDIO_SAMPLE_RATE    16000
#define LINX_WEBSOCKET_AUDIO_CHANNELS       1
#define LINX_WEBSOCKET_AUDIO_FRAME_DURATION 60

/* WebSocket 协议实现结构体 */
typedef struct {
    linx_protocol_t base;           // 基础协议结构体
    struct mg_mgr mgr;              // Mongoose 管理器
    struct mg_connection* conn;     // WebSocket 连接句柄
    bool connected;                 // 连接状态标志
    bool audio_channel_opened;      // 音频通道开启状态
    int version;                    // 协议版本
    bool server_hello_received;     // 服务器hello消息接收状态
    bool running;                   // 运行状态
    bool should_stop;               // 停止标志
    char* server_url;               // 服务器URL
    char* server_host;              // 服务器主机
    char* server_path;              // 服务器路径
    int server_port;                // 服务器端口
    char* session_id;               // 会话ID
    char* auth_token;               // 认证令牌
    char* device_id;                // 设备ID
    char* client_id;                // 客户端ID
    int server_sample_rate;         // 服务器采样率
    int server_frame_duration;      // 服务器帧持续时间
} linx_websocket_protocol_t;

/* WebSocket 配置结构体 */
typedef struct {
    const char* url;                // WebSocket 服务器URL
    const char* host;               // 服务器主机
    int port;                       // 服务器端口
    const char* path;               // 服务器路径
    const char* auth_token;         // 认证令牌
    const char* device_id;          // 设备ID
    const char* client_id;          // 客户端ID
    int protocol_version;           // 协议版本
} linx_websocket_config_t;

/* 核心接口函数 */

/**
 * 创建并初始化 WebSocket 协议实例
 * @param config WebSocket 配置参数
 * @return 创建的协议实例，失败返回 NULL
 */
linx_websocket_protocol_t* linx_websocket_protocol_create(const linx_websocket_config_t* config);

/* vtable 函数 */
bool linx_websocket_start(linx_protocol_t* protocol);
bool linx_websocket_send_audio(linx_protocol_t* protocol, linx_audio_stream_packet_t* packet);
bool linx_websocket_send_text(linx_protocol_t* protocol, const char* message);

/**
 * 销毁 WebSocket 协议实例
 * @param protocol 要销毁的协议实例
 */
void linx_websocket_destroy(linx_protocol_t* protocol);

/* 事件循环函数 */

/**
 * 轮询 WebSocket 事件
 * @param protocol WebSocket 协议实例
 * @param timeout_ms 超时时间（毫秒）
 */
void linx_websocket_poll(linx_websocket_protocol_t* protocol, int timeout_ms);

/**
 * 停止 WebSocket 连接
 * @param protocol WebSocket 协议实例
 */
void linx_websocket_stop(linx_websocket_protocol_t* protocol);

#ifdef __cplusplus
}
#endif

#endif /* LINX_WEBSOCKET_H */