/**
 * @file linx_sdk.c
 * @brief Linx SDK - 智能语音交互SDK实现
 * @version 1.0.0
 * @date 2024
 * 
 * 本文件实现了 linx_sdk.h 中定义的所有接口，提供简化的智能语音交互功能。
 */

#include "linx_sdk.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/**
 * @brief LinxSdk 内部结构体
 */
struct LinxSdk {
    LinxSdkConfig config;                   ///< SDK配置
    LinxDeviceState state;                  ///< 当前状态
    LinxEventCallback event_callback;       ///< 事件回调函数
    void* user_data;                        ///< 用户数据
    
    // 状态管理
    bool initialized;                       ///< 是否已初始化
    bool connected;                         ///< 是否已连接
    char last_error[256];                   ///< 最后错误信息
    
    // 简化的连接状态
    time_t connect_time;                    ///< 连接时间
    uint32_t message_count;                 ///< 消息计数
};

// ============================================================================
// 内部函数声明
// ============================================================================

static void _linx_sdk_set_state(LinxSdk* sdk, LinxDeviceState new_state);
static void _linx_sdk_emit_event(LinxSdk* sdk, const LinxEvent* event);
static void _linx_sdk_set_error(LinxSdk* sdk, const char* error_msg, int error_code);

// ============================================================================
// 核心API函数实现
// ============================================================================

LinxSdk* linx_sdk_create(const LinxSdkConfig* config) {
    if (!config) {
        return NULL;
    }
    
    LinxSdk* sdk = (LinxSdk*)calloc(1, sizeof(LinxSdk));
    if (!sdk) {
        return NULL;
    }
    
    // 复制配置
    memcpy(&sdk->config, config, sizeof(LinxSdkConfig));
    
    // 设置默认值
    if (sdk->config.sample_rate == 0) {
        sdk->config.sample_rate = 16000;
    }
    if (sdk->config.channels == 0) {
        sdk->config.channels = 1;
    }
    if (sdk->config.timeout_ms == 0) {
        sdk->config.timeout_ms = 30000;
    }
    
    // 初始化状态
    sdk->state = LINX_DEVICE_STATE_IDLE;
    sdk->initialized = true;
    sdk->connected = false;
    sdk->event_callback = NULL;
    sdk->user_data = NULL;
    sdk->connect_time = 0;
    sdk->message_count = 0;
    
    memset(sdk->last_error, 0, sizeof(sdk->last_error));
    
    return sdk;
}

void linx_sdk_destroy(LinxSdk* sdk) {
    if (!sdk) {
        return;
    }
    
    // 断开连接
    if (sdk->connected) {
        linx_sdk_disconnect(sdk);
    }
    
    free(sdk);
}

LinxSdkError linx_sdk_set_event_callback(LinxSdk* sdk, LinxEventCallback callback, void* user_data) {
    if (!sdk) {
        return LINX_SDK_ERROR_INVALID_PARAM;
    }
    
    if (!sdk->initialized) {
        return LINX_SDK_ERROR_NOT_INITIALIZED;
    }
    
    sdk->event_callback = callback;
    sdk->user_data = user_data;
    
    return LINX_SDK_SUCCESS;
}

LinxSdkError linx_sdk_connect(LinxSdk* sdk) {
    if (!sdk) {
        return LINX_SDK_ERROR_INVALID_PARAM;
    }
    
    if (!sdk->initialized) {
        return LINX_SDK_ERROR_NOT_INITIALIZED;
    }
    
    if (sdk->connected) {
        return LINX_SDK_SUCCESS;
    }
    
    _linx_sdk_set_state(sdk, LINX_DEVICE_STATE_CONNECTING);
    
    // 模拟连接过程
    printf("正在连接到服务器: %s\n", sdk->config.server_url);
    
    // 简化的连接逻辑 - 在实际实现中这里会有真正的网络连接
    if (strlen(sdk->config.server_url) == 0) {
        _linx_sdk_set_error(sdk, "服务器URL为空", LINX_SDK_ERROR_INVALID_PARAM);
        _linx_sdk_set_state(sdk, LINX_DEVICE_STATE_ERROR);
        return LINX_SDK_ERROR_INVALID_PARAM;
    }
    
    // 模拟成功连接
    sdk->connected = true;
    sdk->connect_time = time(NULL);
    _linx_sdk_set_state(sdk, LINX_DEVICE_STATE_LISTENING);
    
    printf("连接成功\n");
    return LINX_SDK_SUCCESS;
}

LinxSdkError linx_sdk_disconnect(LinxSdk* sdk) {
    if (!sdk) {
        return LINX_SDK_ERROR_INVALID_PARAM;
    }
    
    if (!sdk->initialized) {
        return LINX_SDK_ERROR_NOT_INITIALIZED;
    }
    
    if (!sdk->connected) {
        return LINX_SDK_SUCCESS;
    }
    
    printf("断开连接\n");
    
    sdk->connected = false;
    sdk->connect_time = 0;
    _linx_sdk_set_state(sdk, LINX_DEVICE_STATE_IDLE);
    
    return LINX_SDK_SUCCESS;
}

