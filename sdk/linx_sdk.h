#ifndef LINX_SDK_H
#define LINX_SDK_H

/*
 * Linx SDK - 简化的协议处理和消息管理 SDK
 * 提供友好的 API 接口，封装复杂的协议处理逻辑
 */

#include <stdbool.h>
#include <stdint.h>
#include "cjson/cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 前向声明 */
typedef struct linx_sdk linx_sdk_t;

/* 协议类型 */
typedef enum {
    LINX_PROTOCOL_MQTT = 0,
    LINX_PROTOCOL_WEBSOCKET = 1
} linx_protocol_type_t;

/* 监听模式 */
typedef enum {
    LINX_LISTENING_MODE_MANUAL_STOP = 0,
    LINX_LISTENING_MODE_AUTO_STOP = 1,
    LINX_LISTENING_MODE_REALTIME = 2
} linx_listening_mode_t;

/* 中止原因 */
typedef enum {
    LINX_ABORT_REASON_USER_REQUEST = 0,
    LINX_ABORT_REASON_WAKE_WORD_DETECTED = 1
} linx_abort_reason_t;

/* 音频流数据包 */
typedef struct {
    uint8_t* data;
    size_t size;
    uint64_t timestamp;
} linx_audio_packet_t;

/* 回调函数类型定义 */
typedef void (*linx_on_connected_cb_t)(void* user_data);
typedef void (*linx_on_disconnected_cb_t)(void* user_data);
typedef void (*linx_on_network_error_cb_t)(const char* error_message, void* user_data);
typedef void (*linx_on_audio_channel_opened_cb_t)(void* user_data);
typedef void (*linx_on_audio_channel_closed_cb_t)(void* user_data);
typedef void (*linx_on_incoming_audio_cb_t)(const linx_audio_packet_t* packet, void* user_data);

/* 消息处理回调函数 */
typedef void (*linx_on_tts_start_cb_t)(void* user_data);
typedef void (*linx_on_tts_stop_cb_t)(void* user_data);
typedef void (*linx_on_tts_sentence_cb_t)(const char* text, void* user_data);
typedef void (*linx_on_stt_result_cb_t)(const char* text, void* user_data);
typedef void (*linx_on_llm_emotion_cb_t)(const char* emotion, void* user_data);
typedef void (*linx_on_system_command_cb_t)(const char* command, void* user_data);
typedef void (*linx_on_alert_cb_t)(const char* status, const char* message, const char* emotion, void* user_data);
typedef void (*linx_on_custom_message_cb_t)(const cJSON* payload, void* user_data);
typedef void (*linx_on_mcp_message_cb_t)(const cJSON* payload, void* user_data);

/* 配置结构体 */
typedef struct {
    linx_protocol_type_t protocol_type;
    char* server_url;
    int server_sample_rate;
    int server_frame_duration;
    void* user_data;
} linx_config_t;

/* 回调函数集合 */
typedef struct {
    /* 连接回调 */
    linx_on_connected_cb_t on_connected;
    linx_on_disconnected_cb_t on_disconnected;
    linx_on_network_error_cb_t on_network_error;
    
    /* 音频回调 */
    linx_on_audio_channel_opened_cb_t on_audio_channel_opened;
    linx_on_audio_channel_closed_cb_t on_audio_channel_closed;
    linx_on_incoming_audio_cb_t on_incoming_audio;
    
    /* 消息处理回调 */
    linx_on_tts_start_cb_t on_tts_start;
    linx_on_tts_stop_cb_t on_tts_stop;
    linx_on_tts_sentence_cb_t on_tts_sentence;
    linx_on_stt_result_cb_t on_stt_result;
    linx_on_llm_emotion_cb_t on_llm_emotion;
    linx_on_system_command_cb_t on_system_command;
    linx_on_alert_cb_t on_alert;
    linx_on_custom_message_cb_t on_custom_message;
    linx_on_mcp_message_cb_t on_mcp_message;
} linx_callbacks_t;

/* ========== 核心 API 函数 ========== */

/**
 * 创建 Linx SDK 实例
 * @param config 配置参数
 * @return SDK 实例指针，失败返回 NULL
 */
linx_sdk_t* linx_sdk_create(const linx_config_t* config);

/**
 * 销毁 SDK 实例
 * @param sdk SDK 实例
 */
void linx_sdk_destroy(linx_sdk_t* sdk);

