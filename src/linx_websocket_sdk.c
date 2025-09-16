/**
 * @file linx_websocket_sdk.c
 * @brief 灵矽AI WebSocket SDK主实现文件
 * @version 1.0.0
 * @date 2024
 */

#include "linx_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* 全局变量定义 */
linx_context_t g_linx_context = {0};
linx_msg_node_t g_msg_pool[LINX_MAX_MSG_NODES] = {0};
bool g_msg_pool_used[LINX_MAX_MSG_NODES] = {false};
static int g_log_level = LINX_LOG_INFO;

/* ========== 公共API函数实现 ========== */

const char* linx_get_version(void) {
    return LINX_SDK_VERSION_STRING;
}

int linx_sdk_init(const linx_config_t *config) {
    if (g_linx_context.initialized) {
        LINX_LOGW("SDK already initialized");
        return LINX_ERROR_ALREADY_INIT;
    }
    
    if (!config) {
        LINX_LOGE("Invalid config parameter");
        return LINX_ERROR_INVALID_PARAM;
    }
    
    /* 验证配置参数 */
    int ret = linx_validate_config(config);
    if (ret != LINX_OK) {
        LINX_LOGE("Config validation failed: %d", ret);
        return ret;
    }
    
    /* 清零上下文 */
    memset(&g_linx_context, 0, sizeof(g_linx_context));
    
    /* 复制配置 */
    memcpy(&g_linx_context.config, config, sizeof(linx_config_t));
    
    /* 初始化连接状态 */
    g_linx_context.connection_state.state = LINX_STATE_DISCONNECTED;
    g_linx_context.connection_state.hello_received = false;
    g_linx_context.connection_state.listening = false;
    g_linx_context.connection_state.reconnect_count = 0;
    
    /* 初始化Mongoose管理器 */
    mg_mgr_init(&g_linx_context.mgr);
    
    /* 初始化内存池 */
    linx_init_memory_pool();
    
    /* 生成会话ID */
    linx_generate_session_id(g_linx_context.session_info.session_id, 
                            sizeof(g_linx_context.session_info.session_id));
    
    g_linx_context.initialized = true;
    
    LINX_LOGI("SDK initialized successfully, version: %s", LINX_SDK_VERSION_STRING);
    return LINX_OK;
}

int linx_set_config(const linx_config_t *config) {
    if (!config) {
        linx_set_error(LINX_ERROR_INVALID_PARAM, "Config is NULL");
        return LINX_ERROR_INVALID_PARAM;
    }
    
    /* 验证配置 */
    int ret = linx_validate_config(config);
    if (ret != LINX_OK) {
        return ret;
    }
    
    /* 复制配置 */
    memcpy(&g_linx_context.config, config, sizeof(linx_config_t));
    
    /* 设置日志级别 */
    g_linx_context.config.log_level = config->log_level;
    
    linx_log(LINX_LOG_INFO, __FILE__, __LINE__, __func__, "SDK config updated successfully");
    return LINX_OK;
}

int linx_sdk_cleanup(void) {
    if (!g_linx_context.initialized) {
        return LINX_ERROR_NOT_INIT;
    }
    
    /* 断开连接 */
    if (g_linx_context.conn) {
        linx_disconnect();
    }
    
    /* 清理消息队列 */
    linx_clear_message_queue();
    
    /* 清理内存池 */
    linx_cleanup_memory_pool();
    
    /* 清理Mongoose管理器 */
    mg_mgr_free(&g_linx_context.mgr);
    
    /* 重置上下文 */
    memset(&g_linx_context, 0, sizeof(g_linx_context));
    
    LINX_LOGI("SDK cleanup completed");
    return LINX_OK;
}

int linx_connect(void) {
    if (!g_linx_context.initialized) {
        return LINX_ERROR_NOT_INIT;
    }
    
    if (g_linx_context.connection_state.state == LINX_STATE_CONNECTED ||
        g_linx_context.connection_state.state == LINX_STATE_CONNECTING) {
        LINX_LOGW("Already connected or connecting");
        return LINX_OK;
    }
    
    return linx_establish_connection();
}