LinxSdkError linx_sdk_send_text(LinxSdk* sdk, const char* text) {
    if (!sdk || !text) {
        return LINX_SDK_ERROR_INVALID_PARAM;
    }
    
    if (!sdk->initialized) {
        return LINX_SDK_ERROR_NOT_INITIALIZED;
    }
    
    if (!sdk->connected) {
        return LINX_SDK_ERROR_NETWORK;
    }
    
    printf("发送文本消息: %s\n", text);
    
    // 增加消息计数
    sdk->message_count++;
    
    // 模拟发送成功，触发回显事件
    if (sdk->event_callback) {
        LinxEvent event = {0};
        event.type = LINX_EVENT_TEXT_MESSAGE;
        event.data.text_message.text = (char*)text;
        event.data.text_message.role = "user";
        
        _linx_sdk_emit_event(sdk, &event);
        
        // 模拟服务器响应
        LinxEvent response_event = {0};
        response_event.type = LINX_EVENT_TEXT_MESSAGE;
        response_event.data.text_message.text = "收到您的消息";
        response_event.data.text_message.role = "assistant";
        
        _linx_sdk_emit_event(sdk, &response_event);
    }
    
    return LINX_SDK_SUCCESS;
}

LinxSdkError linx_sdk_send_audio(LinxSdk* sdk, const uint8_t* data, size_t size) {
    if (!sdk || !data || size == 0) {
        return LINX_SDK_ERROR_INVALID_PARAM;
    }
    
    if (!sdk->initialized) {
        return LINX_SDK_ERROR_NOT_INITIALIZED;
    }
    
    if (!sdk->connected) {
        return LINX_SDK_ERROR_NETWORK;
    }
    
    printf("发送音频数据: %zu 字节\n", size);
    
    // 增加消息计数
    sdk->message_count++;
    
    // 模拟音频处理，触发音频数据事件
    if (sdk->event_callback) {
        LinxEvent event = {0};
        event.type = LINX_EVENT_AUDIO_DATA;
        event.data.audio_data.data = (uint8_t*)data;
        event.data.audio_data.size = size;
        
        _linx_sdk_emit_event(sdk, &event);
    }
    
    return LINX_SDK_SUCCESS;
}

LinxDeviceState linx_sdk_get_state(LinxSdk* sdk) {
    if (!sdk) {
        return LINX_DEVICE_STATE_ERROR;
    }
    
    return sdk->state;
}

// ============================================================================
// 内部函数实现
// ============================================================================

static void _linx_sdk_set_state(LinxSdk* sdk, LinxDeviceState new_state) {
    if (!sdk || sdk->state == new_state) {
        return;
    }
    
    LinxDeviceState old_state = sdk->state;
    sdk->state = new_state;
    
    printf("状态变化: %d -> %d\n", old_state, new_state);
    
    // 发送状态变化事件
    if (sdk->event_callback) {
        LinxEvent event = {0};
        event.type = LINX_EVENT_STATE_CHANGED;
        event.data.state_changed.old_state = old_state;
        event.data.state_changed.new_state = new_state;
        
        _linx_sdk_emit_event(sdk, &event);
    }
}

static void _linx_sdk_emit_event(LinxSdk* sdk, const LinxEvent* event) {
    if (!sdk || !event || !sdk->event_callback) {
        return;
    }
    
    sdk->event_callback(event, sdk->user_data);
}

static void _linx_sdk_set_error(LinxSdk* sdk, const char* error_msg, int error_code) {
    if (!sdk) {
        return;
    }
    
    // 保存错误信息
    if (error_msg) {
        strncpy(sdk->last_error, error_msg, sizeof(sdk->last_error) - 1);
        sdk->last_error[sizeof(sdk->last_error) - 1] = '\0';
    }
    
    printf("错误: %s (代码: %d)\n", error_msg ? error_msg : "未知错误", error_code);
    
    // 发送错误事件
    if (sdk->event_callback) {
        LinxEvent event = {0};
        event.type = LINX_EVENT_ERROR;
        event.data.error.message = sdk->last_error;
        event.data.error.code = error_code;
        
        _linx_sdk_emit_event(sdk, &event);
    }
}

// ============================================================================
// 辅助函数实现
// ============================================================================

const char* linx_sdk_get_error_string(LinxSdkError error) {
    switch (error) {
        case LINX_SDK_SUCCESS:
            return "成功";
        case LINX_SDK_ERROR_INVALID_PARAM:
            return "无效参数";
        case LINX_SDK_ERROR_NOT_INITIALIZED:
            return "未初始化";
        case LINX_SDK_ERROR_NETWORK:
            return "网络错误";
        case LINX_SDK_ERROR_MEMORY:
            return "内存错误";
        case LINX_SDK_ERROR_UNKNOWN:
        default:
            return "未知错误";
    }
}

const char* linx_sdk_get_state_string(LinxDeviceState state) {
    switch (state) {
        case LINX_DEVICE_STATE_IDLE:
            return "空闲";
        case LINX_DEVICE_STATE_CONNECTING:
            return "连接中";
        case LINX_DEVICE_STATE_LISTENING:
            return "监听中";
        case LINX_DEVICE_STATE_SPEAKING:
            return "说话中";
        case LINX_DEVICE_STATE_ERROR:
            return "错误";
        default:
            return "未知状态";
    }
}

bool linx_sdk_is_connected(LinxSdk* sdk) {
    if (!sdk) {
        return false;
    }
    
    return sdk->connected;
}

uint32_t linx_sdk_get_message_count(LinxSdk* sdk) {
    if (!sdk) {
        return 0;
    }
    
    return sdk->message_count;
}

time_t linx_sdk_get_connect_time(LinxSdk* sdk) {
    if (!sdk) {
        return 0;
    }
    
    return sdk->connect_time;
}