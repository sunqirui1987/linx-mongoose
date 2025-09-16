/**
 * @file linx_internal.h
 * @brief 灵矽AI WebSocket SDK内部头文件
 * @version 1.0.0
 * @date 2024
 * 
 * SDK内部数据结构和私有函数定义
 */

#ifndef LINX_INTERNAL_H
#define LINX_INTERNAL_H

#include "linx_websocket_sdk.h"
#include "mongoose.h"
#include "cjson/cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 内部常量定义 */
#define LINX_MAX_MSG_NODES        16
#define LINX_WEBSOCKET_PROTOCOL   "websocket"
#define LINX_PROTOCOL_VERSION     "1"
#define LINX_USER_AGENT          "LinxSDK/1.0.0"

/* 消息类型枚举 */
typedef enum {
    LINX_MSG_JSON = 0,
    LINX_MSG_AUDIO = 1
} linx_msg_type_t;

/* 消息队列节点 */
typedef struct linx_msg_node {
    linx_msg_type_t type;
    union {
        struct {
            char *json_data;
            size_t json_len;
        } json;
        struct {
            uint8_t *audio_data;
            size_t audio_len;
        } audio;
    } data;
    struct linx_msg_node *next;
} linx_msg_node_t;

/* SDK主上下文结构 */
struct linx_context {
    /* 连接状态 */
    struct {
        linx_connection_state_t state;
        bool hello_received;
        bool listening;
        bool audio_streaming;
        time_t last_heartbeat;
        int reconnect_count;
    } connection_state;
    
    /* 会话信息 */
    struct {
        char session_id[64];
        linx_listen_mode_t listen_mode;
        bool tts_playing;
        time_t session_start;
    } session_info;
    
    /* 音频缓冲区 */
    struct {
        uint8_t send_buffer[LINX_AUDIO_BUFFER_SIZE];
        size_t send_buffer_len;
        uint8_t recv_buffer[LINX_AUDIO_BUFFER_SIZE];
        size_t recv_buffer_len;
    } audio_buffers;
    
    /* 配置数据 */
    linx_config_t config;
    
    /* Mongoose连接 */
    struct mg_connection *conn;
    struct mg_mgr mgr;
    
    /* 错误信息 */
    struct {
        int last_error_code;
        char last_error_msg[LINX_MAX_ERROR_MSG_LEN];
    } error_info;
    
    /* 消息队列 */
    linx_msg_node_t *msg_queue_head;
    linx_msg_node_t *msg_queue_tail;
    
    /* 统计信息 */
    struct {
        size_t audio_bytes_sent;
        size_t audio_bytes_received;
        size_t messages_sent;
        size_t messages_received;
    } stats;
    
    /* 错误信息 */
    struct {
        int code;
        char message[256];
    } last_error;
    
    /* 用户数据 */
    void *user_data;
    
    /* 初始化标志 */
    bool initialized;
};

/* 全局变量声明 */
extern linx_context_t g_linx_context;
extern linx_msg_node_t g_msg_pool[LINX_MAX_MSG_NODES];
extern bool g_msg_pool_used[LINX_MAX_MSG_NODES];

/* ========== 内部函数声明 ========== */

/* 内存管理函数 */
linx_msg_node_t* linx_alloc_msg_node(void);
void linx_free_msg_node(linx_msg_node_t *node);
void linx_init_memory_pool(void);
void linx_cleanup_memory_pool(void);

/* 消息队列函数 */
int linx_enqueue_message(linx_msg_type_t type, const void *data, size_t len);
linx_msg_node_t* linx_dequeue_message(void);
void linx_clear_message_queue(void);

/* JSON处理函数 */
int linx_parse_json_message(const char *json_str, size_t len);
int linx_build_hello_message(char *buffer, size_t buffer_size);
int linx_build_audio_start_message(char *buffer, size_t buffer_size);
int linx_build_audio_end_message(char *buffer, size_t buffer_size);
int linx_build_heartbeat_message(char *buffer, size_t buffer_size);
int linx_parse_hello_response(const char *json_str, linx_hello_msg_t *hello_msg);
int linx_parse_tts_message(const char *json_str, linx_tts_msg_t *tts_msg);
int linx_parse_emotion_message(const char *json_str, linx_emotion_msg_t *emotion_msg);

/* 消息处理函数 - 原有接口（向后兼容） */
int linx_handle_hello_ack_message(const char *json_str, size_t json_len);
int linx_handle_audio_start_ack_message(const char *json_str, size_t json_len);
int linx_handle_audio_end_ack_message(const char *json_str, size_t json_len);
int linx_handle_text_result_message(const char *json_str, size_t json_len);
int linx_handle_audio_result_message(const char *json_str, size_t json_len);
int linx_handle_heartbeat_ack_message(const char *json_str, size_t json_len);
int linx_handle_error_message(const char *json_str, size_t json_len);

/* 消息处理函数 - cJSON版本 */
int linx_handle_hello_ack_message_cjson(cJSON *root);
int linx_handle_audio_start_ack_message_cjson(cJSON *root);
int linx_handle_audio_end_ack_message_cjson(cJSON *root);
int linx_handle_heartbeat_ack_message_cjson(cJSON *root);
int linx_handle_error_message_cjson(cJSON *root);

/* WebSocket事件处理函数 */
void linx_websocket_event_handler(struct mg_connection *c, int ev, void *ev_data);
void linx_handle_websocket_open(struct mg_connection *c);
void linx_handle_websocket_message(struct mg_connection *c, struct mg_ws_message *wm);
void linx_handle_websocket_close(struct mg_connection *c);
void linx_handle_websocket_error(struct mg_connection *c, const char *error_msg);

/* 连接管理函数 */
int linx_establish_connection(void);
int linx_send_hello_message(void);
int linx_handle_reconnect(void);
void linx_reset_connection_state(void);

/* 音频处理函数 */
int linx_send_audio_data(const uint8_t *data, size_t len);
void linx_handle_audio_data(const uint8_t *data, size_t len);

/* 错误处理函数 */
void linx_set_error(int error_code, const char *error_msg);
const char* linx_error_code_to_string(int error_code);

/* 工具函数 */
int linx_validate_config(const linx_config_t *config);
void linx_generate_session_id(char *session_id, size_t len);
long long linx_get_timestamp_ms(void);
int linx_url_encode(const char *src, char *dst, size_t dst_size);

/* 日志函数 */
void linx_log(linx_log_level_t level, const char *file, int line, 
              const char *func, const char *format, ...);

/* 日志级别定义 */
#define LINX_LOG_ERROR   0
#define LINX_LOG_WARN    1
#define LINX_LOG_INFO    2
#define LINX_LOG_DEBUG   3
#define LINX_LOG_TRACE   4

/* 日志宏定义 */
#define LINX_LOGE(fmt, ...) linx_log(LINX_LOG_ERROR, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LINX_LOGW(fmt, ...) linx_log(LINX_LOG_WARN,  __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LINX_LOGI(fmt, ...) linx_log(LINX_LOG_INFO,  __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LINX_LOGD(fmt, ...) linx_log(LINX_LOG_DEBUG, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LINX_LOGT(fmt, ...) linx_log(LINX_LOG_TRACE, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* LINX_INTERNAL_H */