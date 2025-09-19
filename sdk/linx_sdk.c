#include "linx_sdk.h"
#include "protocols/linx_protocol.h"
#include "protocols/linx_websocket.h"
#include "mcp/mcp_server.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* SDK 内部结构体 */
struct linx_sdk {
    linx_protocol_t* protocol;
    linx_callbacks_t callbacks;
    linx_config_t config;
    mcp_server_t* mcp_server;
    bool started;
    bool error_occurred;
    char* last_error_message;
};

/* 内部函数声明 */
static void internal_on_connected(void* user_data);
static void internal_on_disconnected(void* user_data);
static void internal_on_network_error(const char* message, void* user_data);
static void internal_on_audio_channel_opened(void* user_data);
static void internal_on_audio_channel_closed(void* user_data);
static void internal_on_incoming_audio(linx_audio_stream_packet_t* packet, void* user_data);
static void internal_on_incoming_json(const cJSON* root, void* user_data);
static void parse_json_message(linx_sdk_t* sdk, const cJSON* root);
static linx_protocol_t* create_protocol(linx_protocol_type_t type);
static void setup_protocol_callbacks(linx_sdk_t* sdk);

/* ========== 核心 API 函数实现 ========== */

linx_sdk_t* linx_sdk_create(const linx_config_t* config) {
    if (!config || !config->server_url) {
        return NULL;
    }

    linx_sdk_t* sdk = (linx_sdk_t*)calloc(1, sizeof(linx_sdk_t));
    if (!sdk) {
        return NULL;
    }

    // 复制配置
    sdk->config = *config;
    sdk->config.server_url = strdup(config->server_url);
    if (!sdk->config.server_url) {
        free(sdk);
        return NULL;
    }

    // 创建协议实例
    sdk->protocol = create_protocol(config->protocol_type);
    if (!sdk->protocol) {
        free(sdk->config.server_url);
        free(sdk);
        return NULL;
    }

    // 创建 MCP 服务器
    sdk->mcp_server = mcp_server_create("linx_sdk", "1.0.0");
    if (!sdk->mcp_server) {
        // 协议销毁逻辑需要根据具体实现添加
        free(sdk->config.server_url);
        free(sdk);
        return NULL;
    }

    // 设置协议回调
    setup_protocol_callbacks(sdk);

    return sdk;
}

void linx_sdk_destroy(linx_sdk_t* sdk) {
    if (!sdk) {
        return;
    }

    if (sdk->started) {
        linx_sdk_stop(sdk);
    }

    if (sdk->mcp_server) {
        mcp_server_destroy(sdk->mcp_server);
    }

    if (sdk->protocol) {
        // 协议销毁逻辑需要根据具体实现添加
        free(sdk->protocol);
    }

    if (sdk->config.server_url) {
        free(sdk->config.server_url);
    }

    if (sdk->last_error_message) {
        free(sdk->last_error_message);
    }

    free(sdk);
}

void linx_sdk_set_callbacks(linx_sdk_t* sdk, const linx_callbacks_t* callbacks) {
    if (!sdk || !callbacks) {
        return;
    }

    sdk->callbacks = *callbacks;
}

bool linx_sdk_start(linx_sdk_t* sdk) {
    if (!sdk || !sdk->protocol) {
        return false;
    }

    if (sdk->started) {
        return true;
    }

    bool result = linx_protocol_start(sdk->protocol);
    if (result) {
        sdk->started = true;
        sdk->error_occurred = false;
    }

    return result;
}

void linx_sdk_stop(linx_sdk_t* sdk) {
    if (!sdk || !sdk->started) {
        return;
    }

    // 关闭音频通道
    if (linx_protocol_is_audio_channel_opened(sdk->protocol)) {
        linx_protocol_close_audio_channel(sdk->protocol);
    }

    // 停止协议（需要根据具体实现添加停止方法）
    sdk->started = false;
}

/* ========== 音频通道管理 ========== */

bool linx_sdk_open_audio_channel(linx_sdk_t* sdk) {
    if (!sdk || !sdk->protocol || !sdk->started) {
        return false;
    }

    return linx_protocol_open_audio_channel(sdk->protocol);
}

