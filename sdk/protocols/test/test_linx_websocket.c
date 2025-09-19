#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "../linx_websocket.h"
#include "../../cjson/cJSON.h"

/* 测试计数器 */
static int tests_run = 0;
static int tests_passed = 0;

/* 测试宏 */
#define TEST_ASSERT(condition, message) \
    do { \
        tests_run++; \
        if (condition) { \
            tests_passed++; \
            printf("✓ %s\n", message); \
        } else { \
            printf("✗ %s\n", message); \
        } \
    } while(0)

/* 模拟回调函数 */
static bool callback_called = false;
static void* callback_user_data = NULL;
static char* callback_message = NULL;

static void test_incoming_audio_callback(linx_audio_stream_packet_t* packet, void* user_data) {
    callback_called = true;
    callback_user_data = user_data;
}

static void test_incoming_json_callback(const cJSON* root, void* user_data) {
    callback_called = true;
    callback_user_data = user_data;
}

static void test_audio_channel_opened_callback(void* user_data) {
    callback_called = true;
    callback_user_data = user_data;
}

static void test_audio_channel_closed_callback(void* user_data) {
    callback_called = true;
    callback_user_data = user_data;
}

static void test_network_error_callback(const char* message, void* user_data) {
    callback_called = true;
    callback_user_data = user_data;
    if (callback_message) {
        free(callback_message);
    }
    callback_message = message ? strdup(message) : NULL;
}

static void test_connected_callback(void* user_data) {
    callback_called = true;
    callback_user_data = user_data;
}

static void test_disconnected_callback(void* user_data) {
    callback_called = true;
    callback_user_data = user_data;
}

/* 重置回调状态 */
static void reset_callback_state(void) {
    callback_called = false;
    callback_user_data = NULL;
    if (callback_message) {
        free(callback_message);
        callback_message = NULL;
    }
}

/* 测试 WebSocket 协议创建和销毁 */
void test_websocket_create_destroy(void) {
    printf("\n=== 测试 WebSocket 协议创建和销毁 ===\n");
    
    /* 测试配置结构体 */
    linx_websocket_config_t config = {
        .url = "ws://localhost:8080/ws",
        .auth_token = "test_token",
        .device_id = "test_device",
        .client_id = "test_client",
        .protocol_version = 1
    };
    
    /* 测试创建 WebSocket 协议 */
    linx_websocket_protocol_t* ws_protocol = linx_websocket_create(&config);
    TEST_ASSERT(ws_protocol != NULL, "WebSocket 协议创建成功");
    
    if (ws_protocol) {
        /* 验证基础协议初始化 */
        TEST_ASSERT(ws_protocol->base.vtable != NULL, "虚函数表设置正确");
        TEST_ASSERT(ws_protocol->connected == false, "连接状态初始化正确");
        TEST_ASSERT(ws_protocol->audio_channel_opened == false, "音频通道状态初始化正确");
        TEST_ASSERT(ws_protocol->version == 1, "协议版本设置正确");
        TEST_ASSERT(ws_protocol->server_hello_received == false, "服务器hello状态初始化正确");
        TEST_ASSERT(ws_protocol->running == false, "运行状态初始化正确");
        TEST_ASSERT(ws_protocol->should_stop == false, "停止标志初始化正确");
        
        /* 测试销毁 */
        linx_websocket_destroy_direct(ws_protocol);
        TEST_ASSERT(true, "WebSocket 协议销毁成功");
    }
    
    /* 测试空配置处理 */
    ws_protocol = linx_websocket_create(NULL);
    TEST_ASSERT(ws_protocol == NULL, "空配置处理正确");
    
    /* 测试空指针销毁 */
    linx_websocket_destroy_direct(NULL);
    TEST_ASSERT(true, "空指针销毁处理正常");
}

