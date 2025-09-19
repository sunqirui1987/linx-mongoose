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
        return;
    }
    
    memset(protocol, 0, sizeof(linx_protocol_t));
    protocol->vtable = vtable;
    protocol->server_sample_rate = 24000;      // 默认采样率24kHz
    protocol->server_frame_duration = 60;      // 默认帧持续时间60ms
    protocol->error_occurred = false;
    protocol->session_id = NULL;
    protocol->last_incoming_time = get_current_time_ms();
}

void linx_protocol_destroy(linx_protocol_t* protocol) {
    if (!protocol) {
        return;
    }
    
    // 释放会话ID内存
    if (protocol->session_id) {
        protocol->session_id = NULL;
    }
    
    // 调用具体协议的销毁函数
    if (protocol->vtable && protocol->vtable->destroy) {
        protocol->vtable->destroy(protocol);
    }
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
        return false;
    }
    return protocol->vtable->start(protocol);
}

bool linx_protocol_send_audio(linx_protocol_t* protocol, linx_audio_stream_packet_t* packet) {
    if (!protocol || !protocol->vtable || !protocol->vtable->send_audio) {
        return false;
    }
    return protocol->vtable->send_audio(protocol, packet);
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
        return;
    }
    
    protocol->error_occurred = true;
    if (protocol->on_network_error && message) {
        protocol->on_network_error(message, protocol->user_data);
    }
}

bool linx_protocol_is_timeout(const linx_protocol_t* protocol) {
    if (!protocol) {
        return false;
    }
    
    uint64_t current_time = get_current_time_ms();
    return (current_time - protocol->last_incoming_time) > LINX_TIMEOUT_MS;
}

/* 音频数据包管理 */
linx_audio_stream_packet_t* linx_audio_stream_packet_create(size_t payload_size) {
    linx_audio_stream_packet_t* packet = malloc(sizeof(linx_audio_stream_packet_t));
    if (!packet) {
        return NULL;
    }
    
    memset(packet, 0, sizeof(linx_audio_stream_packet_t));
    
    // 如果需要载荷，分配内存
    if (payload_size > 0) {
        packet->payload = malloc(payload_size);
        if (!packet->payload) {
            free(packet);
            return NULL;
        }
        packet->payload_size = payload_size;
    }
    
    return packet;
}

void linx_audio_stream_packet_destroy(linx_audio_stream_packet_t* packet) {
    if (!packet) {
        return;
    }
    
    // 释放载荷内存
    if (packet->payload) {
        free(packet->payload);
    }
    free(packet);
}