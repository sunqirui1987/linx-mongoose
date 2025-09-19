/**
 * @file example_linx_websocket.c
 * @brief linx_websocket 长连接应用示例
 * 
 * 这个示例展示了如何使用 linx_websocket 创建一个持续运行的长连接应用，
 * 类似于完整的语音交互客户端，包括多线程处理、状态管理和优雅退出。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include "linx_websocket.h"

// ==================== 全局状态管理 ====================

/**
 * @brief 应用程序状态结构体
 */
typedef struct {
    bool running;                    // 应用程序运行状态
    bool connected;                  // WebSocket连接状态
    char* session_id;               // 会话ID
    char* listen_state;             // 监听状态: "start" 或 "stop"
    char* tts_state;                // TTS状态: "start", "stop", "idle"
    pthread_mutex_t state_mutex;    // 状态访问互斥锁
} app_state_t;

// 全局状态实例
static app_state_t g_app_state = {
    .running = true,
    .connected = false,
    .session_id = NULL,
    .listen_state = NULL,
    .tts_state = NULL,
    .state_mutex = PTHREAD_MUTEX_INITIALIZER
};

// 全局WebSocket协议实例
static linx_websocket_protocol_t* g_ws_protocol = NULL;

// ==================== 状态管理函数 ====================

static void set_app_running(bool running) {
    pthread_mutex_lock(&g_app_state.state_mutex);
    g_app_state.running = running;
    pthread_mutex_unlock(&g_app_state.state_mutex);
}

static bool is_app_running() {
    pthread_mutex_lock(&g_app_state.state_mutex);
    bool running = g_app_state.running;
    pthread_mutex_unlock(&g_app_state.state_mutex);
    return running;
}

static void set_session_id(const char* session_id) {
    pthread_mutex_lock(&g_app_state.state_mutex);
    if (g_app_state.session_id) {
        free(g_app_state.session_id);
    }
    g_app_state.session_id = session_id ? strdup(session_id) : NULL;
    pthread_mutex_unlock(&g_app_state.state_mutex);
}

static void set_listen_state(const char* state) {
    pthread_mutex_lock(&g_app_state.state_mutex);
    if (g_app_state.listen_state) {
        free(g_app_state.listen_state);
    }
    g_app_state.listen_state = state ? strdup(state) : NULL;
    pthread_mutex_unlock(&g_app_state.state_mutex);
}

static void set_tts_state(const char* state) {
    pthread_mutex_lock(&g_app_state.state_mutex);
    if (g_app_state.tts_state) {
        free(g_app_state.tts_state);
    }
    g_app_state.tts_state = state ? strdup(state) : NULL;
    pthread_mutex_unlock(&g_app_state.state_mutex);
}

// ==================== 信号处理 ====================

static void signal_handler(int sig) {
    printf("\n🛑 收到信号 %d，准备退出...\n", sig);
    set_app_running(false);
}

// ==================== WebSocket 回调函数 ====================

static void on_websocket_connected(void* user_data) {
    printf("🔗 WebSocket 连接已建立\n");
    pthread_mutex_lock(&g_app_state.state_mutex);
    g_app_state.connected = true;
    pthread_mutex_unlock(&g_app_state.state_mutex);
}

static void on_websocket_disconnected(void* user_data) {
    printf("🔌 WebSocket 连接已断开\n");
    pthread_mutex_lock(&g_app_state.state_mutex);
    g_app_state.connected = false;
    pthread_mutex_unlock(&g_app_state.state_mutex);
    set_app_running(false);
}

static void on_websocket_error(const char* error_msg, void* user_data) {
    printf("❌ WebSocket 错误: %s\n", error_msg);
}

static void on_websocket_message(const cJSON* root, void* user_data) {
    char* json_string = cJSON_Print(root);
    if (json_string) {
        printf("📨 收到消息: %s\n", json_string);
        
        // 解析消息类型
        const cJSON* type = cJSON_GetObjectItem(root, "type");
        if (type && cJSON_IsString(type)) {
            // 处理hello响应
            if (strcmp(type->valuestring, "hello") == 0) {
                const cJSON* session_id = cJSON_GetObjectItem(root, "session_id");
                if (session_id && cJSON_IsString(session_id)) {
                    set_session_id(session_id->valuestring);
                    printf("✅ 会话建立，ID: %s\n", session_id->valuestring);
                    
                    // 开始监听
                    set_listen_state("start");
                    printf("🎤 开始语音监听\n");
                }
            }
            // 处理TTS状态
            else if (strcmp(type->valuestring, "tts") == 0) {
                const cJSON* state = cJSON_GetObjectItem(root, "state");
                if (state && cJSON_IsString(state)) {
                    set_tts_state(state->valuestring);
                    printf("🔊 TTS状态: %s\n", state->valuestring);
                    
                    if (strcmp(state->valuestring, "start") == 0) {
                        set_listen_state("stop");
                        printf("🔇 停止监听（TTS播放中）\n");
                    } else if (strcmp(state->valuestring, "stop") == 0) {
                        set_listen_state("start");
                        printf("🎤 恢复语音监听\n");
                    }
                }
            }
            // 处理goodbye消息
            else if (strcmp(type->valuestring, "goodbye") == 0) {
                printf("👋 会话结束\n");
                set_session_id(NULL);
            }
        }
        
        free(json_string);
    }
}