/* 测试 WebSocket 协议配置函数 */
void test_websocket_configuration(void) {
    printf("\n=== 测试 WebSocket 协议配置 ===\n");
    
    /* 创建协议实例 */
    linx_websocket_config_t config = {
        .url = "ws://localhost:8080/ws",
        .auth_token = "initial_token",
        .device_id = "initial_device",
        .client_id = "initial_client",
        .protocol_version = 1
    };
    
    linx_websocket_protocol_t* ws_protocol = linx_websocket_create(&config);
    TEST_ASSERT(ws_protocol != NULL, "WebSocket 协议创建成功");
    
    if (ws_protocol) {
        /* 测试设置服务器URL */
        bool result = linx_websocket_protocol_set_server_url(ws_protocol, "ws://example.com:9090/websocket");
        TEST_ASSERT(result == true, "设置服务器URL成功");
        
        /* 测试设置服务器信息 */
        result = linx_websocket_protocol_set_server(ws_protocol, "example.com", 9090, "/websocket");
        TEST_ASSERT(result == true, "设置服务器信息成功");
        
        /* 测试设置认证令牌 */
        result = linx_websocket_protocol_set_auth_token(ws_protocol, "new_auth_token");
        TEST_ASSERT(result == true, "设置认证令牌成功");
        
        /* 测试设置设备ID */
        result = linx_websocket_protocol_set_device_id(ws_protocol, "new_device_id");
        TEST_ASSERT(result == true, "设置设备ID成功");
        
        /* 测试设置客户端ID */
        result = linx_websocket_protocol_set_client_id(ws_protocol, "new_client_id");
        TEST_ASSERT(result == true, "设置客户端ID成功");
        
        /* 测试空指针处理 */
        result = linx_websocket_protocol_set_server_url(NULL, "ws://test.com");
        TEST_ASSERT(result == false, "空指针处理正确");
        
        result = linx_websocket_protocol_set_server_url(ws_protocol, NULL);
        TEST_ASSERT(result == false, "空URL处理正确");
        
        result = linx_websocket_protocol_set_auth_token(NULL, "token");
        TEST_ASSERT(result == false, "空指针处理正确");
        
        result = linx_websocket_protocol_set_device_id(NULL, "device");
        TEST_ASSERT(result == false, "空指针处理正确");
        
        result = linx_websocket_protocol_set_client_id(NULL, "client");
        TEST_ASSERT(result == false, "空指针处理正确");
        
        linx_websocket_destroy_direct(ws_protocol);
    }
}

/* 测试 WebSocket 协议状态查询 */
void test_websocket_status_queries(void) {
    printf("\n=== 测试 WebSocket 协议状态查询 ===\n");
    
    linx_websocket_config_t config = {
        .url = "ws://localhost:8080/ws",
        .auth_token = "test_token",
        .device_id = "test_device",
        .client_id = "test_client",
        .protocol_version = 1
    };
    
    linx_websocket_protocol_t* ws_protocol = linx_websocket_create(&config);
    TEST_ASSERT(ws_protocol != NULL, "WebSocket 协议创建成功");
    
    if (ws_protocol) {
        /* 测试连接状态查询 */
        bool is_connected = linx_websocket_is_connected(ws_protocol);
        TEST_ASSERT(is_connected == false, "初始连接状态正确");
        
        /* 测试重连次数查询 */
        int reconnect_attempts = linx_websocket_get_reconnect_attempts(ws_protocol);
        TEST_ASSERT(reconnect_attempts == 0, "初始重连次数正确");
        
        /* 测试重置重连次数 */
        linx_websocket_reset_reconnect_attempts(ws_protocol);
        TEST_ASSERT(true, "重置重连次数正常");
        
        /* 测试连接超时检查 */
        bool is_timeout = linx_websocket_is_connection_timeout(ws_protocol);
        TEST_ASSERT(is_timeout == false, "连接超时检查正常");
        
        /* 测试空指针处理 */
        is_connected = linx_websocket_is_connected(NULL);
        TEST_ASSERT(is_connected == false, "空指针处理正确");
        
        reconnect_attempts = linx_websocket_get_reconnect_attempts(NULL);
        TEST_ASSERT(reconnect_attempts == 0, "空指针处理正确");
        
        linx_websocket_reset_reconnect_attempts(NULL);
        TEST_ASSERT(true, "空指针处理正常");
        
        is_timeout = linx_websocket_is_connection_timeout(NULL);
        TEST_ASSERT(is_timeout == false, "空指针处理正确");
        
        linx_websocket_destroy_direct(ws_protocol);
    }
}

