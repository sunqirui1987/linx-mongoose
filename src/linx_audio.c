/**
 * @file linx_audio.c
 * @brief 音频处理模块实现
 * @version 1.0.0
 * @date 2024
 */

#include "linx_internal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ========== 音频数据处理函数 ========== */

void linx_handle_audio_data(const uint8_t *audio_data, size_t data_len) {
    if (!audio_data || data_len == 0) {
        LINX_LOGW("Received empty audio data");
        return;
    }
    
    LINX_LOGD("Received audio data: %zu bytes", data_len);
    
    /* 检查SDK是否已初始化 */
    if (!g_linx_context.initialized) {
        LINX_LOGE("SDK not initialized");
        return;
    }
    
    /* 检查连接状态 */
    if (g_linx_context.connection_state.state < LINX_STATE_READY) {
        LINX_LOGW("Connection not ready, dropping audio data");
        return;
    }
    
    /* 直接通过回调函数传递原始音频数据给上层应用 */
    if (g_linx_context.config.callbacks.on_audio_data) {
        g_linx_context.config.callbacks.on_audio_data(&g_linx_context, 
                                                      audio_data, 
                                                      data_len);
    } else {
        LINX_LOGW("No audio data callback registered");
    }
    
    /* 更新统计信息 */
    g_linx_context.stats.audio_bytes_received += data_len;
    g_linx_context.stats.messages_received++;
}

int linx_send_audio_data(const uint8_t *audio_data, size_t data_len) {
    if (!audio_data || data_len == 0) {
        return LINX_ERROR_INVALID_PARAM;
    }
    
    /* 检查连接状态 */
    if (!g_linx_context.conn) {
        linx_set_error(LINX_ERROR_NOT_CONNECTED, "Not connected to server");
        return LINX_ERROR_NOT_CONNECTED;
    }
    
    if (g_linx_context.connection_state.state < LINX_STATE_READY) {
        linx_set_error(LINX_ERROR_NOT_READY, "SDK not ready for audio transmission");
        return LINX_ERROR_NOT_READY;
    }
    
    /* 检查音频流状态 */
    if (!g_linx_context.connection_state.audio_streaming) {
        LINX_LOGW("Audio streaming not started");
        return LINX_ERROR_AUDIO_NOT_STARTED;
    }
    
    /* 验证音频数据大小 */
    if (data_len > LINX_MAX_AUDIO_CHUNK_SIZE) {
        linx_set_error(LINX_ERROR_INVALID_PARAM, "Audio data size too large");
        return LINX_ERROR_INVALID_PARAM;
    }
    
    LINX_LOGD("Sending audio data, size: %zu bytes", data_len);
    
    /* 发送原始音频数据（二进制消息） */
    mg_ws_send(g_linx_context.conn, (const char *)audio_data, data_len, WEBSOCKET_OP_BINARY);
    
    /* 更新统计信息 */
    g_linx_context.stats.audio_bytes_sent += data_len;
    g_linx_context.stats.messages_sent++;
    
    return LINX_OK;
}

int linx_start_audio_stream(void) {
    /* 检查连接状态 */
    if (g_linx_context.connection_state.state < LINX_STATE_READY) {
        linx_set_error(LINX_ERROR_NOT_READY, "SDK not ready");
        return LINX_ERROR_NOT_READY;
    }
    
    if (g_linx_context.connection_state.audio_streaming) {
        LINX_LOGW("Audio streaming already started");
        return LINX_OK;
    }
    
    LINX_LOGI("Starting audio stream");
    
    /* 构建并发送audio_start消息 */
    char json_buffer[LINX_JSON_BUFFER_SIZE];
    int ret = linx_build_audio_start_message(json_buffer, sizeof(json_buffer));
    if (ret != LINX_OK) {
        return ret;
    }
    
    /* 发送audio_start消息 */
    mg_ws_send(g_linx_context.conn, json_buffer, strlen(json_buffer), WEBSOCKET_OP_TEXT);
    
    LINX_LOGI("Audio start message sent");
    LINX_LOGD("Audio start message: %s", json_buffer);
    
    return LINX_OK;
}

int linx_stop_audio_stream(void) {
    /* 检查连接状态 */
    if (g_linx_context.connection_state.state < LINX_STATE_READY) {
        linx_set_error(LINX_ERROR_NOT_READY, "SDK not ready");
        return LINX_ERROR_NOT_READY;
    }
    
    if (!g_linx_context.connection_state.audio_streaming) {
        LINX_LOGW("Audio streaming not started");
        return LINX_OK;
    }
    
    LINX_LOGI("Stopping audio stream");
    
    /* 构建并发送audio_end消息 */
    char json_buffer[LINX_JSON_BUFFER_SIZE];
    int ret = linx_build_audio_end_message(json_buffer, sizeof(json_buffer));
    if (ret != LINX_OK) {
        return ret;
    }
    
    /* 发送audio_end消息 */
    mg_ws_send(g_linx_context.conn, json_buffer, strlen(json_buffer), WEBSOCKET_OP_TEXT);
    
    LINX_LOGI("Audio end message sent");
    LINX_LOGD("Audio end message: %s", json_buffer);
    
    return LINX_OK;
}

