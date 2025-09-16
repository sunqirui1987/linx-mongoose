/**
 * @file linx_json.c
 * @brief JSON消息处理模块实现
 * @version 1.0.0
 * @date 2024
 */

#include "linx_websocket_sdk.h"
#include "linx_internal.h"
#include "linx_log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ========== JSON消息构建函数 ========== */

int linx_build_hello_message(char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return LINX_ERROR_INVALID_PARAM;
    }
    
    /* 使用字符串拼接构建Hello消息 */
    const char *listen_mode = "auto"; /* 默认自动模式 */
    
    int ret = snprintf(buffer, buffer_size,
        "{"
        "\"type\":\"hello\","
        "\"data\":{"
        "\"protocol_version\":\"%s\","
        "\"device_id\":\"%s\","
        "\"client_id\":\"%s\","
        "\"audio_params\":{"
        "\"format\":\"%s\","
        "\"sample_rate\":%d,"
        "\"channels\":%d,"
        "\"bits_per_sample\":%d"
        "},"
        "\"listen_mode\":\"%s\""
        "}"
        "}",
        LINX_PROTOCOL_VERSION,
        g_linx_context.config.device_id,
        g_linx_context.config.client_id,
        g_linx_context.config.audio_params.format,
        g_linx_context.config.audio_params.sample_rate,
        g_linx_context.config.audio_params.channels,
        g_linx_context.config.audio_params.bits_per_sample,
        listen_mode);
    
    if (ret >= (int)buffer_size) {
        linx_set_error(LINX_ERROR_BUFFER_TOO_SMALL, "Hello message buffer too small");
        return LINX_ERROR_BUFFER_TOO_SMALL;
    }
    
    if (ret < 0) {
        linx_set_error(LINX_ERROR_INVALID_MESSAGE, "Failed to format hello message");
        return LINX_ERROR_INVALID_MESSAGE;
    }
    
    return LINX_OK;
}

int linx_build_audio_start_message(char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return LINX_ERROR_INVALID_PARAM;
    }
    
    /* 使用字符串拼接构建Audio Start消息 */
    int ret = snprintf(buffer, buffer_size,
        "{"
        "\"type\":\"audio_start\","
        "\"data\":{"
        "\"audio_params\":{"
        "\"format\":\"%s\","
        "\"sample_rate\":%d,"
        "\"channels\":%d,"
        "\"bits_per_sample\":%d"
        "}"
        "}"
        "}",
        g_linx_context.config.audio_params.format,
        g_linx_context.config.audio_params.sample_rate,
        g_linx_context.config.audio_params.channels,
        g_linx_context.config.audio_params.bits_per_sample);
    
    if (ret >= (int)buffer_size) {
        linx_set_error(LINX_ERROR_BUFFER_TOO_SMALL, "Audio start message buffer too small");
        return LINX_ERROR_BUFFER_TOO_SMALL;
    }
    
    if (ret < 0) {
        linx_set_error(LINX_ERROR_INVALID_MESSAGE, "Failed to format audio start message");
        return LINX_ERROR_INVALID_MESSAGE;
    }
    
    return LINX_OK;
}

int linx_build_audio_end_message(char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return LINX_ERROR_INVALID_PARAM;
    }
    
    /* 使用字符串拼接构建Audio End消息 */
    int ret = snprintf(buffer, buffer_size, "{\"type\":\"audio_end\"}");
    
    if (ret >= (int)buffer_size) {
        linx_set_error(LINX_ERROR_BUFFER_TOO_SMALL, "Audio end message buffer too small");
        return LINX_ERROR_BUFFER_TOO_SMALL;
    }
    
    if (ret < 0) {
        linx_set_error(LINX_ERROR_INVALID_MESSAGE, "Failed to format audio end message");
        return LINX_ERROR_INVALID_MESSAGE;
    }
    
    return LINX_OK;
}

int linx_build_heartbeat_message(char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return LINX_ERROR_INVALID_PARAM;
    }
    
    /* 使用字符串拼接构建Heartbeat消息 */
    int ret = snprintf(buffer, buffer_size,
        "{"
        "\"type\":\"heartbeat\","
        "\"data\":{"
        "\"timestamp\":%ld"
        "}"
        "}",
        (long)time(NULL));
    
    if (ret >= (int)buffer_size) {
        linx_set_error(LINX_ERROR_BUFFER_TOO_SMALL, "Heartbeat message buffer too small");
        return LINX_ERROR_BUFFER_TOO_SMALL;
    }
    
    if (ret < 0) {
        linx_set_error(LINX_ERROR_INVALID_MESSAGE, "Failed to format heartbeat message");
        return LINX_ERROR_INVALID_MESSAGE;
    }
    
    return LINX_OK;
}