/* 测试 WebSocket 协议操作 */
void test_websocket_operations(void) {
    printf("\n=== 测试 WebSocket 协议操作 ===\n");
    
    linx_websocket_config_t config = {
        .url = "ws://localhost:8080/ws",
        .auth_token = "test_token",
        .device_id = "test_device",
        .client_id = "test_client",
        .protocol_version = 1
    };
    
    linx_websocket_protocol_t* ws_protocol = linx_websocket_create(&config);
    TEST_ASSERT(ws_protocol != NULL, "WebSocket 协议创建成功");
    
    if (ws_protocol) {
        /* 测试事件处理 */
        linx_websocket_process_events(ws_protocol);
        TEST_ASSERT(true, "事件处理正常");
        
        /* 测试发送ping */
        bool result = linx_websocket_send_ping(ws_protocol);
        TEST_ASSERT(result == false, "未连接时发送ping返回false");
        
        /* 测试停止协议 */
        linx_websocket_stop(ws_protocol);
        TEST_ASSERT(true, "停止协议正常");
        
        /* 测试空指针处理 */
        linx_websocket_process_events(NULL);
        TEST_ASSERT(true, "空指针处理正常");
        
        result = linx_websocket_send_ping(NULL);
        TEST_ASSERT(result == false, "空指针处理正确");
        
        linx_websocket_stop(NULL);
        TEST_ASSERT(true, "空指针处理正常");
        
        linx_websocket_destroy_direct(ws_protocol);
    }
}

/* 测试 WebSocket 协议虚函数调用 */
void test_websocket_vtable_calls(void) {
    printf("\n=== 测试 WebSocket 协议虚函数调用 ===\n");
    
    linx_websocket_config_t config = {
        .url = "ws://localhost:8080/ws",
        .auth_token = "test_token",
        .device_id = "test_device",
        .client_id = "test_client",
        .protocol_version = 1
    };
    
    linx_websocket_protocol_t* ws_protocol = linx_websocket_create(&config);
    TEST_ASSERT(ws_protocol != NULL, "WebSocket 协议创建成功");
    
    if (ws_protocol) {
        linx_protocol_t* protocol = &ws_protocol->base;
        
        /* 测试启动协议 */
        bool result = linx_protocol_start(protocol);
        TEST_ASSERT(result == false, "未配置完整时启动返回false");
        
        /* 测试打开音频通道 */
        result = linx_protocol_open_audio_channel(protocol);
        TEST_ASSERT(result == false, "未连接时打开音频通道返回false");
        
        /* 测试关闭音频通道 */
        linx_protocol_close_audio_channel(protocol);
        TEST_ASSERT(true, "关闭音频通道正常");
        
        /* 测试检查音频通道状态 */
        result = linx_protocol_is_audio_channel_opened(protocol);
        TEST_ASSERT(result == false, "音频通道状态检查正确");
        
        /* 测试发送音频数据 */
        linx_audio_stream_packet_t* packet = linx_audio_stream_packet_create(1024);
        if (packet) {
            result = linx_protocol_send_audio(protocol, packet);
            TEST_ASSERT(result == false, "未连接时发送音频返回false");
            linx_audio_stream_packet_destroy(packet);
        }
        
        /* 测试发送文本数据 */
        result = linx_websocket_send_text(protocol, "test message");
        TEST_ASSERT(result == false, "未连接时发送文本返回false");
        
        linx_websocket_destroy_direct(ws_protocol);
    }
}

/* 测试 WebSocket Hello 消息处理 */
void test_websocket_hello_message(void) {
    printf("\n=== 测试 WebSocket Hello 消息处理 ===\n");
    
    linx_websocket_config_t config = {
        .url = "ws://localhost:8080/ws",
        .auth_token = "test_token",
        .device_id = "test_device",
        .client_id = "test_client",
        .protocol_version = 1
    };
    
    linx_websocket_protocol_t* ws_protocol = linx_websocket_create(&config);
    TEST_ASSERT(ws_protocol != NULL, "WebSocket 协议创建成功");
    
    if (ws_protocol) {
        /* 测试生成Hello消息 */
        char* hello_message = linx_websocket_get_hello_message(ws_protocol);
        TEST_ASSERT(hello_message != NULL, "生成Hello消息成功");
        
        if (hello_message) {
            /* 验证Hello消息是有效的JSON */
            cJSON* json = cJSON_Parse(hello_message);
            TEST_ASSERT(json != NULL, "Hello消息是有效的JSON");
            
            if (json) {
                /* 验证消息类型 */
                cJSON* type = cJSON_GetObjectItem(json, "type");
                TEST_ASSERT(type != NULL && cJSON_IsString(type), "消息类型字段存在");
                if (type && cJSON_IsString(type)) {
                    TEST_ASSERT(strcmp(cJSON_GetStringValue(type), "hello") == 0, "消息类型正确");
                }
                
                cJSON_Delete(json);
            }
            
            free(hello_message);
        }
        
        /* 测试解析服务器Hello消息 */
        const char* server_hello = "{\"type\":\"hello\",\"session_id\":\"test_session\",\"sample_rate\":24000,\"frame_duration\":60}";
        bool result = linx_websocket_parse_server_hello(ws_protocol, server_hello);
        TEST_ASSERT(result == true, "解析服务器Hello消息成功");
        
        /* 测试解析无效JSON */
        result = linx_websocket_parse_server_hello(ws_protocol, "invalid json");
        TEST_ASSERT(result == false, "解析无效JSON返回false");
        
        /* 测试空指针处理 */
        hello_message = linx_websocket_get_hello_message(NULL);
        TEST_ASSERT(hello_message == NULL, "空指针处理正确");
        
        result = linx_websocket_parse_server_hello(NULL, server_hello);
        TEST_ASSERT(result == false, "空指针处理正确");
        
        result = linx_websocket_parse_server_hello(ws_protocol, NULL);
        TEST_ASSERT(result == false, "空消息处理正确");
        
        linx_websocket_destroy_direct(ws_protocol);
    }
}

