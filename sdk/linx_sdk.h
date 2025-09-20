/**
 * @file linx_sdk.h
 * @brief Linx SDK - 智能语音交互SDK统一接口
 * @version 1.0.0
 * @date 2024
 * 
 * 本SDK提供了完整的智能语音交互解决方案，整合了音频处理、编解码、
 * MCP协议和WebSocket通信等核心功能模块。
 */

#ifndef LINX_SDK_H
#define LINX_SDK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief SDK版本信息
 */
#define LINX_SDK_VERSION "1.0.0"

/**
 * @brief SDK错误码
 */
typedef enum {
    LINX_SDK_SUCCESS = 0,
    LINX_SDK_ERROR_INVALID_PARAM,
    LINX_SDK_ERROR_NOT_INITIALIZED,
    LINX_SDK_ERROR_NETWORK,
    LINX_SDK_ERROR_MEMORY,
    LINX_SDK_ERROR_UNKNOWN
} LinxSdkError;

/**
 * @brief 设备状态
 */
typedef enum {
    LINX_DEVICE_STATE_IDLE = 0,
    LINX_DEVICE_STATE_CONNECTING,
    LINX_DEVICE_STATE_LISTENING,
    LINX_DEVICE_STATE_SPEAKING,
    LINX_DEVICE_STATE_ERROR
} LinxDeviceState;

/**
 * @brief SDK配置结构体
 */
typedef struct {
    char server_url[256];           ///< 服务器URL
    uint32_t sample_rate;           ///< 采样率 (默认16000)
    uint16_t channels;              ///< 声道数 (默认1)
    uint32_t timeout_ms;            ///< 超时时间(毫秒)
} LinxSdkConfig;

/**
 * @brief SDK事件类型
 */
typedef enum {
    LINX_EVENT_STATE_CHANGED,       ///< 状态改变
    LINX_EVENT_TEXT_MESSAGE,        ///< 文本消息
    LINX_EVENT_AUDIO_DATA,          ///< 音频数据
    LINX_EVENT_ERROR                ///< 错误事件
} LinxEventType;

/**
 * @brief SDK事件数据
 */
typedef struct {
    LinxEventType type;
    union {
        struct {
            LinxDeviceState old_state;
            LinxDeviceState new_state;
        } state_changed;
        
        struct {
            char* text;
            char* role;  // "user", "assistant"
        } text_message;
        
        struct {
            uint8_t* data;
            size_t size;
        } audio_data;
        
        struct {
            char* message;
            int code;
        } error;
    } data;
} LinxEvent;

/**
 * @brief SDK事件回调函数类型
 */
typedef void (*LinxEventCallback)(const LinxEvent* event, void* user_data);

/**
 * @brief SDK句柄 (不透明结构体)
 */
typedef struct LinxSdk LinxSdk;

// ============================================================================
// 核心API函数
// ============================================================================

/**
 * @brief 创建SDK实例
 */
LinxSdk* linx_sdk_create(const LinxSdkConfig* config);

/**
 * @brief 销毁SDK实例
 */
void linx_sdk_destroy(LinxSdk* sdk);

/**
 * @brief 设置事件回调
 */
LinxSdkError linx_sdk_set_event_callback(LinxSdk* sdk, LinxEventCallback callback, void* user_data);

/**
 * @brief 连接到服务器
 */
LinxSdkError linx_sdk_connect(LinxSdk* sdk);

/**
 * @brief 断开连接
 */
LinxSdkError linx_sdk_disconnect(LinxSdk* sdk);

/**
 * @brief 发送文本消息
 */
LinxSdkError linx_sdk_send_text(LinxSdk* sdk, const char* text);

/**
 * @brief 发送音频数据
 */
LinxSdkError linx_sdk_send_audio(LinxSdk* sdk, const uint8_t* data, size_t size);

/**
 * @brief 获取当前状态
 */
LinxDeviceState linx_sdk_get_state(LinxSdk* sdk);

// ============================================================================
// 辅助函数
// ============================================================================

/**
 * @brief 获取错误码对应的字符串描述
 */
const char* linx_sdk_get_error_string(LinxSdkError error);

/**
 * @brief 获取设备状态对应的字符串描述
 */
const char* linx_sdk_get_state_string(LinxDeviceState state);

/**
 * @brief 检查是否已连接
 */
bool linx_sdk_is_connected(LinxSdk* sdk);

/**
 * @brief 获取消息计数
 */
uint32_t linx_sdk_get_message_count(LinxSdk* sdk);

/**
 * @brief 获取连接时间
 */
time_t linx_sdk_get_connect_time(LinxSdk* sdk);

#ifdef __cplusplus
}
#endif

#endif // LINX_SDK_H