/* ========== JSON消息解析函数 ========== */

int linx_parse_json_message(const char *json_str, size_t json_len) {
    if (!json_str || json_len == 0) {
        return LINX_ERROR_INVALID_PARAM;
    }
    
    /* 使用字符串解析提取消息类型 */
    char *type_start = strstr(json_str, "\"type\":");
    if (!type_start) {
        LINX_LOGW("No type field found in JSON message");
        return LINX_ERROR_INVALID_MESSAGE;
    }
    
    /* 找到type字段的值 */
    char *type_value_start = strchr(type_start + 7, '"');
    if (!type_value_start) {
        LINX_LOGW("Invalid type field format");
        return LINX_ERROR_INVALID_MESSAGE;
    }
    type_value_start++; /* 跳过引号 */
    
    char *type_value_end = strchr(type_value_start, '"');
    if (!type_value_end) {
        LINX_LOGW("Invalid type field format");
        return LINX_ERROR_INVALID_MESSAGE;
    }
    
    /* 提取消息类型字符串 */
    size_t type_len = type_value_end - type_value_start;
    char message_type[64];
    if (type_len >= sizeof(message_type)) {
        LINX_LOGW("Message type too long");
        return LINX_ERROR_INVALID_MESSAGE;
    }
    
    strncpy(message_type, type_value_start, type_len);
    message_type[type_len] = '\0';
    
    LINX_LOGD("Parsing message type: %s", message_type);
    
    int result = LINX_ERROR_INVALID_MESSAGE;
    
    /* 根据消息类型分发处理 */
    if (strcmp(message_type, "hello_ack") == 0) {
        result = linx_handle_hello_ack_message(json_str, json_len);
    }
    else if (strcmp(message_type, "audio_start_ack") == 0) {
        result = linx_handle_audio_start_ack_message(json_str, json_len);
    }
    else if (strcmp(message_type, "audio_end_ack") == 0) {
        result = linx_handle_audio_end_ack_message(json_str, json_len);
    }
    else if (strcmp(message_type, "text_result") == 0) {
        result = linx_handle_text_result_message(json_str, json_len);
    }
    else if (strcmp(message_type, "audio_result") == 0) {
        result = linx_handle_audio_result_message(json_str, json_len);
    }
    else if (strcmp(message_type, "error") == 0) {
        result = linx_handle_error_message(json_str, json_len);
    }
    else if (strcmp(message_type, "heartbeat_ack") == 0) {
        result = linx_handle_heartbeat_ack_message(json_str, json_len);
    }
    else {
        LINX_LOGW("Unknown message type: %s", message_type);
        result = LINX_ERROR_INVALID_MESSAGE;
    }
    
    return result;
}

/* ========== 具体消息处理函数 ========== */

int linx_handle_hello_ack_message(const char *json_str, size_t json_len) {
    (void)json_len; /* 避免未使用参数警告 */
    
    LINX_LOGI("Received hello_ack message");
    
    /* 检查是否包含错误信息 */
    if (strstr(json_str, "error") != NULL) {
        LINX_LOGE("Hello handshake failed");
        linx_set_error(LINX_ERROR_PROTOCOL, "Hello handshake failed");
        return LINX_ERROR_PROTOCOL;
    }
    
    /* 握手成功，更新状态 */
    g_linx_context.connection_state.state = LINX_STATE_READY;
    
    /* 调用握手成功回调 */
    if (g_linx_context.config.callbacks.on_hello_received) {
        g_linx_context.config.callbacks.on_hello_received(&g_linx_context, NULL);
    }
    
    return LINX_OK;
}

int linx_handle_audio_start_ack_message(const char *json_str, size_t json_len) {
    (void)json_len; /* 避免未使用参数警告 */
    
    LINX_LOGI("Received audio_start_ack message");
    
    /* 检查是否包含错误信息 */
    if (strstr(json_str, "\"error\"")) {
        LINX_LOGE("Audio start failed");
        linx_set_error(LINX_ERROR_AUDIO, "Audio start failed");
        return LINX_ERROR_AUDIO;
    }
    
    /* 音频开始成功，更新状态 */
    g_linx_context.connection_state.state = LINX_STATE_LISTENING;
    
    return LINX_OK;
}