int linx_disconnect(void) {
    if (!g_linx_context.initialized) {
        return LINX_ERROR_NOT_INIT;
    }
    
    if (g_linx_context.conn) {
        g_linx_context.conn->is_closing = 1;
        g_linx_context.conn = NULL;
    }
    
    linx_reset_connection_state();
    
    LINX_LOGI("Disconnected from server");
    return LINX_OK;
}

linx_connection_state_t linx_get_connection_state(void) {
    if (!g_linx_context.initialized) {
        return LINX_STATE_ERROR;
    }
    return g_linx_context.connection_state.state;
}

int linx_start_listen(linx_listen_mode_t mode) {
    if (!g_linx_context.initialized) {
        return LINX_ERROR_NOT_INIT;
    }
    
    if (g_linx_context.connection_state.state != LINX_STATE_READY) {
        LINX_LOGE("Not ready for listening, current state: %d", 
                 g_linx_context.connection_state.state);
        return LINX_ERROR_NOT_CONNECTED;
    }
    
    char json_buffer[LINX_JSON_BUFFER_SIZE];
    int ret = linx_build_audio_start_message(json_buffer, sizeof(json_buffer));
    if (ret != LINX_OK) {
        return ret;
    }
    
    /* 发送监听开始消息 */
    mg_ws_send(g_linx_context.conn, json_buffer, strlen(json_buffer), WEBSOCKET_OP_TEXT);
    
    g_linx_context.connection_state.listening = true;
    g_linx_context.session_info.listen_mode = mode;
    g_linx_context.connection_state.state = LINX_STATE_LISTENING;
    
    LINX_LOGI("Started listening with mode: %d", mode);
    return LINX_OK;
}

int linx_stop_listen(void) {
    if (!g_linx_context.initialized) {
        return LINX_ERROR_NOT_INIT;
    }
    
    if (!g_linx_context.connection_state.listening) {
        LINX_LOGW("Not currently listening");
        return LINX_OK;
    }
    
    char json_buffer[LINX_JSON_BUFFER_SIZE];
    int ret = linx_build_audio_end_message(json_buffer, sizeof(json_buffer));
    if (ret != LINX_OK) {
        return ret;
    }
    
    /* 发送监听停止消息 */
    mg_ws_send(g_linx_context.conn, json_buffer, strlen(json_buffer), WEBSOCKET_OP_TEXT);
    
    g_linx_context.connection_state.listening = false;
    g_linx_context.connection_state.state = LINX_STATE_READY;
    
    LINX_LOGI("Stopped listening");
    return LINX_OK;
}

int linx_send_audio(const uint8_t *data, size_t len) {
    if (!g_linx_context.initialized) {
        return LINX_ERROR_NOT_INIT;
    }
    
    if (!data || len == 0) {
        return LINX_ERROR_INVALID_PARAM;
    }
    
    if (!g_linx_context.connection_state.listening) {
        LINX_LOGW("Not in listening state, audio data ignored");
        return LINX_ERROR_NOT_CONNECTED;
    }
    
    return linx_send_audio_data(data, len);
}

int linx_poll_events(int timeout_ms) {
    if (!g_linx_context.initialized) {
        return LINX_ERROR_NOT_INIT;
    }
    
    mg_mgr_poll(&g_linx_context.mgr, timeout_ms);
    return LINX_OK;
}

int linx_get_last_error(char *error_msg, size_t msg_len) {
    if (!error_msg || msg_len == 0) {
        return LINX_ERROR_INVALID_PARAM;
    }
    
    if (!g_linx_context.initialized) {
        snprintf(error_msg, msg_len, "SDK not initialized");
        return LINX_ERROR_NOT_INIT;
    }
    
    strncpy(error_msg, g_linx_context.error_info.last_error_msg, msg_len - 1);
    error_msg[msg_len - 1] = '\0';
    
    return g_linx_context.error_info.last_error_code;
}

void linx_set_log_level(linx_log_level_t level) {
    if (level >= LINX_LOG_ERROR && level <= LINX_LOG_TRACE) {
        g_linx_context.config.log_level = level;
        LINX_LOGI("Log level set to: %d", level);
    }
}

linx_context_t* linx_get_context(void) {
    return g_linx_context.initialized ? &g_linx_context : NULL;
}

void linx_set_user_data(void *user_data) {
    if (g_linx_context.initialized) {
        g_linx_context.user_data = user_data;
    }
}

