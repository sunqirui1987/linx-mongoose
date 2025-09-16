/**
 * @file basic_example.c
 * @brief LINX WebSocket SDK基本使用示例
 * @version 1.0.0
 * @date 2024
 * 
 * 本示例展示如何使用LINX WebSocket SDK进行基本的音频数据传输和接收。
 * SDK不进行音频编解码，只负责通过回调函数传递原始音频数据。
 */

#include "../include/linx_websocket_sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

/* 全局变量已在文件开头定义 */
static bool g_running = true;
static linx_context_t *g_context = NULL;
static bool is_connected = false;
static int audio_send_counter = 0;

/* 信号处理函数 */
void signal_handler(int sig) {
    printf("\nReceived signal %d, shutting down...\n", sig);
    g_running = false;
}

/* ========== 回调函数实现 ========== */

/* 回调函数实现 */
void on_connected(linx_context_t *ctx) {
    printf("[INFO] Connected to server\n");
    is_connected = true;
}

void on_disconnected(linx_context_t *ctx, int reason) {
    printf("[INFO] Disconnected from server, reason: %d\n", reason);
    is_connected = false;
}

/* on_ready回调已移除，使用on_connected代替 */

/**
 * 接收到音频数据回调（核心功能）
 * 这里接收到的是服务器发送的原始音频数据
 */
void on_audio_data(linx_context_t *ctx, const uint8_t *data, size_t len) {
    printf("[CALLBACK] Received audio data: %zu bytes\n", len);
    
    /* 在这里处理接收到的原始音频数据 */
    /* 例如：播放音频、保存到文件、进行音频处理等 */
    
    /* 示例：打印前几个字节的数据（用于调试） */
    if (len > 0) {
        printf("[AUDIO] First few bytes: ");
        size_t print_len = len > 16 ? 16 : len;
        for (size_t i = 0; i < print_len; i++) {
            printf("%02X ", data[i]);
        }
        printf("\n");
    }
    
    /* 示例：将音频数据保存到文件 */
    static FILE *audio_file = NULL;
    if (!audio_file) {
        audio_file = fopen("received_audio.raw", "wb");
        if (audio_file) {
            printf("[INFO] Created audio output file: received_audio.raw\n");
        }
    }
    
    if (audio_file) {
        fwrite(data, 1, len, audio_file);
        fflush(audio_file);
    }
}

/**
 * 接收到文本结果回调
 */
void on_text_result(linx_context_t *ctx, const char *text) {
    printf("[CALLBACK] Received text result: %s\n", text);
}

/**
 * 接收到音频结果回调
 */
void on_audio_result(linx_context_t *ctx, const uint8_t *data, size_t len) {
    printf("[CALLBACK] Received audio result: %zu bytes\n", len);
}

/**
 * 错误回调
 */
void on_error(linx_context_t *ctx, int error_code, const char *error_msg) {
    printf("[CALLBACK] Error occurred: code=%d, message=%s\n", 
           error_code, error_msg ? error_msg : "Unknown error");
}

/* ========== 音频数据模拟函数 ========== */

/**
 * 模拟发送音频数据
 * 在实际应用中，这里应该是从麦克风或音频文件读取的真实音频数据
 */
void simulate_audio_input(void) {
    static int counter = 0;
    
    /* 模拟PCM音频数据（16位，单声道，16kHz） */
    const size_t chunk_size = 1024; /* 每次发送1024字节 */
    uint8_t audio_data[chunk_size];
    
    /* 生成简单的正弦波音频数据（用于测试） */
    for (size_t i = 0; i < chunk_size / 2; i++) {
        int16_t sample = (int16_t)(32767 * 0.5 * sin(2.0 * 3.14159265359 * 440.0 * (counter * chunk_size / 2 + i) / 16000.0));
        audio_data[i * 2] = sample & 0xFF;
        audio_data[i * 2 + 1] = (sample >> 8) & 0xFF;
    }
    
    /* 发送音频数据 */
    int ret = linx_send_audio(audio_data, chunk_size);
    if (ret != LINX_OK) {
        printf("[ERROR] Failed to send audio data: %d\n", ret);
    } else {
        printf("[INFO] Sent audio chunk %d: %zu bytes\n", counter, chunk_size);
    }
    
    counter++;
}