int linx_handle_audio_end_ack_message(const char *json_str, size_t json_len) {
    (void)json_len; /* 避免未使用参数警告 */
    
    LINX_LOGI("Received audio_end_ack message");
    
    /* 检查是否包含错误信息 */
    if (strstr(json_str, "\"error\"")) {
        LINX_LOGE("Audio end failed");
        linx_set_error(LINX_ERROR_AUDIO, "Audio end failed");
        return LINX_ERROR_AUDIO;
    }
    
    /* 音频结束成功，更新状态 */
    g_linx_context.connection_state.state = LINX_STATE_READY;
    
    return LINX_OK;
}

int linx_handle_text_result_message(const char *json_str, size_t json_len) {
    (void)json_len; /* 避免未使用参数警告 */
    
    LINX_LOGI("Received text_result message");
    
    /* 提取文本内容（简化处理） */
    char *text_start = strstr(json_str, "\"text\":");
    if (!text_start) {
        LINX_LOGW("No text field found in text_result message");
        return LINX_ERROR_INVALID_MESSAGE;
    }
    
    char *text_value_start = strchr(text_start + 7, '"');
    if (!text_value_start) {
        LINX_LOGW("Invalid text field format");
        return LINX_ERROR_INVALID_MESSAGE;
    }
    text_value_start++; /* 跳过引号 */
    
    char *text_value_end = strchr(text_value_start, '"');
    if (!text_value_end) {
        LINX_LOGW("Invalid text field format");
        return LINX_ERROR_INVALID_MESSAGE;
    }
    
    /* 复制文本内容 */
    size_t text_len = text_value_end - text_value_start;
    char *text_content = malloc(text_len + 1);
    if (!text_content) {
        linx_set_error(LINX_ERROR_MEMORY, "Failed to allocate memory for text content");
        return LINX_ERROR_MEMORY;
    }
    
    strncpy(text_content, text_value_start, text_len);
    text_content[text_len] = '\0';
    
    LINX_LOGI("Received text result: %s", text_content);
    
    /* 调用文本结果回调 */
    if (g_linx_context.config.callbacks.on_text_result) {
        g_linx_context.config.callbacks.on_text_result(&g_linx_context, text_content);
    }
    
    free(text_content);
    return LINX_OK;
}

int linx_handle_audio_result_message(const char *json_str, size_t json_len) {
    (void)json_str; /* 避免未使用参数警告 */
    (void)json_len; /* 避免未使用参数警告 */
    
    LINX_LOGI("Received audio_result message");
    
    /* 音频结果消息通常包含音频数据的元信息 */
    /* 实际的音频数据会通过二进制消息发送 */
    
    /* 调用音频结果回调 */
    if (g_linx_context.config.callbacks.on_audio_result) {
        g_linx_context.config.callbacks.on_audio_result(&g_linx_context, NULL, 0);
    }
    
    return LINX_OK;
}

int linx_handle_error_message(const char *json_str, size_t json_len) {
    (void)json_str; /* 避免未使用参数警告 */
    (void)json_len; /* 避免未使用参数警告 */
    
    LINX_LOGI("Received error message");
    
    /* 简单的错误信息提取 */
    const char *error_msg = "Server error";
    
    LINX_LOGE("Server error: %s", error_msg);
    linx_set_error(LINX_ERROR_SERVER, error_msg);
    
    /* 调用错误回调 */
    if (g_linx_context.config.callbacks.on_error) {
        g_linx_context.config.callbacks.on_error(&g_linx_context, LINX_ERROR_SERVER, error_msg);
    }
    
    return LINX_OK;
}

/* 心跳确认消息处理 */
int linx_handle_heartbeat_ack_message(const char *json_str, size_t json_len) {
    (void)json_str; /* 避免未使用参数警告 */
    (void)json_len; /* 避免未使用参数警告 */
    
    LINX_LOGI("Received heartbeat_ack message");
    
    /* 更新心跳时间 */
    g_linx_context.connection_state.last_heartbeat = time(NULL);
    
    return LINX_OK;
}