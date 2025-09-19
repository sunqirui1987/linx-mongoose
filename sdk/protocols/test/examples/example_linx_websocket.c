/**
 * @file example_linx_websocket.c
 * @brief linx_websocket 使用示例
 * 
 * 这个示例展示了如何使用 linx_websocket 进行 WebSocket 连接，
 * 包括配置、连接、消息处理和断开连接。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "linx_websocket.h"

// 示例回调函数
static void on_websocket_connected(void* user_data) {
    printf("🔗 WebSocket 连接已建立\n");
}

static void on_websocket_disconnected(void* user_data) {
    printf("🔌 WebSocket 连接已断开\n");
}

static void on_websocket_error(const char* error_msg, void* user_data) {
    printf("❌ WebSocket 错误: %s\n", error_msg);
}

static void on_websocket_message(const cJSON* root, void* user_data) {
    char* json_string = cJSON_Print(root);
    if (json_string) {
        printf("📨 收到 WebSocket JSON 消息: %s\n", json_string);
        free(json_string);
    }
}

static void on_websocket_audio_data(linx_audio_stream_packet_t* packet, void* user_data) {
    printf("🎵 收到音频数据: %zu 字节, 采样率: %d, 帧时长: %d\n", 
           packet->payload_size, packet->sample_rate, packet->frame_duration);
}

int main() {
    printf("🚀 linx_websocket 使用示例\n");
    printf("==========================\n\n");

    // 1. 创建 WebSocket 协议实例
    printf("1️⃣ 创建 WebSocket 协议实例...\n");
    
    // 配置参数
    linx_websocket_config_t config = {
        .url = "ws://xrobo-io.qiniuapi.com/v1/ws/", //不支持wss 【这个demo】
        .auth_token = "test-token",
        .device_id = "98:a3:16:f9:d9:34",
        .client_id = "test-client",
        .protocol_version = 1
    };
    
    linx_websocket_protocol_t* ws = linx_websocket_create(&config);
    if (!ws) {
        fprintf(stderr, "❌ 创建 WebSocket 协议失败\n");
        return 1;
    }
    printf("✅ WebSocket 协议创建成功\n\n");

    // 2. 设置回调函数
    printf("2️⃣ 设置回调函数...\n");
    linx_protocol_set_on_connected((linx_protocol_t*)ws, on_websocket_connected, NULL);
    linx_protocol_set_on_disconnected((linx_protocol_t*)ws, on_websocket_disconnected, NULL);
    linx_protocol_set_on_network_error((linx_protocol_t*)ws, on_websocket_error, NULL);
    linx_protocol_set_on_incoming_json((linx_protocol_t*)ws, on_websocket_message, NULL);
    linx_protocol_set_on_incoming_audio((linx_protocol_t*)ws, on_websocket_audio_data, NULL);
    printf("✅ 回调函数设置完成\n\n");

    // 3. 启动 WebSocket 连接
    printf("3️⃣ 启动 WebSocket 连接...\n");
    if (linx_websocket_start((linx_protocol_t*)ws)) {
        printf("✅ WebSocket 连接启动成功\n");
    } else {
        printf("❌ WebSocket 连接启动失败\n");
        linx_websocket_destroy_direct(ws);
        return 1;
    }
    printf("\n");

    // 4. 等待连接建立并处理事件
    printf("4️⃣ 等待连接建立...\n");
    for (int i = 0; i < 10; i++) {
        linx_websocket_process_events(ws);
        if (linx_websocket_is_connected(ws)) {
            printf("✅ WebSocket 连接已建立\n");
            break;
        }
        usleep(500000); // 等待 500ms
    }
    printf("\n");

    // 5. 打开音频通道
    printf("5️⃣ 打开音频通道...\n");
    if (linx_websocket_open_audio_channel((linx_protocol_t*)ws)) {
        printf("✅ 音频通道打开成功\n");
        
        // 发送测试音频数据
        printf("   📤 发送测试音频数据...\n");
        linx_audio_stream_packet_t* packet = linx_audio_stream_packet_create(1024);
        if (packet) {
            packet->sample_rate = 16000;
            packet->frame_duration = 20;
            packet->timestamp = 0;
            memset(packet->payload, 0, packet->payload_size); // 填充测试数据
            
            if (linx_websocket_send_audio((linx_protocol_t*)ws, packet)) {
                printf("✅ 音频数据发送成功\n");
            } else {
                printf("❌ 音频数据发送失败\n");
            }
            
            linx_audio_stream_packet_destroy(packet);
        }
        
        // 关闭音频通道
        linx_websocket_close_audio_channel((linx_protocol_t*)ws);
        printf("✅ 音频通道关闭\n");
    } else {
        printf("❌ 音频通道打开失败\n");
    }
    printf("\n");

    // 6. 发送文本消息
    printf("6️⃣ 发送文本消息...\n");
    const char* test_message = "Hello from WebSocket client!";
    if (linx_websocket_send_text((linx_protocol_t*)ws, test_message)) {
        printf("✅ 文本消息发送成功: %s\n", test_message);
    } else {
        printf("❌ 文本消息发送失败\n");
    }
    printf("\n");

    // 7. 处理事件一段时间
    printf("7️⃣ 处理事件...\n");
    for (int i = 0; i < 20; i++) {
        linx_websocket_process_events(ws);
        usleep(100000); // 等待 100ms
    }
    printf("✅ 事件处理完成\n\n");

    // 8. 检查连接状态
    printf("8️⃣ 检查连接状态...\n");
    printf("   🔗 连接状态: %s\n", 
           linx_websocket_is_connected(ws) ? "已连接" : "未连接");
    printf("✅ 连接状态检查完成\n\n");

    // 9. 停止并清理
    printf("9️⃣ 停止并清理资源...\n");
    linx_websocket_stop(ws);
    linx_websocket_destroy((linx_protocol_t*)ws);
    printf("✅ 资源清理完成\n\n");

    printf("🎉 linx_websocket 示例运行完成！\n");
    return 0;
}