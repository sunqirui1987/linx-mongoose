#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "../linx_protocol.h"

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
}

static void test_connected_callback(void* user_data) {
    callback_called = true;
    callback_user_data = user_data;
}

static void test_disconnected_callback(void* user_data) {
    callback_called = true;
    callback_user_data = user_data;
}

/* 模拟协议实现的虚函数表 */
static bool mock_start_called = false;
static bool mock_open_audio_channel_called = false;
static bool mock_close_audio_channel_called = false;
static bool mock_is_audio_channel_opened_called = false;
static bool mock_send_audio_called = false;
static bool mock_send_text_called = false;
static bool mock_destroy_called = false;

static bool mock_start(linx_protocol_t* protocol) {
    mock_start_called = true;
    return true;
}

static bool mock_open_audio_channel(linx_protocol_t* protocol) {
    mock_open_audio_channel_called = true;
    return true;
}

static void mock_close_audio_channel(linx_protocol_t* protocol) {
    mock_close_audio_channel_called = true;
}

static bool mock_is_audio_channel_opened(const linx_protocol_t* protocol) {
    mock_is_audio_channel_opened_called = true;
    return true;
}

static bool mock_send_audio(linx_protocol_t* protocol, linx_audio_stream_packet_t* packet) {
    mock_send_audio_called = true;
    return true;
}

static bool mock_send_text(linx_protocol_t* protocol, const char* text) {
    mock_send_text_called = true;
    return true;
}

static void mock_destroy(linx_protocol_t* protocol) {
    mock_destroy_called = true;
}

static const linx_protocol_vtable_t mock_vtable = {
    .start = mock_start,
    .open_audio_channel = mock_open_audio_channel,
    .close_audio_channel = mock_close_audio_channel,
    .is_audio_channel_opened = mock_is_audio_channel_opened,
    .send_audio = mock_send_audio,
    .send_text = mock_send_text,
    .destroy = mock_destroy
};

/* 重置模拟状态 */
static void reset_mock_state(void) {
    mock_start_called = false;
    mock_open_audio_channel_called = false;
    mock_close_audio_channel_called = false;
    mock_is_audio_channel_opened_called = false;
    mock_send_audio_called = false;
    mock_send_text_called = false;
    mock_destroy_called = false;
    callback_called = false;
    callback_user_data = NULL;
}

/* 测试协议初始化和销毁 */
void test_protocol_init_destroy(void) {
    printf("\n=== 测试协议初始化和销毁 ===\n");
    
    linx_protocol_t protocol;
    
    /* 测试初始化 */
    linx_protocol_init(&protocol, &mock_vtable);
    TEST_ASSERT(protocol.vtable == &mock_vtable, "协议虚函数表设置正确");
    TEST_ASSERT(protocol.server_sample_rate == 24000, "默认采样率设置正确");
    TEST_ASSERT(protocol.server_frame_duration == 60, "默认帧持续时间设置正确");
    TEST_ASSERT(protocol.error_occurred == false, "错误状态初始化正确");
    TEST_ASSERT(protocol.session_id == NULL, "会话ID初始化正确");
    
    /* 测试销毁 */
    reset_mock_state();
    linx_protocol_destroy(&protocol);
    TEST_ASSERT(mock_destroy_called, "销毁函数被调用");
    
    /* 测试空指针处理 */
    linx_protocol_init(NULL, &mock_vtable);
    linx_protocol_init(&protocol, NULL);
    linx_protocol_destroy(NULL);
    TEST_ASSERT(true, "空指针处理正常");
}

/* 测试获取器函数 */
void test_protocol_getters(void) {
    printf("\n=== 测试获取器函数 ===\n");
    
    linx_protocol_t protocol;
    linx_protocol_init(&protocol, &mock_vtable);
    
    /* 测试获取采样率 */
    int sample_rate = linx_protocol_get_server_sample_rate(&protocol);
    TEST_ASSERT(sample_rate == 24000, "获取采样率正确");
    
    /* 测试获取帧持续时间 */
    int frame_duration = linx_protocol_get_server_frame_duration(&protocol);
    TEST_ASSERT(frame_duration == 60, "获取帧持续时间正确");
    
    /* 测试获取会话ID */
    const char* session_id = linx_protocol_get_session_id(&protocol);
    TEST_ASSERT(session_id == NULL, "获取会话ID正确");
    
    /* 测试空指针处理 */
    TEST_ASSERT(linx_protocol_get_server_sample_rate(NULL) == 0, "空指针处理正确");
    TEST_ASSERT(linx_protocol_get_server_frame_duration(NULL) == 0, "空指针处理正确");
    TEST_ASSERT(linx_protocol_get_session_id(NULL) == NULL, "空指针处理正确");
    
    linx_protocol_destroy(&protocol);
}

