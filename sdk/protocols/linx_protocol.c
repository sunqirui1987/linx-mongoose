#include "linx_protocol.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define LINX_TIMEOUT_MS 120000  /* 120秒超时 */

/* 获取当前时间戳（毫秒） */
static uint64_t get_current_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

/* 协议管理函数 */
void linx_protocol_init(linx_protocol_t* protocol, const linx_protocol_vtable_t* vtable) {
    if (!protocol || !vtable) {
        LOG_ERROR("协议初始化失败: 无效的参数 (protocol=%p, vtable=%p)", protocol, vtable);
        return;
    }
    
    LOG_DEBUG("开始初始化协议实例");
    memset(protocol, 0, sizeof(linx_protocol_t));
    protocol->vtable = vtable;
    protocol->server_sample_rate = 24000;      // 默认采样率24kHz
    protocol->server_frame_duration = 60;      // 默认帧持续时间60ms
    protocol->error_occurred = false;
    protocol->session_id = NULL;
    protocol->last_incoming_time = get_current_time_ms();
    
    LOG_INFO("协议初始化成功 - 采样率: %d Hz, 帧持续时间: %d ms", 
             protocol->server_sample_rate, protocol->server_frame_duration);
}

void linx_protocol_destroy(linx_protocol_t* protocol) {
    if (!protocol) {
        LOG_WARN("尝试销毁空的协议实例");
        return;
    }
    
    LOG_DEBUG("开始销毁协议实例");
    
    // 释放会话ID内存
    if (protocol->session_id) {
        LOG_DEBUG("释放会话ID: %s", protocol->session_id);
        protocol->session_id = NULL;
    }
    
    // 调用具体协议的销毁函数
    if (protocol->vtable && protocol->vtable->destroy) {
        LOG_DEBUG("调用协议特定的销毁函数");
        protocol->vtable->destroy(protocol);
    }
    
    LOG_INFO("协议实例销毁完成");
}

/* 获取器函数 */
int linx_protocol_get_server_sample_rate(const linx_protocol_t* protocol) {
    return protocol ? protocol->server_sample_rate : 0;
}

int linx_protocol_get_server_frame_duration(const linx_protocol_t* protocol) {
    return protocol ? protocol->server_frame_duration : 0;
}

const char* linx_protocol_get_session_id(const linx_protocol_t* protocol) {
    return protocol ? protocol->session_id : NULL;
}

/* 回调函数注册 */
void linx_protocol_set_on_incoming_audio(linx_protocol_t* protocol, 
                                         linx_on_incoming_audio_cb_t callback, 
                                         void* user_data) {
    if (protocol) {
        protocol->on_incoming_audio = callback;
        protocol->user_data = user_data;
    }
}

void linx_protocol_set_on_incoming_json(linx_protocol_t* protocol, 
                                        linx_on_incoming_json_cb_t callback, 
                                        void* user_data) {
    if (protocol) {
        protocol->on_incoming_json = callback;
        protocol->user_data = user_data;
    }
}



void linx_protocol_set_on_network_error(linx_protocol_t* protocol, 
                                        linx_on_network_error_cb_t callback, 
                                        void* user_data) {
    if (protocol) {
        protocol->on_network_error = callback;
        protocol->user_data = user_data;
    }
}

void linx_protocol_set_on_connected(linx_protocol_t* protocol, 
                                    linx_on_connected_cb_t callback, 
                                    void* user_data) {
    if (protocol) {
        protocol->on_connected = callback;
        protocol->user_data = user_data;
    }
}

void linx_protocol_set_on_disconnected(linx_protocol_t* protocol, 
                                       linx_on_disconnected_cb_t callback, 
                                       void* user_data) {
    if (protocol) {
        protocol->on_disconnected = callback;
        protocol->user_data = user_data;
    }
}

/* 协议操作函数 */
bool linx_protocol_start(linx_protocol_t* protocol) {
    if (!protocol || !protocol->vtable || !protocol->vtable->start) {
        LOG_ERROR("协议启动失败: 无效的协议实例或vtable");
        return false;
    }
    
    LOG_INFO("开始启动协议");
    bool result = protocol->vtable->start(protocol);
    
    if (result) {
        LOG_INFO("协议启动成功");
    } else {
        LOG_ERROR("协议启动失败");
    }
    
    return result;
}

bool linx_protocol_send_audio(linx_protocol_t* protocol, linx_audio_stream_packet_t* packet) {
    if (!protocol || !protocol->vtable || !protocol->vtable->send_audio) {
        LOG_ERROR("音频发送失败: 无效的协议实例或vtable");
        return false;
    }
    
    if (!packet) {
        LOG_ERROR("音频发送失败: 音频包为空");
        return false;
    }
    
    LOG_DEBUG("发送音频包 - 采样率: %d, 帧持续时间: %d, 载荷大小: %zu", 
              packet->sample_rate, packet->frame_duration, packet->payload_size);
    
    bool result = protocol->vtable->send_audio(protocol, packet);
    
    if (!result) {
        LOG_WARN("音频包发送失败");
    }
    
    return result;
}

/* 高级消息发送函数 */
void linx_protocol_send_wake_word_detected(linx_protocol_t* protocol, const char* wake_word) {
    if (!protocol || !protocol->vtable || !protocol->vtable->send_text || !wake_word) {
        return;
    }
    
    char message[512];
    snprintf(message, sizeof(message), 
             "{\"session_id\":\"%s\",\"type\":\"listen\",\"state\":\"detect\",\"text\":\"%s\"}", 
             protocol->session_id ? protocol->session_id : "", wake_word);
    
    protocol->vtable->send_text(protocol, message);
}

