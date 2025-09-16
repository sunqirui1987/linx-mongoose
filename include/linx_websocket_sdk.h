/**
 * @file linx_websocket_sdk.h
 * @brief 灵矽AI WebSocket嵌入式SDK头文件
 * @version 1.0.0
 * @date 2024
 * 
 * 基于Mongoose库实现的C99标准WebSocket客户端SDK
 * 专为嵌入式设备设计，提供灵矽AI语音交互能力
 */

#ifndef LINX_WEBSOCKET_SDK_H
#define LINX_WEBSOCKET_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <time.h>

/* 版本信息 */
#define LINX_SDK_VERSION_MAJOR 1
#define LINX_SDK_VERSION_MINOR 0
#define LINX_SDK_VERSION_PATCH 0
#define LINX_SDK_VERSION_STRING "1.0.0"

/* 配置常量 */
#define LINX_MAX_DEVICE_ID_LEN    64
#define LINX_MAX_CLIENT_ID_LEN    64
#define LINX_MAX_TOKEN_LEN        512
#define LINX_MAX_URL_LEN          256
#define LINX_MAX_ERROR_MSG_LEN    256
#define LINX_AUDIO_BUFFER_SIZE    8192
#define LINX_JSON_BUFFER_SIZE     4096
#define LINX_MAX_AUDIO_CHUNK_SIZE 4096
#define LINX_MAX_RECONNECT_TIMES  5
#define LINX_HEARTBEAT_INTERVAL   30

/* 音频参数常量 */
#define LINX_AUDIO_FORMAT         "pcm"
#define LINX_AUDIO_SAMPLE_RATE_UP 16000
#define LINX_AUDIO_SAMPLE_RATE_DN 24000
#define LINX_AUDIO_CHANNELS       1
#define LINX_AUDIO_BITS_PER_SAMPLE 16

/* 错误码定义 */
typedef enum {
    LINX_OK                    = 0,
    LINX_ERROR_INVALID_PARAM   = -1,
    LINX_ERROR_NOT_CONNECTED   = -2,
    LINX_ERROR_NETWORK         = -3,
    LINX_ERROR_PROTOCOL        = -4,
    LINX_ERROR_AUDIO           = -5,
    LINX_ERROR_MEMORY          = -6,
    LINX_ERROR_TIMEOUT         = -7,
    LINX_ERROR_JSON_PARSE      = -8,
    LINX_ERROR_ALREADY_INIT    = -9,
    LINX_ERROR_NOT_INIT        = -10,
    LINX_ERROR_NOT_READY       = -11,
    LINX_ERROR_AUDIO_NOT_STARTED   = -12,
    LINX_ERROR_QUEUE_EMPTY         = -13,
    LINX_ERROR_BUFFER_TOO_SMALL    = -14,
    LINX_ERROR_INVALID_MESSAGE     = -15,
    LINX_ERROR_SERVER              = -16,
    LINX_ERROR_UNKNOWN             = -17
} linx_error_code_t;

/* 日志级别 */
typedef enum {
    LINX_LOG_TRACE = 0,
    LINX_LOG_DEBUG = 1,
    LINX_LOG_INFO  = 2,
    LINX_LOG_WARN  = 3,
    LINX_LOG_ERROR = 4,
    LINX_LOG_FATAL = 5
} linx_log_level_t;

/* 监听模式 */
typedef enum {
    LINX_LISTEN_AUTO = 0,
    LINX_LISTEN_MANUAL = 1,
    LINX_LISTEN_REALTIME = 2
} linx_listen_mode_t;

/* 连接状态枚举 */
typedef enum {
    LINX_STATE_DISCONNECTED = 0,
    LINX_STATE_CONNECTING = 1,
    LINX_STATE_CONNECTED = 2,
    LINX_STATE_HELLO_SENT = 3,
    LINX_STATE_READY = 4,
    LINX_STATE_LISTENING = 5,
    LINX_STATE_ERROR = 6
} linx_connection_state_t;

/* 音频参数结构体 */
typedef struct {
    char format[16];          /* "pcm" 或 "raw" */
    int sample_rate;          /* 16000 (上行) / 24000 (下行) */
    int channels;             /* 1 */
    int bits_per_sample;      /* 16 */
} linx_audio_params_t;

/* 音频统计信息结构体 */
typedef struct {
    size_t bytes_sent;        /* 发送的音频字节数 */
    size_t bytes_received;    /* 接收的音频字节数 */
    size_t packets_sent;      /* 发送的消息数 */
    size_t packets_received;  /* 接收的消息数 */
} linx_audio_stats_t;

/* Hello消息结构体 */
typedef struct {
    char type[16];            /* "hello" */
    int version;              /* 1 */
    char transport[16];       /* "websocket" */
    linx_audio_params_t audio_params;
} linx_hello_msg_t;

/* TTS状态消息结构体 */
typedef struct {
    char type[16];            /* "tts" */
    char state[32];           /* "start", "stop", "sentence_start" */
    char text[256];           /* 播放文本（可选） */
} linx_tts_msg_t;

/* 情感状态消息结构体 */
typedef struct {
    char type[16];            /* "llm" */
    char text[8];             /* 表情符号 */
    char emotion[32];         /* 情感标识 */
} linx_emotion_msg_t;