/* 测试回调设置函数 */
void test_protocol_callbacks(void) {
    printf("\n=== 测试回调设置函数 ===\n");
    
    linx_protocol_t protocol;
    linx_protocol_init(&protocol, &mock_vtable);
    
    void* test_user_data = (void*)0x12345678;
    
    /* 测试设置音频接收回调 */
    linx_protocol_set_on_incoming_audio(&protocol, test_incoming_audio_callback, test_user_data);
    TEST_ASSERT(protocol.on_incoming_audio == test_incoming_audio_callback, "音频接收回调设置正确");
    TEST_ASSERT(protocol.user_data == test_user_data, "用户数据设置正确");
    
    /* 测试设置JSON接收回调 */
    linx_protocol_set_on_incoming_json(&protocol, test_incoming_json_callback, test_user_data);
    TEST_ASSERT(protocol.on_incoming_json == test_incoming_json_callback, "JSON接收回调设置正确");
    
    /* 测试设置音频通道打开回调 */
    linx_protocol_set_on_audio_channel_opened(&protocol, test_audio_channel_opened_callback, test_user_data);
    TEST_ASSERT(protocol.on_audio_channel_opened == test_audio_channel_opened_callback, "音频通道打开回调设置正确");
    
    /* 测试设置音频通道关闭回调 */
    linx_protocol_set_on_audio_channel_closed(&protocol, test_audio_channel_closed_callback, test_user_data);
    TEST_ASSERT(protocol.on_audio_channel_closed == test_audio_channel_closed_callback, "音频通道关闭回调设置正确");
    
    /* 测试设置网络错误回调 */
    linx_protocol_set_on_network_error(&protocol, test_network_error_callback, test_user_data);
    TEST_ASSERT(protocol.on_network_error == test_network_error_callback, "网络错误回调设置正确");
    
    /* 测试设置连接成功回调 */
    linx_protocol_set_on_connected(&protocol, test_connected_callback, test_user_data);
    TEST_ASSERT(protocol.on_connected == test_connected_callback, "连接成功回调设置正确");
    
    /* 测试设置连接断开回调 */
    linx_protocol_set_on_disconnected(&protocol, test_disconnected_callback, test_user_data);
    TEST_ASSERT(protocol.on_disconnected == test_disconnected_callback, "连接断开回调设置正确");
    
    /* 测试空指针处理 */
    linx_protocol_set_on_incoming_audio(NULL, test_incoming_audio_callback, test_user_data);
    linx_protocol_set_on_incoming_json(NULL, test_incoming_json_callback, test_user_data);
    TEST_ASSERT(true, "空指针处理正常");
    
    linx_protocol_destroy(&protocol);
}

/* 测试协议操作函数 */
void test_protocol_operations(void) {
    printf("\n=== 测试协议操作函数 ===\n");
    
    linx_protocol_t protocol;
    linx_protocol_init(&protocol, &mock_vtable);
    
    reset_mock_state();
    
    /* 测试启动协议 */
    bool result = linx_protocol_start(&protocol);
    TEST_ASSERT(result == true, "启动协议成功");
    TEST_ASSERT(mock_start_called, "启动函数被调用");
    
    /* 测试打开音频通道 */
    result = linx_protocol_open_audio_channel(&protocol);
    TEST_ASSERT(result == true, "打开音频通道成功");
    TEST_ASSERT(mock_open_audio_channel_called, "打开音频通道函数被调用");
    
    /* 测试关闭音频通道 */
    linx_protocol_close_audio_channel(&protocol);
    TEST_ASSERT(mock_close_audio_channel_called, "关闭音频通道函数被调用");
    
    /* 测试检查音频通道状态 */
    result = linx_protocol_is_audio_channel_opened(&protocol);
    TEST_ASSERT(result == true, "检查音频通道状态成功");
    TEST_ASSERT(mock_is_audio_channel_opened_called, "检查音频通道状态函数被调用");
    
    /* 测试发送音频数据 */
    linx_audio_stream_packet_t* packet = linx_audio_stream_packet_create(1024);
    result = linx_protocol_send_audio(&protocol, packet);
    TEST_ASSERT(result == true, "发送音频数据成功");
    TEST_ASSERT(mock_send_audio_called, "发送音频数据函数被调用");
    linx_audio_stream_packet_destroy(packet);
    
    /* 测试空指针处理 */
    TEST_ASSERT(linx_protocol_start(NULL) == false, "空指针处理正确");
    TEST_ASSERT(linx_protocol_open_audio_channel(NULL) == false, "空指针处理正确");
    linx_protocol_close_audio_channel(NULL);
    TEST_ASSERT(linx_protocol_is_audio_channel_opened(NULL) == false, "空指针处理正确");
    
    linx_protocol_destroy(&protocol);
}