void linx_sdk_close_audio_channel(linx_sdk_t* sdk) {
    if (!sdk || !sdk->protocol) {
        return;
    }

    linx_protocol_close_audio_channel(sdk->protocol);
}

bool linx_sdk_is_audio_channel_opened(const linx_sdk_t* sdk) {
    if (!sdk || !sdk->protocol) {
        return false;
    }

    return linx_protocol_is_audio_channel_opened(sdk->protocol);
}

bool linx_sdk_send_audio(linx_sdk_t* sdk, const linx_audio_packet_t* packet) {
    if (!sdk || !sdk->protocol || !packet) {
        return false;
    }

    // 转换为协议层的音频包格式
    linx_audio_stream_packet_t protocol_packet;
    protocol_packet.data = packet->data;
    protocol_packet.size = packet->size;
    protocol_packet.timestamp = packet->timestamp;

    return linx_protocol_send_audio(sdk->protocol, &protocol_packet);
}

/* ========== 消息发送 ========== */

bool linx_sdk_send_wake_word_detected(linx_sdk_t* sdk, const char* wake_word) {
    if (!sdk || !sdk->protocol || !wake_word) {
        return false;
    }

    return linx_protocol_send_wake_word_detected(sdk->protocol, wake_word);
}

bool linx_sdk_send_start_listening(linx_sdk_t* sdk, linx_listening_mode_t mode) {
    if (!sdk || !sdk->protocol) {
        return false;
    }

    // 转换监听模式
    linx_listening_mode_t protocol_mode;
    switch (mode) {
        case LINX_LISTENING_MODE_MANUAL_STOP:
            protocol_mode = LINX_LISTENING_MODE_MANUAL_STOP;
            break;
        case LINX_LISTENING_MODE_AUTO_STOP:
            protocol_mode = LINX_LISTENING_MODE_AUTO_STOP;
            break;
        case LINX_LISTENING_MODE_REALTIME:
            protocol_mode = LINX_LISTENING_MODE_REALTIME;
            break;
        default:
            return false;
    }

    return linx_protocol_send_start_listening(sdk->protocol, protocol_mode);
}

bool linx_sdk_send_stop_listening(linx_sdk_t* sdk) {
    if (!sdk || !sdk->protocol) {
        return false;
    }

    return linx_protocol_send_stop_listening(sdk->protocol);
}

bool linx_sdk_send_abort_speaking(linx_sdk_t* sdk, linx_abort_reason_t reason) {
    if (!sdk || !sdk->protocol) {
        return false;
    }

    // 转换中止原因
    linx_abort_reason_t protocol_reason;
    switch (reason) {
        case LINX_ABORT_REASON_USER_REQUEST:
            protocol_reason = LINX_ABORT_REASON_USER_REQUEST;
            break;
        case LINX_ABORT_REASON_WAKE_WORD_DETECTED:
            protocol_reason = LINX_ABORT_REASON_WAKE_WORD_DETECTED;
            break;
        default:
            return false;
    }

    return linx_protocol_send_abort_speaking(sdk->protocol, protocol_reason);
}

bool linx_sdk_send_mcp_message(linx_sdk_t* sdk, const char* payload) {
    if (!sdk || !sdk->protocol || !payload) {
        return false;
    }

    return linx_protocol_send_mcp_message(sdk->protocol, payload);
}

bool linx_sdk_send_text_message(linx_sdk_t* sdk, const char* message) {
    if (!sdk || !sdk->protocol || !message) {
        return false;
    }

    return linx_protocol_send_text(sdk->protocol, message);
}

/* ========== 状态查询 ========== */

int linx_sdk_get_server_sample_rate(const linx_sdk_t* sdk) {
    if (!sdk || !sdk->protocol) {
        return 0;
    }

    return linx_protocol_get_server_sample_rate(sdk->protocol);
}

int linx_sdk_get_server_frame_duration(const linx_sdk_t* sdk) {
    if (!sdk || !sdk->protocol) {
        return 0;
    }

    return linx_protocol_get_server_frame_duration(sdk->protocol);
}