/* 前置声明 */
struct linx_context;
typedef struct linx_context linx_context_t;

/* 回调函数类型定义 */
typedef void (*linx_on_connected_cb)(linx_context_t *ctx);
typedef void (*linx_on_disconnected_cb)(linx_context_t *ctx, int reason);
typedef void (*linx_on_hello_received_cb)(linx_context_t *ctx, const linx_hello_msg_t *msg);
typedef void (*linx_on_tts_status_cb)(linx_context_t *ctx, const linx_tts_msg_t *msg);
typedef void (*linx_on_emotion_cb)(linx_context_t *ctx, const linx_emotion_msg_t *msg);
typedef void (*linx_on_audio_data_cb)(linx_context_t *ctx, const uint8_t *data, size_t len);
typedef void (*linx_on_text_result_cb)(linx_context_t *ctx, const char *text);
typedef void (*linx_on_audio_result_cb)(linx_context_t *ctx, const uint8_t *data, size_t len);
typedef void (*linx_on_error_cb)(linx_context_t *ctx, int error_code, const char *error_msg);

/* 回调函数结构体 */
typedef struct {
    linx_on_connected_cb on_connected;
    linx_on_disconnected_cb on_disconnected;
    linx_on_hello_received_cb on_hello_received;
    linx_on_tts_status_cb on_tts_status;
    linx_on_emotion_cb on_emotion;
    linx_on_audio_data_cb on_audio_data;
    linx_on_text_result_cb on_text_result;
    linx_on_audio_result_cb on_audio_result;
    linx_on_error_cb on_error;
} linx_callbacks_t;

/* SDK配置结构体 */
typedef struct {
    char server_url[LINX_MAX_URL_LEN];
    char device_id[LINX_MAX_DEVICE_ID_LEN];
    char client_id[LINX_MAX_CLIENT_ID_LEN];
    char token[LINX_MAX_TOKEN_LEN];
    
    /* 音频配置 */
    bool audio;
    linx_audio_params_t audio_params;
    
    /* 监听模式 */
    linx_listen_mode_t listen_mode;
    
    /* 日志配置 */
    linx_log_level_t log_level;
    
    /* 回调函数 */
    linx_callbacks_t callbacks;
    
    /* 连接配置 */
    int connect_timeout_ms;
    int heartbeat_interval_ms;
    int max_reconnect_attempts;
} linx_config_t;

/* ========== 核心API函数 ========== */

/**
 * @brief 获取SDK版本信息
 * @return 版本字符串
 */
const char* linx_get_version(void);

/**
 * @brief 初始化SDK
 * @param config SDK配置结构体
 * @return 成功返回LINX_OK，失败返回错误码
 */
int linx_sdk_init(const linx_config_t *config);

/**
 * @brief 设置SDK配置
 * @param config SDK配置结构体
 * @return 成功返回LINX_OK，失败返回错误码
 */
int linx_set_config(const linx_config_t *config);

/**
 * @brief 清理SDK资源
 * @return 成功返回LINX_OK，失败返回错误码
 */
int linx_sdk_cleanup(void);

/**
 * @brief 建立WebSocket连接
 * @return 成功返回LINX_OK，失败返回错误码
 */
int linx_connect(void);

/**
 * @brief 断开WebSocket连接
 * @return 成功返回LINX_OK，失败返回错误码
 */
int linx_disconnect(void);

/**
 * @brief 获取当前连接状态
 * @return 连接状态枚举值
 */
linx_connection_state_t linx_get_connection_state(void);

/**
 * @brief 开始监听
 * @param mode 监听模式
 * @return 成功返回LINX_OK，失败返回错误码
 */
int linx_start_listen(linx_listen_mode_t mode);

/**
 * @brief 停止监听
 * @return 成功返回LINX_OK，失败返回错误码
 */
int linx_stop_listen(void);

/**
 * @brief 发送原始音频数据
 * @param data 音频数据指针
 * @param len 数据长度
 * @return 成功返回LINX_OK，失败返回错误码
 */
int linx_send_audio(const uint8_t *data, size_t len);

/**
 * @brief 处理网络事件（需要在主循环中调用）
 * @param timeout_ms 超时时间（毫秒）
 * @return 成功返回LINX_OK，失败返回错误码
 */
int linx_poll_events(int timeout_ms);

/**
 * @brief 获取最后错误信息
 * @param error_msg 错误消息缓冲区
 * @param msg_len 缓冲区长度
 * @return 错误码
 */
int linx_get_last_error(char *error_msg, size_t msg_len);

/**
 * @brief 设置日志级别
 * @param level 日志级别（0-4）
 */
void linx_set_log_level(linx_log_level_t level);

/**
 * @brief 获取SDK上下文（用于回调函数中获取用户数据）
 * @return SDK上下文指针
 */
linx_context_t* linx_get_context(void);

/**
 * @brief 设置用户数据到SDK上下文
 * @param user_data 用户数据指针
 */
void linx_set_user_data(void *user_data);

/**
 * @brief 从SDK上下文获取用户数据
 * @return 用户数据指针
 */
void* linx_get_user_data(void);

#ifdef __cplusplus
}
#endif

#endif /* LINX_WEBSOCKET_SDK_H */