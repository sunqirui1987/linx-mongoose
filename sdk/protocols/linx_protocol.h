#ifndef LINX_PROTOCOL_H
#define LINX_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <cJSON.h>
#include "../log/linx_log.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 音频流数据包结构 */
typedef struct {
    int sample_rate;        // 采样率
    int frame_duration;     // 帧持续时间
    uint32_t timestamp;     // 时间戳
    uint8_t* payload;       // 音频数据载荷
    size_t payload_size;    // 载荷大小
} linx_audio_stream_packet_t;

/* 二进制协议 v2 结构 */
typedef struct __attribute__((packed)) {
    uint16_t version;       // 协议版本
    uint16_t type;          // 消息类型 (0: OPUS, 1: JSON)
    uint32_t reserved;      // 保留字段，供将来使用
    uint32_t timestamp;     // 时间戳（毫秒），用于服务端回声消除
    uint32_t payload_size;  // 载荷大小（字节）
    uint8_t payload[];      // 载荷数据
} linx_binary_protocol2_t;

/* 二进制协议 v3 结构 */
typedef struct __attribute__((packed)) {
    uint8_t type;           // 消息类型
    uint8_t reserved;       // 保留字段
    uint16_t payload_size;  // 载荷大小
    uint8_t payload[];      // 载荷数据
} linx_binary_protocol3_t;

/* 中止原因枚举 */
typedef enum {
    LINX_ABORT_REASON_NONE,                 // 无特定原因
    LINX_ABORT_REASON_WAKE_WORD_DETECTED    // 检测到唤醒词
} linx_abort_reason_t;

/* 监听模式枚举 */
typedef enum {
    LINX_LISTENING_MODE_AUTO_STOP,      // 自动停止模式
    LINX_LISTENING_MODE_MANUAL_STOP,    // 手动停止模式
    LINX_LISTENING_MODE_REALTIME        // 实时模式（需要回声消除支持）
} linx_listening_mode_t;

/* 前向声明 */
typedef struct linx_protocol linx_protocol_t;

/* 回调函数类型定义 */
typedef void (*linx_on_incoming_audio_cb_t)(linx_audio_stream_packet_t* packet, void* user_data);      // 接收音频回调
typedef void (*linx_on_incoming_json_cb_t)(const cJSON* root, void* user_data);                       // 接收JSON回调
typedef void (*linx_on_network_error_cb_t)(const char* message, void* user_data);                     // 网络错误回调
typedef void (*linx_on_connected_cb_t)(void* user_data);                                              // 连接成功回调
typedef void (*linx_on_disconnected_cb_t)(void* user_data);                                           // 连接断开回调

/* 协议接口结构（虚函数表） */
typedef struct {
    bool (*start)(linx_protocol_t* protocol);                                                          // 启动协议
    bool (*open_audio_channel)(linx_protocol_t* protocol);                                             // 打开音频通道
    void (*close_audio_channel)(linx_protocol_t* protocol);                                            // 关闭音频通道                               // 检查音频通道是否打开
    bool (*send_audio)(linx_protocol_t* protocol, linx_audio_stream_packet_t* packet);                 // 发送音频数据
    bool (*send_text)(linx_protocol_t* protocol, const char* text);                                    // 发送文本数据
    void (*destroy)(linx_protocol_t* protocol);                                                        // 销毁协议实例
} linx_protocol_vtable_t;

/* 协议基础结构 */
struct linx_protocol {
    const linx_protocol_vtable_t* vtable;   // 虚函数表指针
    
    /* 回调函数 */
    linx_on_incoming_json_cb_t on_incoming_json;                    // JSON消息接收回调
    linx_on_incoming_audio_cb_t on_incoming_audio;                  // 音频数据接收回调
    linx_on_network_error_cb_t on_network_error;                    // 网络错误回调
    linx_on_connected_cb_t on_connected;                            // 连接成功回调
    linx_on_disconnected_cb_t on_disconnected;                      // 连接断开回调
    
    /* 回调函数的用户数据 */
    void* user_data;
    
    /* 协议状态 */
    int server_sample_rate;         // 服务器采样率
    int server_frame_duration;      // 服务器帧持续时间
    bool error_occurred;            // 是否发生错误
    char* session_id;               // 会话ID
    uint64_t last_incoming_time;    // 最后接收数据的时间戳（毫秒）
};

/* 协议管理函数 */
void linx_protocol_init(linx_protocol_t* protocol, const linx_protocol_vtable_t* vtable);    // 初始化协议
void linx_protocol_destroy(linx_protocol_t* protocol);                                       // 销毁协议

/* 获取器函数 */
int linx_protocol_get_server_sample_rate(const linx_protocol_t* protocol);      // 获取服务器采样率
int linx_protocol_get_server_frame_duration(const linx_protocol_t* protocol);   // 获取服务器帧持续时间
const char* linx_protocol_get_session_id(const linx_protocol_t* protocol);      // 获取会话ID

/* 回调函数注册 */
void linx_protocol_set_on_incoming_audio(linx_protocol_t* protocol, 
                                         linx_on_incoming_audio_cb_t callback, 
                                         void* user_data);                       // 设置音频接收回调

void linx_protocol_set_on_incoming_json(linx_protocol_t* protocol, 
                                        linx_on_incoming_json_cb_t callback, 
                                        void* user_data);                        // 设置JSON接收回调



void linx_protocol_set_on_network_error(linx_protocol_t* protocol, 
                                        linx_on_network_error_cb_t callback, 
                                        void* user_data);                        // 设置网络错误回调

void linx_protocol_set_on_connected(linx_protocol_t* protocol, 
                                    linx_on_connected_cb_t callback, 
                                    void* user_data);                            // 设置连接成功回调

void linx_protocol_set_on_disconnected(linx_protocol_t* protocol, 
                                       linx_on_disconnected_cb_t callback, 
                                       void* user_data);                         // 设置连接断开回调

/* 协议操作函数 */
bool linx_protocol_start(linx_protocol_t* protocol);                                            // 启动协议    
bool linx_protocol_send_audio(linx_protocol_t* protocol, linx_audio_stream_packet_t* packet);  // 发送音频数据

/* 高级消息发送函数 */
void linx_protocol_send_wake_word_detected(linx_protocol_t* protocol, const char* wake_word);          // 发送唤醒词检测消息
void linx_protocol_send_start_listening(linx_protocol_t* protocol, linx_listening_mode_t mode);        // 发送开始监听消息
void linx_protocol_send_stop_listening(linx_protocol_t* protocol);                                     // 发送停止监听消息
void linx_protocol_send_abort_speaking(linx_protocol_t* protocol, linx_abort_reason_t reason);         // 发送中止说话消息
void linx_protocol_send_mcp_message(linx_protocol_t* protocol, const char* message);                  // 发送MCP消息

/* 工具函数 */
void linx_protocol_set_error(linx_protocol_t* protocol, const char* message);      // 设置错误状态
bool linx_protocol_is_timeout(const linx_protocol_t* protocol);                    // 检查是否超时

/* 音频数据包管理 */
linx_audio_stream_packet_t* linx_audio_stream_packet_create(size_t payload_size);  // 创建音频数据包
void linx_audio_stream_packet_destroy(linx_audio_stream_packet_t* packet);         // 销毁音频数据包

#ifdef __cplusplus
}
#endif

#endif /* LINX_PROTOCOL_H */