void* linx_get_user_data(void) {
    return g_linx_context.initialized ? g_linx_context.user_data : NULL;
}

/* ========== 内部函数实现 ========== */

void linx_log(linx_log_level_t level, const char *file, int line, 
              const char *func, const char *format, ...) {
    if (level < g_linx_context.config.log_level) {
        return;
    }
    
    const char *level_str;
    switch (level) {
        case LINX_LOG_ERROR:
            level_str = "ERROR";
            break;
        case LINX_LOG_WARN:
            level_str = "WARN";
            break;
        case LINX_LOG_INFO:
            level_str = "INFO";
            break;
        case LINX_LOG_DEBUG:
            level_str = "DEBUG";
            break;
        default:
            level_str = "UNKNOWN";
            break;
    }
    
    /* 获取当前时间 */
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    
    /* 输出日志头 */
    printf("[%s] [%s] [%s:%d:%s] ", time_str, level_str, file, line, func);
    
    /* 输出日志内容 */
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\n");
    fflush(stdout);
}

void linx_set_error(int error_code, const char *error_msg) {
    g_linx_context.error_info.last_error_code = error_code;
    if (error_msg) {
        strncpy(g_linx_context.error_info.last_error_msg, error_msg, 
                LINX_MAX_ERROR_MSG_LEN - 1);
        g_linx_context.error_info.last_error_msg[LINX_MAX_ERROR_MSG_LEN - 1] = '\0';
    } else {
        strncpy(g_linx_context.error_info.last_error_msg, 
                linx_error_code_to_string(error_code), 
                LINX_MAX_ERROR_MSG_LEN - 1);
    }
    
    LINX_LOGE("Error set: %d - %s", error_code, g_linx_context.error_info.last_error_msg);
}

const char* linx_error_code_to_string(int error_code) {
    switch (error_code) {
        case LINX_OK: return "Success";
        case LINX_ERROR_INVALID_PARAM: return "Invalid parameter";
        case LINX_ERROR_NOT_CONNECTED: return "Not connected";
        case LINX_ERROR_NETWORK: return "Network error";
        case LINX_ERROR_PROTOCOL: return "Protocol error";
        case LINX_ERROR_AUDIO: return "Audio error";
        case LINX_ERROR_MEMORY: return "Memory error";
        case LINX_ERROR_TIMEOUT: return "Timeout error";
        case LINX_ERROR_JSON_PARSE: return "JSON parse error";
        case LINX_ERROR_ALREADY_INIT: return "Already initialized";
        case LINX_ERROR_NOT_INIT: return "Not initialized";
        default: return "Unknown error";
    }
}

int linx_validate_config(const linx_config_t *config) {
    if (!config) {
        return LINX_ERROR_INVALID_PARAM;
    }
    
    if (strlen(config->device_id) == 0) {
        LINX_LOGE("Device ID is required");
        return LINX_ERROR_INVALID_PARAM;
    }
    
    if (strlen(config->client_id) == 0) {
        LINX_LOGE("Client ID is required");
        return LINX_ERROR_INVALID_PARAM;
    }
    
    if (strlen(config->server_url) == 0) {
        LINX_LOGE("Server URL is required");
        return LINX_ERROR_INVALID_PARAM;
    }
    
    if (config->audio && (config->audio_params.sample_rate <= 0 || config->audio_params.channels <= 0)) {
        LINX_LOGE("Invalid audio parameters");
        return LINX_ERROR_INVALID_PARAM;
    }
    
    return LINX_OK;
}

void linx_generate_session_id(char *session_id, size_t len) {
    if (!session_id || len < 16) {
        return;
    }
    
    /* 简单的会话ID生成（实际应用中可以使用更复杂的算法） */
    long long timestamp = linx_get_timestamp_ms();
    snprintf(session_id, len, "session_%lld", timestamp);
}

long long linx_get_timestamp_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

void linx_reset_connection_state(void) {
    g_linx_context.connection_state.state = LINX_STATE_DISCONNECTED;
    g_linx_context.connection_state.hello_received = false;
    g_linx_context.connection_state.listening = false;
    g_linx_context.session_info.tts_playing = false;
    g_linx_context.conn = NULL;
}