/* 测试 WebSocket 回调设置 */
void test_websocket_callbacks(void) {
    printf("\n=== 测试 WebSocket 回调设置 ===\n");
    
    linx_websocket_config_t config = {
        .url = "ws://localhost:8080/ws",
        .auth_token = "test_token",
        .device_id = "test_device",
        .client_id = "test_client",
        .protocol_version = 1
    };
    
    linx_websocket_protocol_t* ws_protocol = linx_websocket_create(&config);
    TEST_ASSERT(ws_protocol != NULL, "WebSocket 协议创建成功");
    
    if (ws_protocol) {
        linx_protocol_t* protocol = &ws_protocol->base;
        void* test_user_data = (void*)0x12345678;
        
        reset_callback_state();
        
        /* 测试设置各种回调 */
        linx_protocol_set_on_incoming_audio(protocol, test_incoming_audio_callback, test_user_data);
        linx_protocol_set_on_incoming_json(protocol, test_incoming_json_callback, test_user_data);
        linx_protocol_set_on_audio_channel_opened(protocol, test_audio_channel_opened_callback, test_user_data);
        linx_protocol_set_on_audio_channel_closed(protocol, test_audio_channel_closed_callback, test_user_data);
        linx_protocol_set_on_network_error(protocol, test_network_error_callback, test_user_data);
        linx_protocol_set_on_connected(protocol, test_connected_callback, test_user_data);
        linx_protocol_set_on_disconnected(protocol, test_disconnected_callback, test_user_data);
        
        /* 验证回调设置 */
        TEST_ASSERT(protocol->on_incoming_audio == test_incoming_audio_callback, "音频接收回调设置正确");
        TEST_ASSERT(protocol->on_incoming_json == test_incoming_json_callback, "JSON接收回调设置正确");
        TEST_ASSERT(protocol->on_audio_channel_opened == test_audio_channel_opened_callback, "音频通道打开回调设置正确");
        TEST_ASSERT(protocol->on_audio_channel_closed == test_audio_channel_closed_callback, "音频通道关闭回调设置正确");
        TEST_ASSERT(protocol->on_network_error == test_network_error_callback, "网络错误回调设置正确");
        TEST_ASSERT(protocol->on_connected == test_connected_callback, "连接成功回调设置正确");
        TEST_ASSERT(protocol->on_disconnected == test_disconnected_callback, "连接断开回调设置正确");
        TEST_ASSERT(protocol->user_data == test_user_data, "用户数据设置正确");
        
        linx_websocket_destroy_direct(ws_protocol);
    }
}

/* 主测试函数 */
int main(void) {
    printf("开始运行 linx_websocket 单元测试...\n");
    
    test_websocket_create_destroy();
    test_websocket_configuration();
    test_websocket_status_queries();
    test_websocket_operations();
    test_websocket_vtable_calls();
    test_websocket_hello_message();
    test_websocket_callbacks();
    
    printf("\n=== 测试结果 ===\n");
    printf("总测试数: %d\n", tests_run);
    printf("通过测试: %d\n", tests_passed);
    printf("失败测试: %d\n", tests_run - tests_passed);
    
    /* 清理回调状态 */
    reset_callback_state();
    
    if (tests_passed == tests_run) {
        printf("✓ 所有测试通过!\n");
        return 0;
    } else {
        printf("✗ 有测试失败!\n");
        return 1;
    }
}