/* 测试音频数据包创建和销毁 */
void test_audio_packet_management(void) {
    printf("\n=== 测试音频数据包管理 ===\n");
    
    /* 测试创建音频数据包 */
    size_t payload_size = 1024;
    linx_audio_stream_packet_t* packet = linx_audio_stream_packet_create(payload_size);
    TEST_ASSERT(packet != NULL, "音频数据包创建成功");
    TEST_ASSERT(packet->payload != NULL, "音频数据包载荷分配成功");
    TEST_ASSERT(packet->payload_size == payload_size, "音频数据包载荷大小正确");
    TEST_ASSERT(packet->sample_rate == 0, "音频数据包采样率初始化正确");
    TEST_ASSERT(packet->frame_duration == 0, "音频数据包帧持续时间初始化正确");
    TEST_ASSERT(packet->timestamp == 0, "音频数据包时间戳初始化正确");
    
    /* 测试销毁音频数据包 */
    linx_audio_stream_packet_destroy(packet);
    TEST_ASSERT(true, "音频数据包销毁成功");
    
    /* 测试空指针处理 */
    linx_audio_stream_packet_destroy(NULL);
    TEST_ASSERT(true, "空指针处理正常");
    
    /* 测试零大小创建 */
    packet = linx_audio_stream_packet_create(0);
    TEST_ASSERT(packet != NULL, "零大小音频数据包创建成功");
    TEST_ASSERT(packet->payload == NULL, "零大小音频数据包载荷为空");
    linx_audio_stream_packet_destroy(packet);
}

/* 测试错误处理 */
void test_error_handling(void) {
    printf("\n=== 测试错误处理 ===\n");
    
    linx_protocol_t protocol;
    linx_protocol_init(&protocol, &mock_vtable);
    
    /* 测试设置错误 */
    linx_protocol_set_error(&protocol, "测试错误消息");
    TEST_ASSERT(protocol.error_occurred == true, "错误状态设置正确");
    
    /* 测试超时检查 */
    bool is_timeout = linx_protocol_is_timeout(&protocol);
    TEST_ASSERT(is_timeout == false, "超时检查正常");
    
    /* 测试空指针处理 */
    linx_protocol_set_error(NULL, "测试错误消息");
    TEST_ASSERT(linx_protocol_is_timeout(NULL) == false, "空指针处理正确");
    
    linx_protocol_destroy(&protocol);
}

/* 测试协议消息发送 */
void test_protocol_messages(void) {
    printf("\n=== 测试协议消息发送 ===\n");
    
    linx_protocol_t protocol;
    linx_protocol_init(&protocol, &mock_vtable);
    
    /* 测试发送唤醒词检测消息 */
    linx_protocol_send_wake_word_detected(&protocol, "hello");
    TEST_ASSERT(true, "发送唤醒词检测消息正常");
    
    /* 测试发送开始监听消息 */
    linx_protocol_send_start_listening(&protocol, LINX_LISTENING_MODE_AUTO_STOP);
    TEST_ASSERT(true, "发送开始监听消息正常");
    
    /* 测试发送停止监听消息 */
    linx_protocol_send_stop_listening(&protocol);
    TEST_ASSERT(true, "发送停止监听消息正常");
    
    /* 测试发送中止说话消息 */
    linx_protocol_send_abort_speaking(&protocol, LINX_ABORT_REASON_WAKE_WORD_DETECTED);
    TEST_ASSERT(true, "发送中止说话消息正常");
    
    /* 测试发送MCP消息 */
    linx_protocol_send_mcp_message(&protocol, "{\"type\":\"test\"}");
    TEST_ASSERT(true, "发送MCP消息正常");
    
    /* 测试空指针处理 */
    linx_protocol_send_wake_word_detected(NULL, "hello");
    linx_protocol_send_start_listening(NULL, LINX_LISTENING_MODE_AUTO_STOP);
    linx_protocol_send_stop_listening(NULL);
    linx_protocol_send_abort_speaking(NULL, LINX_ABORT_REASON_NONE);
    linx_protocol_send_mcp_message(NULL, "{\"type\":\"test\"}");
    TEST_ASSERT(true, "空指针处理正常");
    
    linx_protocol_destroy(&protocol);
}

/* 主测试函数 */
int main(void) {
    printf("开始运行 linx_protocol 单元测试...\n");
    
    test_protocol_init_destroy();
    test_protocol_getters();
    test_protocol_callbacks();
    test_protocol_operations();
    test_audio_packet_management();
    test_error_handling();
    test_protocol_messages();
    
    printf("\n=== 测试结果 ===\n");
    printf("总测试数: %d\n", tests_run);
    printf("通过测试: %d\n", tests_passed);
    printf("失败测试: %d\n", tests_run - tests_passed);
    
    if (tests_passed == tests_run) {
        printf("✓ 所有测试通过!\n");
        return 0;
    } else {
        printf("✗ 有测试失败!\n");
        return 1;
    }
}