void linx_protocol_send_start_listening(linx_protocol_t* protocol, linx_listening_mode_t mode) {
    if (!protocol || !protocol->vtable || !protocol->vtable->send_text) {
        return;
    }
    
    const char* mode_str;
    switch (mode) {
        case LINX_LISTENING_MODE_AUTO_STOP:
            mode_str = "auto";
            break;
        case LINX_LISTENING_MODE_MANUAL_STOP:
            mode_str = "manual";
            break;
        case LINX_LISTENING_MODE_REALTIME:
            mode_str = "realtime";
            break;
        default:
            mode_str = "auto";
            break;
    }
    
    char message[256];
    snprintf(message, sizeof(message), 
             "{\"session_id\":\"%s\",\"type\":\"listen\",\"state\":\"start\",\"mode\":\"%s\"}", 
             protocol->session_id ? protocol->session_id : "", mode_str);
    
    protocol->vtable->send_text(protocol, message);
}

void linx_protocol_send_stop_listening(linx_protocol_t* protocol) {
    if (!protocol || !protocol->vtable || !protocol->vtable->send_text) {
        return;
    }
    
    char message[256];
    snprintf(message, sizeof(message), 
             "{\"session_id\":\"%s\",\"type\":\"listen\",\"state\":\"stop\"}", 
             protocol->session_id ? protocol->session_id : "");
    
    protocol->vtable->send_text(protocol, message);
}

void linx_protocol_send_abort_speaking(linx_protocol_t* protocol, linx_abort_reason_t reason) {
    if (!protocol || !protocol->vtable || !protocol->vtable->send_text) {
        return;
    }
    
    char message[256];
    if (reason == LINX_ABORT_REASON_WAKE_WORD_DETECTED) {
        snprintf(message, sizeof(message), 
                 "{\"session_id\":\"%s\",\"type\":\"abort\",\"reason\":\"wake_word_detected\"}", 
                 protocol->session_id ? protocol->session_id : "");
    } else {
        snprintf(message, sizeof(message), 
                 "{\"session_id\":\"%s\",\"type\":\"abort\"}", 
                 protocol->session_id ? protocol->session_id : "");
    }
    
    protocol->vtable->send_text(protocol, message);
}

void linx_protocol_send_mcp_message(linx_protocol_t* protocol, const char* message) {
    if (!protocol || !protocol->vtable || !protocol->vtable->send_text || !message) {
        return;
    }
    
    char mcp_message[1024];
    snprintf(mcp_message, sizeof(mcp_message), 
             "{\"session_id\":\"%s\",\"type\":\"mcp\",\"payload\":\"%s\"}", 
             protocol->session_id ? protocol->session_id : "", message);
    
    protocol->vtable->send_text(protocol, mcp_message);
}

/* 工具函数 */
void linx_protocol_set_error(linx_protocol_t* protocol, const char* message) {
    if (!protocol) {
        LOG_ERROR("设置协议错误失败: 协议实例为空");
        return;
    }
    
    LOG_ERROR("协议错误: %s", message ? message : "未知错误");
    protocol->error_occurred = true;
    
    if (protocol->on_network_error && message) {
        LOG_DEBUG("调用网络错误回调函数");
        protocol->on_network_error(message, protocol->user_data);
    }
}

bool linx_protocol_is_timeout(const linx_protocol_t* protocol) {
    if (!protocol) {
        LOG_WARN("检查超时失败: 协议实例为空");
        return false;
    }
    
    uint64_t current_time = get_current_time_ms();
    uint64_t elapsed = current_time - protocol->last_incoming_time;
    bool is_timeout = elapsed > LINX_TIMEOUT_MS;
    
    if (is_timeout) {
        LOG_WARN("协议超时检测: 已超时 %llu ms (阈值: %d ms)", elapsed, LINX_TIMEOUT_MS);
    } else {
        LOG_DEBUG("协议超时检测: 正常 (%llu ms)", elapsed);
    }
    
    return is_timeout;
}

/* 音频数据包管理 */
linx_audio_stream_packet_t* linx_audio_stream_packet_create(size_t payload_size) {
    LOG_DEBUG("创建音频数据包 - 载荷大小: %zu", payload_size);
    
    linx_audio_stream_packet_t* packet = malloc(sizeof(linx_audio_stream_packet_t));
    if (!packet) {
        LOG_ERROR("音频数据包创建失败: 内存分配失败");
        return NULL;
    }
    
    memset(packet, 0, sizeof(linx_audio_stream_packet_t));
    
    // 如果需要载荷，分配内存
    if (payload_size > 0) {
        packet->payload = malloc(payload_size);
        if (!packet->payload) {
            LOG_ERROR("音频数据包载荷分配失败: 大小 %zu", payload_size);
            free(packet);
            return NULL;
        }
        packet->payload_size = payload_size;
    }
    
    LOG_DEBUG("音频数据包创建成功");
    return packet;
}

void linx_audio_stream_packet_destroy(linx_audio_stream_packet_t* packet) {
    if (!packet) {
        LOG_DEBUG("尝试销毁空的音频数据包");
        return;
    }
    
    LOG_DEBUG("销毁音频数据包 - 载荷大小: %zu", packet->payload_size);
    
    // 释放载荷内存
    if (packet->payload) {
        free(packet->payload);
    }
    free(packet);
    
    LOG_DEBUG("音频数据包销毁完成");
}