static void on_websocket_audio_data(linx_audio_stream_packet_t* packet, void* user_data) {
    printf("🎵 收到音频数据: %zu 字节, 采样率: %d, 帧时长: %d\n", 
           packet->payload_size, packet->sample_rate, packet->frame_duration);
    
    // 这里可以添加音频播放逻辑
    // 例如：将音频数据推入播放缓冲区
}

// ==================== 工作线程函数 ====================

/**
 * @brief WebSocket事件处理线程
 */
static void* websocket_event_thread(void* arg) {
    printf("🔄 WebSocket事件处理线程启动\n");
    
    while (is_app_running()) {
        if (g_ws_protocol) {
            linx_websocket_process_events(g_ws_protocol);
        }
        usleep(10000); // 10ms
    }
    
    printf("🔄 WebSocket事件处理线程退出\n");
    return NULL;
}

/**
 * @brief 模拟音频录制线程
 */
static void* audio_record_thread(void* arg) {
    printf("🎤 音频录制线程启动\n");
    
    while (is_app_running()) {
        // 检查是否应该录制音频
        pthread_mutex_lock(&g_app_state.state_mutex);
        bool should_record = g_app_state.connected && 
                           g_app_state.listen_state && 
                           strcmp(g_app_state.listen_state, "start") == 0;
        pthread_mutex_unlock(&g_app_state.state_mutex);
        
        if (should_record && g_ws_protocol) {
            // 模拟音频数据发送
            linx_audio_stream_packet_t* packet = linx_audio_stream_packet_create(960);
            if (packet) {
                packet->sample_rate = 16000;
                packet->frame_duration = 60;
                packet->timestamp = time(NULL) * 1000;
                
                // 填充模拟音频数据（静音）
                memset(packet->payload, 0, packet->payload_size);
                
                if (linx_websocket_send_audio((linx_protocol_t*)g_ws_protocol, packet)) {
                    printf("🎵 发送音频数据: %zu 字节\n", packet->payload_size);
                }
                
                linx_audio_stream_packet_destroy(packet);
            }
        }
        
        usleep(60000); // 60ms，对应音频帧间隔
    }
    
    printf("🎤 音频录制线程退出\n");
    return NULL;
}

/**
 * @brief 模拟音频播放线程
 */
static void* audio_playback_thread(void* arg) {
    printf("🔊 音频播放线程启动\n");
    
    while (is_app_running()) {
        // 这里可以添加音频播放逻辑
        // 例如：从音频缓冲区取出数据并播放
        usleep(20000); // 20ms
    }
    
    printf("🔊 音频播放线程退出\n");
    return NULL;
}

/**
 * @brief 状态监控线程
 */
static void* status_monitor_thread(void* arg) {
    printf("📊 状态监控线程启动\n");
    
    while (is_app_running()) {
        pthread_mutex_lock(&g_app_state.state_mutex);
        printf("📊 状态报告 - 连接: %s, 会话: %s, 监听: %s, TTS: %s\n",
               g_app_state.connected ? "已连接" : "未连接",
               g_app_state.session_id ? g_app_state.session_id : "无",
               g_app_state.listen_state ? g_app_state.listen_state : "无",
               g_app_state.tts_state ? g_app_state.tts_state : "无");
        pthread_mutex_unlock(&g_app_state.state_mutex);
        
        sleep(10); // 每10秒报告一次状态
    }
    
    printf("📊 状态监控线程退出\n");
    return NULL;
}

// ==================== 主函数 ====================