const char* linx_sdk_get_session_id(const linx_sdk_t* sdk) {
    if (!sdk || !sdk->protocol) {
        return NULL;
    }

    return linx_protocol_get_session_id(sdk->protocol);
}

bool linx_sdk_has_error(const linx_sdk_t* sdk) {
    if (!sdk) {
        return true;
    }

    return sdk->error_occurred;
}

/* ========== 便利函数 ========== */

linx_config_t linx_sdk_create_default_config(linx_protocol_type_t protocol_type, const char* server_url) {
    linx_config_t config = {0};
    config.protocol_type = protocol_type;
    config.server_url = (char*)server_url; // 注意：这里只是引用，实际使用时需要复制
    config.server_sample_rate = 16000;
    config.server_frame_duration = 20;
    config.user_data = NULL;
    return config;
}

linx_callbacks_t linx_sdk_create_empty_callbacks(void) {
    linx_callbacks_t callbacks = {0};
    return callbacks;
}

/* ========== 内部函数实现 ========== */

static linx_protocol_t* create_protocol(linx_protocol_type_t type) {
    switch (type) {
        case LINX_PROTOCOL_WEBSOCKET:
            return linx_websocket_create();
        case LINX_PROTOCOL_MQTT:
            // MQTT 协议实现需要添加
            return NULL;
        default:
            return NULL;
    }
}

static void setup_protocol_callbacks(linx_sdk_t* sdk) {
    if (!sdk || !sdk->protocol) {
        return;
    }

    linx_protocol_set_on_connected(sdk->protocol, internal_on_connected, sdk);
    linx_protocol_set_on_disconnected(sdk->protocol, internal_on_disconnected, sdk);
    linx_protocol_set_on_network_error(sdk->protocol, internal_on_network_error, sdk);
    linx_protocol_set_on_audio_channel_opened(sdk->protocol, internal_on_audio_channel_opened, sdk);
    linx_protocol_set_on_audio_channel_closed(sdk->protocol, internal_on_audio_channel_closed, sdk);
    linx_protocol_set_on_incoming_audio(sdk->protocol, internal_on_incoming_audio, sdk);
    linx_protocol_set_on_incoming_json(sdk->protocol, internal_on_incoming_json, sdk);
}

static void internal_on_connected(void* user_data) {
    linx_sdk_t* sdk = (linx_sdk_t*)user_data;
    if (sdk && sdk->callbacks.on_connected) {
        sdk->callbacks.on_connected(sdk->config.user_data);
    }
}

static void internal_on_disconnected(void* user_data) {
    linx_sdk_t* sdk = (linx_sdk_t*)user_data;
    if (sdk && sdk->callbacks.on_disconnected) {
        sdk->callbacks.on_disconnected(sdk->config.user_data);
    }
}

static void internal_on_network_error(const char* message, void* user_data) {
    linx_sdk_t* sdk = (linx_sdk_t*)user_data;
    if (sdk) {
        sdk->error_occurred = true;
        if (sdk->last_error_message) {
            free(sdk->last_error_message);
        }
        sdk->last_error_message = strdup(message);

        if (sdk->callbacks.on_network_error) {
            sdk->callbacks.on_network_error(message, sdk->config.user_data);
        }
    }
}

static void internal_on_audio_channel_opened(void* user_data) {
    linx_sdk_t* sdk = (linx_sdk_t*)user_data;
    if (sdk && sdk->callbacks.on_audio_channel_opened) {
        sdk->callbacks.on_audio_channel_opened(sdk->config.user_data);
    }
}

static void internal_on_audio_channel_closed(void* user_data) {
    linx_sdk_t* sdk = (linx_sdk_t*)user_data;
    if (sdk && sdk->callbacks.on_audio_channel_closed) {
        sdk->callbacks.on_audio_channel_closed(sdk->config.user_data);
    }
}