/* ========== 主函数 ========== */

int main(int argc, char *argv[]) {
    (void)argc; /* 避免未使用参数警告 */
    (void)argv; /* 避免未使用参数警告 */
    printf("LINX WebSocket SDK Basic Example\n");
    printf("================================\n\n");
    
    /* 设置信号处理 */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    /* 配置SDK */
    linx_config_t config;
    memset(&config, 0, sizeof(config));
    
    /* 基本配置 */
    strncpy(config.server_url, "wss://xrobo-io.qiniuapi.com/v1/ws/", sizeof(config.server_url) - 1);
    strncpy(config.device_id, "D4:06:06:B6:A9:FB", sizeof(config.device_id) - 1);  /* MAC地址格式 */
    strncpy(config.client_id, "web_test_client", sizeof(config.client_id) - 1);
    strncpy(config.token, "your_auth_token_here", sizeof(config.token) - 1);
    
    /* 音频参数配置（原始PCM格式） */
    config.audio = true;
    strncpy(config.audio_params.format, "pcm", sizeof(config.audio_params.format) - 1);
    config.audio_params.sample_rate = 16000;
    config.audio_params.channels = 1;
    config.audio_params.bits_per_sample = 16;
    
    /* 监听模式配置 */
    config.listen_mode = LINX_LISTEN_AUTO;
    
    /* 日志配置 */
    config.log_level = LINX_LOG_INFO;
    
    /* 连接配置 */
    config.connect_timeout_ms = 10000;
    config.heartbeat_interval_ms = 30000;
    config.max_reconnect_attempts = 3;
    
    /* 设置回调函数 */
    config.callbacks.on_connected = on_connected;
    config.callbacks.on_disconnected = on_disconnected;
    config.callbacks.on_audio_data = on_audio_data;  /* 核心回调：接收原始音频数据 */
    config.callbacks.on_text_result = on_text_result;
    config.callbacks.on_audio_result = on_audio_result;
    config.callbacks.on_error = on_error;
    
    /* 初始化SDK */
    int ret = linx_sdk_init(&config);
    if (ret != LINX_OK) {
        printf("[ERROR] Failed to initialize SDK: %d\n", ret);
        return 1;
    }
    
    printf("[INFO] SDK initialized successfully\n");
    
    printf("[INFO] Configuration applied successfully\n");
    
    /* 连接到服务器 */
    ret = linx_start_listen(config.listen_mode);
    if (ret != LINX_OK) {
        printf("[ERROR] Failed to connect: %d\n", ret);
        linx_sdk_cleanup();
        return 1;
    }
    
    printf("[INFO] Connecting to server...\n");
    
    /* 主循环 */
    int audio_send_counter = 0;
    while (g_running) {
        /* 处理网络事件 */
        ret = linx_poll_events(100); /* 100ms超时 */
        if (ret != LINX_OK && ret != LINX_ERROR_TIMEOUT) {
            printf("[ERROR] Poll failed: %d\n", ret);
            break;
        }
        
        /* 模拟定期发送音频数据（每500ms发送一次） */
        audio_send_counter++;
        if (audio_send_counter >= 5) { /* 5 * 100ms = 500ms */
            /* 简化状态检查，直接发送音频数据 */
            simulate_audio_input();
            audio_send_counter = 0;
        }
        
        /* 检查错误状态 */
        char error_msg[256];
        if (linx_get_last_error(error_msg, sizeof(error_msg)) != LINX_OK) {
            printf("[ERROR] Last error: %s\n", error_msg);
        }
    }
    
    printf("\n[INFO] Shutting down...\n");
    
    /* 停止监听 */
    ret = linx_stop_listen();
    if (ret != LINX_OK) {
        printf("[WARN] Failed to stop listen: %d\n", ret);
    }
    
    /* 清理SDK */
    linx_sdk_cleanup();
    
    printf("[INFO] SDK cleaned up\n");
    printf("[INFO] Example completed\n");
    
    return 0;
}