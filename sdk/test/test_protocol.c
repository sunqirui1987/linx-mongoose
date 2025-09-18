#include "sdk/protocols/linx_protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 测试回调函数
void test_on_incoming_json(const cJSON* json_data, void* user_data) {
    char* json_string = cJSON_Print(json_data);
    printf("收到JSON消息: %s\n", json_string);
    free(json_string);
}

void test_on_network_error(const char* message, void* user_data) {
    printf("网络错误: %s\n", message);
}

void test_on_connected(void* user_data) {
    printf("连接成功\n");
}

void test_on_disconnected(void* user_data) {
    printf("连接断开\n");
}

// 模拟发送文本的函数
bool mock_send_text(linx_protocol_t* protocol, const char* text) {
    printf("发送文本: %s\n", text);
    return true;
}

// 创建模拟的vtable
linx_protocol_vtable_t mock_vtable = {
    .send_text = mock_send_text,
    .start = NULL,
    .open_audio_channel = NULL,
    .close_audio_channel = NULL,
    .is_audio_channel_opened = NULL,
    .send_audio = NULL
};

int main() {
    printf("=== LINX协议功能测试 ===\n\n");
    
    // 测试协议初始化
    linx_protocol_t protocol;
    linx_protocol_init(&protocol, &mock_vtable);
    
    // 设置会话ID
    protocol.session_id = "test_session_123";
    
    // 设置回调函数
    linx_protocol_set_on_incoming_json(&protocol, test_on_incoming_json, NULL);
    linx_protocol_set_on_network_error(&protocol, test_on_network_error, NULL);
    linx_protocol_set_on_connected(&protocol, test_on_connected, NULL);
    linx_protocol_set_on_disconnected(&protocol, test_on_disconnected, NULL);
    
    printf("1. 测试唤醒词检测消息格式:\n");
    linx_protocol_send_wake_word_detected(&protocol, "小爱同学");
    
    printf("\n2. 测试开始监听消息格式:\n");
    linx_protocol_send_start_listening(&protocol, LINX_LISTENING_MODE_AUTO_STOP);
    linx_protocol_send_start_listening(&protocol, LINX_LISTENING_MODE_MANUAL_STOP);
    linx_protocol_send_start_listening(&protocol, LINX_LISTENING_MODE_REALTIME);
    
    printf("\n3. 测试停止监听消息格式:\n");
    linx_protocol_send_stop_listening(&protocol);
    
    printf("\n4. 测试中止说话消息格式:\n");
    linx_protocol_send_abort_speaking(&protocol, LINX_ABORT_REASON_NONE);
    linx_protocol_send_abort_speaking(&protocol, LINX_ABORT_REASON_WAKE_WORD_DETECTED);
    
    printf("\n5. 测试MCP消息发送:\n");
    linx_protocol_send_mcp_message(&protocol, "{\"method\":\"test\",\"params\":{}}");
    
    printf("\n6. 测试超时检查:\n");
    printf("超时时间设置: 120秒\n");
    bool is_timeout = linx_protocol_is_timeout(&protocol);
    printf("当前是否超时: %s\n", is_timeout ? "是" : "否");
    
    printf("\n7. 测试错误处理:\n");
    linx_protocol_set_error(&protocol, "测试错误消息");
    
    printf("\n8. 测试音频数据包管理:\n");
    linx_audio_stream_packet_t* packet = linx_audio_stream_packet_create(1024);
    if (packet) {
        printf("音频数据包创建成功，载荷大小: %zu\n", packet->payload_size);
        linx_audio_stream_packet_destroy(packet);
        printf("音频数据包销毁成功\n");
    }
    
    // 清理资源（跳过destroy以避免释放字符串常量）
    linx_protocol_destroy(&protocol);
    
    printf("\n=== 测试完成 ===\n");
    return 0;
}