static void internal_on_incoming_audio(linx_audio_stream_packet_t* packet, void* user_data) {
    linx_sdk_t* sdk = (linx_sdk_t*)user_data;
    if (sdk && sdk->callbacks.on_incoming_audio && packet) {
        // 转换为 SDK 层的音频包格式
        linx_audio_packet_t sdk_packet;
        sdk_packet.data = packet->data;
        sdk_packet.size = packet->size;
        sdk_packet.timestamp = packet->timestamp;

        sdk->callbacks.on_incoming_audio(&sdk_packet, sdk->config.user_data);
    }
}

static void internal_on_incoming_json(const cJSON* root, void* user_data) {
    linx_sdk_t* sdk = (linx_sdk_t*)user_data;
    if (sdk && root) {
        parse_json_message(sdk, root);
    }
}

static void parse_json_message(linx_sdk_t* sdk, const cJSON* root) {
    if (!sdk || !root) {
        return;
    }

    const cJSON* type = cJSON_GetObjectItem(root, "type");
    if (!cJSON_IsString(type)) {
        return;
    }

    const char* type_str = type->valuestring;

    if (strcmp(type_str, "tts") == 0) {
        const cJSON* state = cJSON_GetObjectItem(root, "state");
        if (cJSON_IsString(state)) {
            const char* state_str = state->valuestring;
            if (strcmp(state_str, "start") == 0 && sdk->callbacks.on_tts_start) {
                sdk->callbacks.on_tts_start(sdk->config.user_data);
            } else if (strcmp(state_str, "stop") == 0 && sdk->callbacks.on_tts_stop) {
                sdk->callbacks.on_tts_stop(sdk->config.user_data);
            } else if (strcmp(state_str, "sentence_start") == 0) {
                const cJSON* text = cJSON_GetObjectItem(root, "text");
                if (cJSON_IsString(text) && sdk->callbacks.on_tts_sentence) {
                    sdk->callbacks.on_tts_sentence(text->valuestring, sdk->config.user_data);
                }
            }
        }
    } else if (strcmp(type_str, "stt") == 0) {
        const cJSON* text = cJSON_GetObjectItem(root, "text");
        if (cJSON_IsString(text) && sdk->callbacks.on_stt_result) {
            sdk->callbacks.on_stt_result(text->valuestring, sdk->config.user_data);
        }
    } else if (strcmp(type_str, "llm") == 0) {
        const cJSON* emotion = cJSON_GetObjectItem(root, "emotion");
        if (cJSON_IsString(emotion) && sdk->callbacks.on_llm_emotion) {
            sdk->callbacks.on_llm_emotion(emotion->valuestring, sdk->config.user_data);
        }
    } else if (strcmp(type_str, "mcp") == 0) {
        const cJSON* payload = cJSON_GetObjectItem(root, "payload");
        if (cJSON_IsObject(payload)) {
            // 处理 MCP 消息
            if (sdk->mcp_server) {
                mcp_server_parse_json_message(sdk->mcp_server, payload);
            }
            if (sdk->callbacks.on_mcp_message) {
                sdk->callbacks.on_mcp_message(payload, sdk->config.user_data);
            }
        }
    } else if (strcmp(type_str, "system") == 0) {
        const cJSON* command = cJSON_GetObjectItem(root, "command");
        if (cJSON_IsString(command) && sdk->callbacks.on_system_command) {
            sdk->callbacks.on_system_command(command->valuestring, sdk->config.user_data);
        }
    } else if (strcmp(type_str, "alert") == 0) {
        const cJSON* status = cJSON_GetObjectItem(root, "status");
        const cJSON* message = cJSON_GetObjectItem(root, "message");
        const cJSON* emotion = cJSON_GetObjectItem(root, "emotion");
        if (cJSON_IsString(status) && cJSON_IsString(message) && 
            cJSON_IsString(emotion) && sdk->callbacks.on_alert) {
            sdk->callbacks.on_alert(status->valuestring, message->valuestring, 
                                   emotion->valuestring, sdk->config.user_data);
        }
    } else if (strcmp(type_str, "custom") == 0) {
        const cJSON* payload = cJSON_GetObjectItem(root, "payload");
        if (cJSON_IsObject(payload) && sdk->callbacks.on_custom_message) {
            sdk->callbacks.on_custom_message(payload, sdk->config.user_data);
        }
    }
}