/* ========== 音频参数验证函数 ========== */

int linx_validate_audio_params(const linx_audio_params_t *params) {
    if (!params) {
        return LINX_ERROR_INVALID_PARAM;
    }
    
    /* 验证音频格式 */
    if (strlen(params->format) == 0) {
        LINX_LOGE("Audio format not specified");
        return LINX_ERROR_INVALID_PARAM;
    }
    
    /* 支持的音频格式：pcm, raw */
    if (strcmp(params->format, "pcm") != 0 && strcmp(params->format, "raw") != 0) {
        LINX_LOGE("Unsupported audio format: %s", params->format);
        return LINX_ERROR_INVALID_PARAM;
    }
    
    /* 验证采样率 */
    if (params->sample_rate <= 0 || params->sample_rate > 48000) {
        LINX_LOGE("Invalid sample rate: %d", params->sample_rate);
        return LINX_ERROR_INVALID_PARAM;
    }
    
    /* 常见的采样率：8000, 16000, 22050, 44100, 48000 */
    int valid_rates[] = {8000, 16000, 22050, 44100, 48000};
    int rate_count = sizeof(valid_rates) / sizeof(valid_rates[0]);
    bool rate_valid = false;
    
    for (int i = 0; i < rate_count; i++) {
        if (params->sample_rate == valid_rates[i]) {
            rate_valid = true;
            break;
        }
    }
    
    if (!rate_valid) {
        LINX_LOGW("Uncommon sample rate: %d", params->sample_rate);
    }
    
    /* 验证声道数 */
    if (params->channels <= 0 || params->channels > 2) {
        LINX_LOGE("Invalid channel count: %d", params->channels);
        return LINX_ERROR_INVALID_PARAM;
    }
    
    /* 验证位深度 */
    if (params->bits_per_sample != 8 && params->bits_per_sample != 16 && 
        params->bits_per_sample != 24 && params->bits_per_sample != 32) {
        LINX_LOGE("Invalid bits per sample: %d", params->bits_per_sample);
        return LINX_ERROR_INVALID_PARAM;
    }
    
    LINX_LOGI("Audio params validated: format=%s, rate=%d, channels=%d, bits=%d",
             params->format, params->sample_rate, params->channels, params->bits_per_sample);
    
    return LINX_OK;
}

/* ========== 音频统计函数 ========== */

void linx_reset_audio_stats(void) {
    g_linx_context.stats.audio_bytes_sent = 0;
    g_linx_context.stats.audio_bytes_received = 0;
    g_linx_context.stats.messages_sent = 0;
    g_linx_context.stats.messages_received = 0;
    
    LINX_LOGD("Audio statistics reset");
}

void linx_get_audio_stats(linx_audio_stats_t *stats) {
    if (!stats) {
        return;
    }
    
    stats->bytes_sent = g_linx_context.stats.audio_bytes_sent;
    stats->bytes_received = g_linx_context.stats.audio_bytes_received;
    stats->packets_sent = g_linx_context.stats.messages_sent;
    stats->packets_received = g_linx_context.stats.messages_received;
}

void linx_print_audio_stats(void) {
    LINX_LOGI("Audio Statistics:");
    LINX_LOGI("  Bytes sent: %zu", g_linx_context.stats.audio_bytes_sent);
    LINX_LOGI("  Bytes received: %zu", g_linx_context.stats.audio_bytes_received);
    LINX_LOGI("  Messages sent: %zu", g_linx_context.stats.messages_sent);
    LINX_LOGI("  Messages received: %zu", g_linx_context.stats.messages_received);
}

/* ========== 音频缓冲区管理函数 ========== */

int linx_init_audio_buffer(void) {
    /* 初始化音频缓冲区（如果需要的话） */
    /* 当前实现中，我们直接通过回调传递音频数据，不需要内部缓冲 */
    
    LINX_LOGD("Audio buffer initialized (callback mode)");
    return LINX_OK;
}

void linx_cleanup_audio_buffer(void) {
    /* 清理音频缓冲区 */
    /* 当前实现中，不需要特殊清理 */
    
    LINX_LOGD("Audio buffer cleaned up");
}

/* ========== 音频格式转换函数（预留） ========== */

int linx_convert_audio_format(const uint8_t *input_data, size_t input_len,
                             const linx_audio_params_t *input_params,
                             uint8_t *output_data, size_t *output_len,
                             const linx_audio_params_t *output_params) {
    /* 音频格式转换功能（预留接口） */
    /* 当前实现中，我们直接传递原始音频数据，不进行格式转换 */
    
    if (!input_data || !input_params || !output_data || !output_len || !output_params) {
        return LINX_ERROR_INVALID_PARAM;
    }
    
    /* 简单的直接复制（假设格式相同） */
    if (input_len > *output_len) {
        return LINX_ERROR_BUFFER_TOO_SMALL;
    }
    
    memcpy(output_data, input_data, input_len);
    *output_len = input_len;
    
    LINX_LOGD("Audio format conversion: %zu bytes (passthrough)", input_len);
    
    return LINX_OK;
}