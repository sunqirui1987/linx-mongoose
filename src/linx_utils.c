/**
 * @file linx_utils.c
 * @brief 工具函数模块实现
 * @version 1.0.0
 * @date 2024
 */

#include "linx_internal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

/* ========== 内存管理函数 ========== */

void* linx_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    void *ptr = malloc(size);
    if (!ptr) {
        LINX_LOGE("Memory allocation failed: %zu bytes", size);
        linx_set_error(LINX_ERROR_MEMORY, "Memory allocation failed");
    }
    
    return ptr;
}

void* linx_calloc(size_t count, size_t size) {
    if (count == 0 || size == 0) {
        return NULL;
    }
    
    void *ptr = calloc(count, size);
    if (!ptr) {
        LINX_LOGE("Memory allocation failed: %zu * %zu bytes", count, size);
        linx_set_error(LINX_ERROR_MEMORY, "Memory allocation failed");
    }
    
    return ptr;
}

void* linx_realloc(void *ptr, size_t size) {
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr) {
        LINX_LOGE("Memory reallocation failed: %zu bytes", size);
        linx_set_error(LINX_ERROR_MEMORY, "Memory reallocation failed");
    }
    
    return new_ptr;
}

void linx_free(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}

/* ========== 消息队列管理函数 ========== */

int linx_queue_init(void) {
    /* 消息队列初始化已移至消息模块 */
    
    LINX_LOGD("Message queue initialized");
    return LINX_OK;
}

void linx_queue_cleanup(void) {
    /* 消息队列清理已移至消息模块 */
    LINX_LOGD("Message queue cleaned up");
}

int linx_queue_push(linx_msg_type_t type, const void *data, size_t data_len) {
    if (!data && data_len > 0) {
        return LINX_ERROR_INVALID_PARAM;
    }
    
    /* 消息队列功能已移至消息模块 */
    (void)type;
    return LINX_OK;
}

int linx_queue_pop(linx_msg_type_t *type, void **data, size_t *data_len) {
    /* 消息队列功能已移至消息模块 */
    return LINX_ERROR_QUEUE_EMPTY;
}

size_t linx_queue_size(void) {
    /* 消息队列功能已移至消息模块 */
    return 0;
}

/* ========== 错误处理函数 ========== */
/* 错误处理函数实现已移至linx_websocket_sdk.c */

void linx_clear_error(void) {
    g_linx_context.last_error.code = LINX_OK;
    g_linx_context.last_error.message[0] = '\0';
    /* 清除错误时间戳已移除 */
}

const char* linx_get_error_string(int error_code) {
    switch (error_code) {
        case LINX_OK:
            return "Success";
        case LINX_ERROR_INVALID_PARAM:
            return "Invalid parameter";
        case LINX_ERROR_MEMORY:
            return "Memory allocation failed";
        case LINX_ERROR_NETWORK:
            return "Network error";
        case LINX_ERROR_NOT_CONNECTED:
            return "Not connected";
        case LINX_ERROR_NOT_READY:
            return "SDK not ready";
        case LINX_ERROR_AUDIO:
            return "Audio error";
        case LINX_ERROR_AUDIO_NOT_STARTED:
            return "Audio not started";
        case LINX_ERROR_INVALID_MESSAGE:
            return "Invalid message";
        case LINX_ERROR_BUFFER_TOO_SMALL:
            return "Buffer too small";
        case LINX_ERROR_QUEUE_EMPTY:
            return "Queue is empty";
        case LINX_ERROR_TIMEOUT:
            return "Operation timeout";
        default:
            return "Unknown error";
    }
}

/* ========== 连接状态管理函数 ========== */
/* 连接状态管理函数实现已移至linx_websocket_sdk.c */

const char* linx_get_state_string(linx_connection_state_t state) {
    switch (state) {
        case LINX_STATE_DISCONNECTED:
            return "Disconnected";
        case LINX_STATE_CONNECTING:
            return "Connecting";
        case LINX_STATE_CONNECTED:
            return "Connected";
        case LINX_STATE_HELLO_SENT:
            return "Hello Sent";
        case LINX_STATE_READY:
            return "Ready";
        case LINX_STATE_ERROR:
            return "Error";
        default:
            return "Unknown";
    }
}

/* ========== 字符串工具函数 ========== */

char* linx_strdup(const char *str) {
    if (!str) {
        return NULL;
    }
    
    size_t len = strlen(str);
    char *copy = linx_malloc(len + 1);
    if (copy) {
        strcpy(copy, str);
    }
    
    return copy;
}

int linx_strncpy_safe(char *dest, const char *src, size_t dest_size) {
    if (!dest || !src || dest_size == 0) {
        return LINX_ERROR_INVALID_PARAM;
    }
    
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
    
    return LINX_OK;
}

/* ========== 时间工具函数 ========== */
/* 时间工具函数实现已移至linx_websocket_sdk.c */

int linx_sleep_ms(int milliseconds) {
    if (milliseconds <= 0) {
        return LINX_OK;
    }
    
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    
    return nanosleep(&ts, NULL) == 0 ? LINX_OK : LINX_ERROR_UNKNOWN;
}

/* ========== 日志函数声明 ========== */
/* 日志函数实现已移至linx_websocket_sdk.c */

/* ========== 配置验证函数 ========== */
/* 配置验证函数实现已移至linx_websocket_sdk.c */

/* ========== 消息队列函数实现 ========== */

void linx_init_memory_pool(void) {
    /* 内存池初始化已移至消息模块 */
    LINX_LOGD("Memory pool initialized");
}

void linx_cleanup_memory_pool(void) {
    /* 内存池清理已移至消息模块 */
    LINX_LOGD("Memory pool cleaned up");
}

int linx_enqueue_message(linx_msg_type_t type, const void *data, size_t len) {
    /* 消息入队功能已移至消息模块 */
    (void)type;
    (void)data;
    (void)len;
    return LINX_OK;
}

void linx_clear_message_queue(void) {
    /* 消息队列清理已移至消息模块 */
    LINX_LOGD("Message queue cleared");
}