int main() {
    printf("🚀 Linx WebSocket 长连接应用\n");
    printf("============================\n\n");

    // 1. 设置信号处理
    printf("1️⃣ 设置信号处理...\n");
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    printf("✅ 信号处理设置完成\n\n");

    // 2. 创建 WebSocket 协议实例
    printf("2️⃣ 创建 WebSocket 协议实例...\n");
    
    linx_websocket_config_t config = {
        .url = "ws://xrobo-io.qiniuapi.com/v1/ws/",
        .auth_token = "test-token",
        .device_id = "98:a3:16:f9:d9:34",
        .client_id = "test-client",
        .protocol_version = 2
    };
    
    g_ws_protocol = linx_websocket_create(&config);
    if (!g_ws_protocol) {
        fprintf(stderr, "❌ 创建 WebSocket 协议失败\n");
        return 1;
    }
    printf("✅ WebSocket 协议创建成功\n\n");

    // 3. 设置回调函数
    printf("3️⃣ 设置回调函数...\n");
    linx_protocol_set_on_connected((linx_protocol_t*)g_ws_protocol, on_websocket_connected, NULL);
    linx_protocol_set_on_disconnected((linx_protocol_t*)g_ws_protocol, on_websocket_disconnected, NULL);
    linx_protocol_set_on_network_error((linx_protocol_t*)g_ws_protocol, on_websocket_error, NULL);
    linx_protocol_set_on_incoming_json((linx_protocol_t*)g_ws_protocol, on_websocket_message, NULL);
    linx_protocol_set_on_incoming_audio((linx_protocol_t*)g_ws_protocol, on_websocket_audio_data, NULL);
    printf("✅ 回调函数设置完成\n\n");

    // 4. 启动 WebSocket 连接
    printf("4️⃣ 启动 WebSocket 连接...\n");
    if (!linx_websocket_start((linx_protocol_t*)g_ws_protocol)) {
        printf("❌ WebSocket 连接启动失败\n");
        linx_websocket_destroy_direct(g_ws_protocol);
        return 1;
    }
    printf("✅ WebSocket 连接启动成功\n\n");

    // 5. 启动工作线程
    printf("5️⃣ 启动工作线程...\n");
    
    pthread_t websocket_thread, audio_record_thread_id, audio_playback_thread_id, status_thread;
    
    if (pthread_create(&websocket_thread, NULL, websocket_event_thread, NULL) != 0) {
        fprintf(stderr, "❌ 创建WebSocket事件线程失败\n");
        goto cleanup;
    }
    
    if (pthread_create(&audio_record_thread_id, NULL, audio_record_thread, NULL) != 0) {
        fprintf(stderr, "❌ 创建音频录制线程失败\n");
        goto cleanup;
    }
    
    if (pthread_create(&audio_playback_thread_id, NULL, audio_playback_thread, NULL) != 0) {
        fprintf(stderr, "❌ 创建音频播放线程失败\n");
        goto cleanup;
    }
    
    if (pthread_create(&status_thread, NULL, status_monitor_thread, NULL) != 0) {
        fprintf(stderr, "❌ 创建状态监控线程失败\n");
        goto cleanup;
    }
    
    printf("✅ 所有工作线程启动成功\n\n");

    // 6. 主循环 - 等待连接建立
    printf("6️⃣ 等待连接建立...\n");
    int connection_timeout = 30; // 30秒超时
    while (is_app_running() && connection_timeout > 0) {
        pthread_mutex_lock(&g_app_state.state_mutex);
        bool connected = g_app_state.connected;
        pthread_mutex_unlock(&g_app_state.state_mutex);
        
        if (connected) {
            printf("✅ WebSocket 连接已建立\n");
            break;
        }
        
        sleep(1);
        connection_timeout--;
    }
    
    if (connection_timeout <= 0) {
        printf("⏰ 连接超时，退出应用\n");
        set_app_running(false);
    }

    // 7. 主循环 - 保持应用运行
    printf("\n7️⃣ 应用运行中...\n");
    printf("💡 按 Ctrl+C 退出应用\n\n");
    
    while (is_app_running()) {
        sleep(1);
    }

    // 8. 等待所有线程结束
    printf("\n8️⃣ 等待线程结束...\n");
    
    pthread_join(websocket_thread, NULL);
    pthread_join(audio_record_thread_id, NULL);
    pthread_join(audio_playback_thread_id, NULL);
    pthread_join(status_thread, NULL);
    
    printf("✅ 所有线程已结束\n");

cleanup:
    // 9. 清理资源
    printf("\n9️⃣ 清理资源...\n");
    
    if (g_ws_protocol) {
        linx_websocket_stop(g_ws_protocol);
        linx_websocket_destroy((linx_protocol_t*)g_ws_protocol);
        g_ws_protocol = NULL;
    }
    
    // 清理状态
    pthread_mutex_lock(&g_app_state.state_mutex);
    if (g_app_state.session_id) {
        free(g_app_state.session_id);
        g_app_state.session_id = NULL;
    }
    if (g_app_state.listen_state) {
        free(g_app_state.listen_state);
        g_app_state.listen_state = NULL;
    }
    if (g_app_state.tts_state) {
        free(g_app_state.tts_state);
        g_app_state.tts_state = NULL;
    }
    pthread_mutex_unlock(&g_app_state.state_mutex);
    
    printf("✅ 资源清理完成\n\n");
    printf("👋 Linx WebSocket 长连接应用退出\n");
    
    return 0;
}