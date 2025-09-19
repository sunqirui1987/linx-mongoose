/**
 * @file example_linx_protocol.c
 * @brief linx_protocol 使用示例
 * 
 * 这个示例展示了如何使用 linx_protocol 进行基本的协议操作，
 * 包括初始化、配置、音频数据处理和清理。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "linx_protocol.h"

// 示例回调函数
static void on_incoming_audio(linx_audio_stream_packet_t* packet, void* user_data) {
    printf("📥 收到音频数据: %zu 字节, 采样率: %d Hz, 时间戳: %u\n", 
           packet->payload_size, packet->sample_rate, packet->timestamp);
    // 在实际应用中，这里会处理音频数据
}

static void on_incoming_json(const cJSON* root, void* user_data) {
    char* json_string = cJSON_Print(root);
    if (json_string) {
        printf("💬 收到JSON消息: %s\n", json_string);
        free(json_string);
    }
}

static void on_connected(void* user_data) {
    printf("🔔 协议事件: CONNECTED\n");
}

static void on_disconnected(void* user_data) {
    printf("🔔 协议事件: DISCONNECTED\n");
}

static void on_network_error(const char* message, void* user_data) {
    printf("🔔 网络错误: %s\n", message);
}

static void on_audio_channel_opened(void* user_data) {
    printf("🔔 音频通道已打开\n");
}

static void on_audio_channel_closed(void* user_data) {
    printf("🔔 音频通道已关闭\n");
}

int main() {
    printf("🚀 linx_protocol 使用示例\n");
    printf("========================\n\n");

    // 1. 创建协议实例
    printf("1️⃣ 创建协议实例...\n");
    // 注意：实际使用中需要创建具体的协议实现（如WebSocket协议）
    // 这里我们创建一个基础协议实例用于演示
    linx_protocol_t* protocol = (linx_protocol_t*)malloc(sizeof(linx_protocol_t));
    if (!protocol) {
        fprintf(stderr, "❌ 创建协议失败\n");
        return 1;
    }
    
    // 初始化协议（使用NULL虚函数表，仅用于演示）
    linx_protocol_init(protocol, NULL);
    printf("✅ 协议创建成功\n\n");

    // 2. 设置回调函数
    printf("2️⃣ 设置回调函数...\n");
    linx_protocol_set_on_incoming_audio(protocol, on_incoming_audio, NULL);
    linx_protocol_set_on_incoming_json(protocol, on_incoming_json, NULL);
    linx_protocol_set_on_connected(protocol, on_connected, NULL);
    linx_protocol_set_on_disconnected(protocol, on_disconnected, NULL);
    linx_protocol_set_on_network_error(protocol, on_network_error, NULL);
    linx_protocol_set_on_audio_channel_opened(protocol, on_audio_channel_opened, NULL);
    linx_protocol_set_on_audio_channel_closed(protocol, on_audio_channel_closed, NULL);
    printf("✅ 回调函数设置完成\n\n");

    // 3. 获取协议信息
    printf("3️⃣ 协议信息:\n");
    printf("   📊 服务器采样率: %d Hz\n", linx_protocol_get_server_sample_rate(protocol));
    printf("   ⏱️  服务器帧持续时间: %d ms\n", linx_protocol_get_server_frame_duration(protocol));
    const char* session_id = linx_protocol_get_session_id(protocol);
    printf("   🆔 会话ID: %s\n", session_id ? session_id : "未设置");
    printf("\n");

    // 4. 启动协议
    printf("4️⃣ 启动协议...\n");
    // 注意：由于使用了NULL虚函数表，实际的启动操作会失败，这里仅用于演示
    bool start_result = linx_protocol_start(protocol);
    if (start_result) {
        printf("✅ 协议启动成功\n");
    } else {
        printf("❌ 协议启动失败（预期结果，因为使用了演示用的NULL虚函数表）\n");
    }
    printf("\n");

    // 5. 模拟音频通道操作
    printf("5️⃣ 音频通道操作...\n");
    
    // 打开音频通道
    bool open_result = linx_protocol_open_audio_channel(protocol);
    if (open_result) {
        printf("✅ 音频通道已打开\n");
    } else {
        printf("❌ 打开音频通道失败（预期结果，因为使用了演示用的NULL虚函数表）\n");
    }

    // 模拟发送一些音频数据
    printf("📤 发送测试音频数据...\n");
    
    // 创建音频数据包
    linx_audio_stream_packet_t* audio_packet = linx_audio_stream_packet_create(1024);
    if (audio_packet) {
        // 填充测试数据
        memset(audio_packet->payload, 0x42, audio_packet->payload_size);
        audio_packet->sample_rate = 16000;
        audio_packet->frame_duration = 20;
        audio_packet->timestamp = 12345;
        
        bool send_result = linx_protocol_send_audio(protocol, audio_packet);
        if (send_result) {
            printf("✅ 音频数据发送成功\n");
        } else {
            printf("❌ 音频数据发送失败（预期结果，因为使用了演示用的NULL虚函数表）\n");
        }
        
        linx_audio_stream_packet_destroy(audio_packet);
    }

    // 关闭音频通道
    linx_protocol_close_audio_channel(protocol);
    printf("✅ 音频通道关闭操作已调用\n");
    printf("\n");

    // 6. 创建和使用音频数据包
    printf("6️⃣ 音频数据包操作...\n");
    
    linx_audio_stream_packet_t* packet = linx_audio_stream_packet_create(1024);
    if (packet) {
        printf("✅ 音频数据包创建成功 (大小: %zu 字节)\n", packet->payload_size);
        
        // 模拟填充数据
        memset(packet->payload, 0x42, packet->payload_size);
        packet->sample_rate = 16000;
        packet->frame_duration = 20;
        packet->timestamp = 12345;
        
        printf("   📦 数据包时间戳: %u\n", packet->timestamp);
        printf("   📏 数据包大小: %zu 字节\n", packet->payload_size);
        printf("   📊 采样率: %d Hz\n", packet->sample_rate);
        printf("   ⏱️  帧持续时间: %d ms\n", packet->frame_duration);
        
        // 销毁数据包
        linx_audio_stream_packet_destroy(packet);
        printf("✅ 音频数据包已销毁\n");
    } else {
        printf("❌ 创建音频数据包失败\n");
    }
    printf("\n");

    // 7. 发送协议消息
    printf("7️⃣ 发送协议消息...\n");
    const char* test_message = "Hello from example!";
    
    // 发送MCP消息
    linx_protocol_send_mcp_message(protocol, test_message);
    printf("✅ MCP消息发送操作已调用: %s\n", test_message);
    
    // 发送唤醒词检测消息
    linx_protocol_send_wake_word_detected(protocol, "小爱同学");
    printf("✅ 唤醒词检测消息发送操作已调用\n");
    
    // 发送开始监听消息
    linx_protocol_send_start_listening(protocol, LINX_LISTENING_MODE_AUTO_STOP);
    printf("✅ 开始监听消息发送操作已调用\n");
    
    // 发送停止监听消息
    linx_protocol_send_stop_listening(protocol);
    printf("✅ 停止监听消息发送操作已调用\n");
    
    // 发送中止说话消息
    linx_protocol_send_abort_speaking(protocol, LINX_ABORT_REASON_WAKE_WORD_DETECTED);
    printf("✅ 中止说话消息发送操作已调用\n");
    printf("\n");

    // 8. 模拟运行一段时间
    printf("8️⃣ 运行协议 (3秒)...\n");
    for (int i = 3; i > 0; i--) {
        printf("   ⏳ %d 秒后停止...\n", i);
        sleep(1);
    }
    printf("\n");

    // 9. 检查协议状态
    printf("9️⃣ 检查协议状态...\n");
    bool is_audio_opened = linx_protocol_is_audio_channel_opened(protocol);
    printf("   🔊 音频通道状态: %s\n", is_audio_opened ? "已打开" : "已关闭");
    
    bool is_timeout = linx_protocol_is_timeout(protocol);
    printf("   ⏰ 超时状态: %s\n", is_timeout ? "已超时" : "正常");
    printf("\n");

    // 10. 清理资源
    printf("🔟 清理资源...\n");
    linx_protocol_destroy(protocol);
    free(protocol);  // 释放我们手动分配的内存
    printf("✅ 协议资源已清理\n\n");

    printf("🎉 示例运行完成！\n");
    printf("========================\n");
    printf("\n注意：此示例使用了NULL虚函数表，因此大部分操作会失败。\n");
    printf("在实际使用中，您需要创建具体的协议实现（如WebSocket协议）。\n");
    
    return 0;
}