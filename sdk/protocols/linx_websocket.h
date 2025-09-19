#ifndef LINX_WEBSOCKET_H
#define LINX_WEBSOCKET_H

#include "linx_protocol.h"
#include <stdbool.h>
#include <mongoose.h>

#ifdef __cplusplus
extern "C" {
#endif

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
    char* url;                      // WebSocket 服务器URL
    char* auth_token;               // 认证令牌
    char* device_id;                // 设备ID
    char* client_id;                // 客户端ID
    int protocol_version;           // 协议版本
} linx_websocket_config_t;

/* WebSocket 协议创建和销毁函数 */

/**
 * 创建 WebSocket 协议实例
 * @param config WebSocket 配置参数
 * @return 创建的协议实例，失败返回 NULL
 */
linx_websocket_protocol_t* linx_websocket_create(const linx_websocket_config_t* config);

/**
 * 销毁 WebSocket 协议实例 (vtable function)
 * @param protocol 要销毁的协议实例
 */
void linx_websocket_destroy(linx_protocol_t* protocol);

/**
 * 销毁 WebSocket 协议实例 (direct function)
 * @param protocol 要销毁的协议实例
 */
void linx_websocket_destroy_direct(linx_websocket_protocol_t* protocol);

/* WebSocket 协议操作函数 */

/* WebSocket 协议操作函数 - 这些是vtable函数，使用linx_protocol_t* */

/**
 * 启动 WebSocket 连接
 * @param protocol WebSocket 协议实例
 * @return 成功返回 true，失败返回 false
 */
bool linx_websocket_start(linx_protocol_t* protocol);





/**
 * 发送音频数据
 * @param protocol WebSocket 协议实例
 * @param packet 音频数据包
 * @return 成功返回 true，失败返回 false
 */
bool linx_websocket_send_audio(linx_protocol_t* protocol, 
                               linx_audio_stream_packet_t* packet);

/**
 * 发送文本消息
 * @param protocol WebSocket 协议实例
 * @param message 要发送的文本消息
 * @return 成功返回 true，失败返回 false
 */
bool linx_websocket_send_text(linx_protocol_t* protocol, const char* message);

/* WebSocket 特定函数 - 这些使用linx_websocket_protocol_t* */

/**
 * 停止 WebSocket 连接
 * @param protocol WebSocket 协议实例
 */
void linx_websocket_stop(linx_websocket_protocol_t* protocol);

/* WebSocket 状态查询函数 */

/**
 * 检查 WebSocket 是否已连接
 * @param protocol WebSocket 协议实例
 * @return 已连接返回 true，否则返回 false
 */
bool linx_websocket_is_connected(const linx_websocket_protocol_t* protocol);

/**
 * 获取重连尝试次数
 * @param protocol WebSocket 协议实例
 * @return 重连尝试次数
 */
int linx_websocket_get_reconnect_attempts(const linx_websocket_protocol_t* protocol);

/**
 * 重置重连尝试次数
 * @param protocol WebSocket 协议实例
 */
void linx_websocket_reset_reconnect_attempts(linx_websocket_protocol_t* protocol);

/* WebSocket 工具函数 */

/**
 * 处理 WebSocket 事件（需要在主循环中调用）
 * @param protocol WebSocket 协议实例
 */
void linx_websocket_process_events(linx_websocket_protocol_t* protocol);

/**
 * 发送 ping 消息
 * @param protocol WebSocket 协议实例
 * @return 成功返回 true，失败返回 false
 */
bool linx_websocket_send_ping(linx_websocket_protocol_t* protocol);

/**
 * 检查连接是否超时
 * @param protocol WebSocket 协议实例
 * @return 超时返回 true，否则返回 false
 */
bool linx_websocket_is_connection_timeout(const linx_websocket_protocol_t* protocol);

/* 配置函数 */
bool linx_websocket_protocol_set_server_url(linx_websocket_protocol_t* protocol, const char* url);
bool linx_websocket_protocol_set_server(linx_websocket_protocol_t* protocol, const char* host, int port, const char* path);
bool linx_websocket_protocol_set_auth_token(linx_websocket_protocol_t* protocol, const char* token);
bool linx_websocket_protocol_set_device_id(linx_websocket_protocol_t* protocol, const char* device_id);
bool linx_websocket_protocol_set_client_id(linx_websocket_protocol_t* protocol, const char* client_id);

/* Internal helper functions */
bool linx_websocket_parse_server_hello(linx_websocket_protocol_t* ws_protocol, const char* json_str);
char* linx_websocket_get_hello_message(linx_websocket_protocol_t* ws_protocol);

#ifdef __cplusplus
}
#endif

#endif /* LINX_WEBSOCKET_H */