/**
 * 设置回调函数
 * @param sdk SDK 实例
 * @param callbacks 回调函数集合
 */
void linx_sdk_set_callbacks(linx_sdk_t* sdk, const linx_callbacks_t* callbacks);

/**
 * 启动协议连接
 * @param sdk SDK 实例
 * @return 成功返回 true，失败返回 false
 */
bool linx_sdk_start(linx_sdk_t* sdk);

/**
 * 停止协议连接
 * @param sdk SDK 实例
 */
void linx_sdk_stop(linx_sdk_t* sdk);

/* ========== 音频通道管理 ========== */

/**
 * 打开音频通道
 * @param sdk SDK 实例
 * @return 成功返回 true，失败返回 false
 */
bool linx_sdk_open_audio_channel(linx_sdk_t* sdk);

/**
 * 关闭音频通道
 * @param sdk SDK 实例
 */
void linx_sdk_close_audio_channel(linx_sdk_t* sdk);

/**
 * 检查音频通道是否已打开
 * @param sdk SDK 实例
 * @return 已打开返回 true，否则返回 false
 */
bool linx_sdk_is_audio_channel_opened(const linx_sdk_t* sdk);

/**
 * 发送音频数据
 * @param sdk SDK 实例
 * @param packet 音频数据包
 * @return 成功返回 true，失败返回 false
 */
bool linx_sdk_send_audio(linx_sdk_t* sdk, const linx_audio_packet_t* packet);

/* ========== 消息发送 ========== */

/**
 * 发送唤醒词检测消息
 * @param sdk SDK 实例
 * @param wake_word 唤醒词
 * @return 成功返回 true，失败返回 false
 */
bool linx_sdk_send_wake_word_detected(linx_sdk_t* sdk, const char* wake_word);

/**
 * 发送开始监听消息
 * @param sdk SDK 实例
 * @param mode 监听模式
 * @return 成功返回 true，失败返回 false
 */
bool linx_sdk_send_start_listening(linx_sdk_t* sdk, linx_listening_mode_t mode);

/**
 * 发送停止监听消息
 * @param sdk SDK 实例
 * @return 成功返回 true，失败返回 false
 */
bool linx_sdk_send_stop_listening(linx_sdk_t* sdk);

/**
 * 发送中止说话消息
 * @param sdk SDK 实例
 * @param reason 中止原因
 * @return 成功返回 true，失败返回 false
 */
bool linx_sdk_send_abort_speaking(linx_sdk_t* sdk, linx_abort_reason_t reason);

/**
 * 发送 MCP 消息
 * @param sdk SDK 实例
 * @param payload JSON 格式的消息内容
 * @return 成功返回 true，失败返回 false
 */
bool linx_sdk_send_mcp_message(linx_sdk_t* sdk, const char* payload);

/**
 * 发送自定义文本消息
 * @param sdk SDK 实例
 * @param message 消息内容
 * @return 成功返回 true，失败返回 false
 */
bool linx_sdk_send_text_message(linx_sdk_t* sdk, const char* message);

/* ========== 状态查询 ========== */

/**
 * 获取服务器采样率
 * @param sdk SDK 实例
 * @return 采样率
 */
int linx_sdk_get_server_sample_rate(const linx_sdk_t* sdk);

/**
 * 获取服务器帧持续时间
 * @param sdk SDK 实例
 * @return 帧持续时间
 */
int linx_sdk_get_server_frame_duration(const linx_sdk_t* sdk);

/**
 * 获取会话 ID
 * @param sdk SDK 实例
 * @return 会话 ID 字符串
 */
const char* linx_sdk_get_session_id(const linx_sdk_t* sdk);

/**
 * 检查是否发生错误
 * @param sdk SDK 实例
 * @return 发生错误返回 true，否则返回 false
 */
bool linx_sdk_has_error(const linx_sdk_t* sdk);

/* ========== 便利函数 ========== */

/**
 * 创建默认配置
 * @param protocol_type 协议类型
 * @param server_url 服务器 URL
 * @return 配置结构体
 */
linx_config_t linx_sdk_create_default_config(linx_protocol_type_t protocol_type, const char* server_url);

/**
 * 创建空的回调函数集合
 * @return 回调函数集合
 */
linx_callbacks_t linx_sdk_create_empty_callbacks(void);

#ifdef __cplusplus
}
#endif

#endif /* LINX_SDK_H */