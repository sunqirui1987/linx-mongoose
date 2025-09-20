/**
 * @file test_linx_sdk.c
 * @brief LinxSdk 测试程序
 * @version 1.0.0
 * @date 2024
 * 
 * 本文件用于测试 LinxSdk 的基本功能，包括创建、连接、发送消息等。
 */

#include "linx_sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 事件回调函数
void on_sdk_event(const LinxEvent* event, void* user_data) {
    if (!event) {
        return;
    }
    
    switch (event->type) {
        case LINX_EVENT_STATE_CHANGED:
            printf("状态变化: %d -> %d\n", 
                event->data.state_changed.old_state,
                event->data.state_changed.new_state);
            break;
            
        case LINX_EVENT_TEXT_MESSAGE:
            printf("收到文本消息 [%s]: %s\n",
                event->data.text_message.role,
                event->data.text_message.text);
            break;
            
        case LINX_EVENT_AUDIO_DATA:
            printf("收到音频数据: %zu 字节\n", event->data.audio_data.size);
            break;
            
        case LINX_EVENT_ERROR:
            printf("错误: %s (代码: %d)\n",
                event->data.error.message,
                event->data.error.code);
            break;
            
        default:
            printf("未知事件类型: %d\n", event->type);
            break;
    }
}

int main(int argc, char* argv[]) {
    printf("LinxSdk 测试程序启动\n");
    printf("SDK版本: %s\n", LINX_SDK_VERSION);
    
    // 配置SDK
    LinxSdkConfig config = {0};
    strncpy(config.server_url, "ws://localhost:8080/ws", sizeof(config.server_url) - 1);
    config.sample_rate = 16000;
    config.channels = 1;
    config.timeout_ms = 30000;
    
    // 创建SDK实例
    printf("创建SDK实例...\n");
    LinxSdk* sdk = linx_sdk_create(&config);
    if (!sdk) {
        printf("错误: 创建SDK实例失败\n");
        return -1;
    }
    printf("SDK实例创建成功\n");
    
    // 设置事件回调
    LinxSdkError error = linx_sdk_set_event_callback(sdk, on_sdk_event, NULL);
    if (error != LINX_SDK_SUCCESS) {
        printf("错误: 设置事件回调失败 (%d)\n", error);
        linx_sdk_destroy(sdk);
        return -1;
    }
    printf("事件回调设置成功\n");
    
    // 获取初始状态
    LinxDeviceState state = linx_sdk_get_state(sdk);
    printf("初始状态: %d\n", state);
    
    // 连接到服务器
    printf("连接到服务器...\n");
    error = linx_sdk_connect(sdk);
    if (error != LINX_SDK_SUCCESS) {
        printf("错误: 连接失败 (%d)\n", error);
        linx_sdk_destroy(sdk);
        return -1;
    }
    printf("连接请求已发送\n");
    
    // 等待连接建立
    printf("等待连接建立...\n");
    for (int i = 0; i < 10; i++) {
        sleep(1);
        state = linx_sdk_get_state(sdk);
        printf("当前状态: %d\n", state);
        
        if (state == LINX_DEVICE_STATE_IDLE) {
            printf("连接已建立\n");
            break;
        } else if (state == LINX_DEVICE_STATE_ERROR) {
            printf("连接失败\n");
            break;
        }
    }
    
    // 发送测试文本消息
    if (state == LINX_DEVICE_STATE_IDLE) {
        printf("发送测试文本消息...\n");
        error = linx_sdk_send_text(sdk, "Hello, LinxSdk!");
        if (error == LINX_SDK_SUCCESS) {
            printf("文本消息发送成功\n");
        } else {
            printf("错误: 文本消息发送失败 (%d)\n", error);
        }
        
        // 发送测试音频数据
        printf("发送测试音频数据...\n");
        uint8_t test_audio[1024] = {0};
        // 填充一些测试数据
        for (int i = 0; i < 1024; i++) {
            test_audio[i] = (uint8_t)(i % 256);
        }
        
        error = linx_sdk_send_audio(sdk, test_audio, sizeof(test_audio));
        if (error == LINX_SDK_SUCCESS) {
            printf("音频数据发送成功\n");
        } else {
            printf("错误: 音频数据发送失败 (%d)\n", error);
        }
    }
    
    // 运行一段时间以接收事件
    printf("运行10秒以接收事件...\n");
    for (int i = 0; i < 10; i++) {
        sleep(1);
        printf(".");
        fflush(stdout);
    }
    printf("\n");
    
    // 断开连接
    printf("断开连接...\n");
    error = linx_sdk_disconnect(sdk);
    if (error == LINX_SDK_SUCCESS) {
        printf("断开连接成功\n");
    } else {
        printf("错误: 断开连接失败 (%d)\n", error);
    }
    
    // 销毁SDK实例
    printf("销毁SDK实例...\n");
    linx_sdk_destroy(sdk);
    printf("SDK实例已销毁\n");
    
    printf("测试程序结束\n");
    return 0;
}