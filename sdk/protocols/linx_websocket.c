#include "linx_websocket.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <mongoose.h>
#include "../cjson/cJSON.h"

/* Protocol vtable for WebSocket implementation */
static const linx_protocol_vtable_t linx_websocket_vtable = {
    .start = linx_websocket_start,
    .send_audio = linx_websocket_send_audio,
    .send_text = linx_websocket_send_text,
    .destroy = linx_websocket_destroy
};

/* WebSocket protocol creation and destruction */
linx_websocket_protocol_t* linx_websocket_protocol_create(void) {
    LOG_DEBUG("开始创建WebSocket协议实例");
    
    linx_websocket_protocol_t* ws_protocol = malloc(sizeof(linx_websocket_protocol_t));
    if (!ws_protocol) {
        LOG_ERROR("WebSocket协议创建失败: 内存分配失败");
        return NULL;
    }
    
    memset(ws_protocol, 0, sizeof(linx_websocket_protocol_t));
    
    /* Initialize base protocol */
    linx_protocol_init(&ws_protocol->base, &linx_websocket_vtable);
    
    /* Initialize mongoose manager */
    mg_mgr_init(&ws_protocol->mgr);
    
    /* Set default values */
    ws_protocol->connected = false;
    ws_protocol->audio_channel_opened = false;
    ws_protocol->version = 1;
    ws_protocol->server_hello_received = false;
    ws_protocol->running = false;
    ws_protocol->should_stop = false;
    ws_protocol->conn = NULL;
    ws_protocol->auth_token = NULL;
    ws_protocol->device_id = NULL;
    ws_protocol->client_id = NULL;
    
    LOG_INFO("WebSocket协议实例创建成功 - 版本: %d", ws_protocol->version);
    return ws_protocol;
}

void linx_websocket_protocol_destroy(linx_websocket_protocol_t* ws_protocol) {
    if (!ws_protocol) {
        LOG_WARN("尝试销毁空的WebSocket协议实例");
        return;
    }
    
    LOG_DEBUG("开始销毁WebSocket协议实例");
    
    /* Stop the protocol if running */
    if (ws_protocol->running) {
        LOG_DEBUG("停止运行中的WebSocket协议");
        linx_websocket_stop(ws_protocol);
    }
    
    /* Clean up connection */
    if (ws_protocol->conn) {
        LOG_DEBUG("清理WebSocket连接");
        ws_protocol->conn->is_closing = 1;
        ws_protocol->conn = NULL;
    }
    
    /* Clean up mongoose manager */
    LOG_DEBUG("清理Mongoose管理器");
    mg_mgr_free(&ws_protocol->mgr);
    
    /* Free allocated strings */
    if (ws_protocol->server_url) {
        LOG_DEBUG("释放服务器URL: %s", ws_protocol->server_url);
        free(ws_protocol->server_url);
    }
    if (ws_protocol->server_host) {
        LOG_DEBUG("释放服务器主机: %s", ws_protocol->server_host);
        free(ws_protocol->server_host);
    }
    if (ws_protocol->server_path) {
        LOG_DEBUG("释放服务器路径: %s", ws_protocol->server_path);
        free(ws_protocol->server_path);
    }
    if (ws_protocol->auth_token) {
        LOG_DEBUG("释放认证令牌");
        free(ws_protocol->auth_token);
    }
    if (ws_protocol->device_id) {
        LOG_DEBUG("释放设备ID: %s", ws_protocol->device_id);
        free(ws_protocol->device_id);
    }
    if (ws_protocol->client_id) {
        LOG_DEBUG("释放客户端ID: %s", ws_protocol->client_id);
        free(ws_protocol->client_id);
    }
    
    /* Clean up base protocol resources directly (avoid recursive call) */
    if (ws_protocol->base.session_id) {
        LOG_DEBUG("释放会话ID: %s", ws_protocol->base.session_id);
        free(ws_protocol->base.session_id);
        ws_protocol->base.session_id = NULL;
    }
    
    free(ws_protocol);
    LOG_INFO("WebSocket协议实例销毁完成");
}

/* Configuration functions */
bool linx_websocket_protocol_set_server_url(linx_websocket_protocol_t* ws_protocol, const char* url) {
    if (!ws_protocol || !url) {
        LOG_ERROR("设置服务器URL失败: 无效参数 (protocol=%p, url=%p)", ws_protocol, url);
        return false;
    }
    
    LOG_DEBUG("设置服务器URL: %s", url);
    
    if (ws_protocol->server_url) {
        free(ws_protocol->server_url);
    }
    
    ws_protocol->server_url = strdup(url);
    bool success = ws_protocol->server_url != NULL;
    
    if (success) {
        LOG_INFO("服务器URL设置成功: %s", url);
    } else {
        LOG_ERROR("服务器URL设置失败: 内存分配失败");
    }
    
    return success;
}

bool linx_websocket_protocol_set_server(linx_websocket_protocol_t* ws_protocol, 
                                        const char* host, 
                                        int port, 
                                        const char* path) {
    if (!ws_protocol || !host || !path) {
        LOG_ERROR("设置服务器配置失败: 无效参数 (protocol=%p, host=%p, path=%p)", 
                  ws_protocol, host, path);
        return false;
    }
    
    LOG_DEBUG("设置服务器配置 - 主机: %s, 端口: %d, 路径: %s", host, port, path);
    
    /* Free existing values */
    if (ws_protocol->server_host) {
        free(ws_protocol->server_host);
    }
    if (ws_protocol->server_path) {
        free(ws_protocol->server_path);
    }
    if (ws_protocol->server_url) {
        free(ws_protocol->server_url);
    }
    
    /* Set new values */
    ws_protocol->server_host = strdup(host);
    ws_protocol->server_port = port;
    ws_protocol->server_path = strdup(path);
    
    /* Construct URL */
    char url[512];
    snprintf(url, sizeof(url), "ws://%s:%d%s", host, port, path);
    ws_protocol->server_url = strdup(url);
    
    bool success = ws_protocol->server_host && ws_protocol->server_path && ws_protocol->server_url;
    
    if (success) {
        LOG_INFO("服务器配置设置成功 - URL: %s", url);
    } else {
        LOG_ERROR("服务器配置设置失败: 内存分配失败");
    }
    
    return success;
}

bool linx_websocket_protocol_set_auth_token(linx_websocket_protocol_t* ws_protocol, const char* token) {
    if (!ws_protocol || !token) {
        LOG_ERROR("设置认证令牌失败: 无效参数");
        return false;
    }
    
    LOG_DEBUG("设置认证令牌 (长度: %zu)", strlen(token));
    
    if (ws_protocol->auth_token) {
        free(ws_protocol->auth_token);
    }
    
    ws_protocol->auth_token = strdup(token);
    bool success = ws_protocol->auth_token != NULL;
    
    if (success) {
        LOG_INFO("认证令牌设置成功");
    } else {
        LOG_ERROR("认证令牌设置失败: 内存分配失败");
    }
    
    return success;
}

bool linx_websocket_protocol_set_device_id(linx_websocket_protocol_t* ws_protocol, const char* device_id) {
    if (!ws_protocol || !device_id) {
        LOG_ERROR("设置设备ID失败: 无效参数");
        return false;
    }
    
    LOG_DEBUG("设置设备ID: %s", device_id);
    
    if (ws_protocol->device_id) {
        free(ws_protocol->device_id);
    }
    
    ws_protocol->device_id = strdup(device_id);
    bool success = ws_protocol->device_id != NULL;
    
    if (success) {
        LOG_INFO("设备ID设置成功: %s", device_id);
    } else {
        LOG_ERROR("设备ID设置失败: 内存分配失败");
    }
    
    return success;
}

bool linx_websocket_protocol_set_client_id(linx_websocket_protocol_t* ws_protocol, const char* client_id) {
    if (!ws_protocol || !client_id) {
        LOG_ERROR("设置客户端ID失败: 无效参数");
        return false;
    }
    
    LOG_DEBUG("设置客户端ID: %s", client_id);
    
    if (ws_protocol->client_id) {
        free(ws_protocol->client_id);
    }
    
    ws_protocol->client_id = strdup(client_id);
    bool success = ws_protocol->client_id != NULL;
    
    if (success) {
        LOG_INFO("客户端ID设置成功: %s", client_id);
    } else {
        LOG_ERROR("客户端ID设置失败: 内存分配失败");
    }
    
    return success;
}

/* Event handler for mongoose WebSocket events */
void linx_websocket_event_handler(struct mg_connection* conn, int ev, void* ev_data) {
    linx_websocket_protocol_t* ws_protocol = (linx_websocket_protocol_t*)conn->fn_data;
    
    if (!ws_protocol) {
        LOG_ERROR("WebSocket事件处理失败: 协议实例为空");
        return;
    }
    
    switch (ev) {
        case MG_EV_CONNECT: {
            /* Connection established, WebSocket upgrade will happen automatically */
            LOG_DEBUG("WebSocket连接建立，等待升级");
            break;
        }
        
        case MG_EV_WS_OPEN: {
            /* WebSocket connection opened */
            LOG_INFO("WebSocket连接已打开");
            ws_protocol->connected = true;
            if (ws_protocol->base.on_connected) {
                LOG_DEBUG("调用连接成功回调函数");
                ws_protocol->base.on_connected(ws_protocol->base.user_data);
            }
            
            /* Send hello message */
            char* hello_msg = linx_websocket_get_hello_message(ws_protocol);
            if (hello_msg) {
                LOG_DEBUG("发送hello消息: %s", hello_msg);
                mg_ws_send(conn, hello_msg, strlen(hello_msg), WEBSOCKET_OP_TEXT);
                free(hello_msg);
            } else {
                LOG_WARN("无法生成hello消息");
            }
            break;
        }
        
        case MG_EV_WS_MSG: {
            /* WebSocket message received */
            struct mg_ws_message* wm = (struct mg_ws_message*)ev_data;
            
            if (wm->flags & WEBSOCKET_OP_TEXT) {
                /* Text message - parse as JSON */
                LOG_DEBUG("收到文本消息 (长度: %zu)", wm->data.len);
                
                cJSON* json = cJSON_ParseWithLength((const char*)wm->data.buf, wm->data.len);
                if (!json) {
                    LOG_ERROR("JSON解析失败");
                    return;
                }

                cJSON* type = cJSON_GetObjectItem(json, "type");
                if (!cJSON_IsString(type) || !type->valuestring) {
                    LOG_ERROR("消息类型无效或缺失");
                    cJSON_Delete(json);
                    return;
                }
                
                LOG_INFO("收到消息类型: %s", type->valuestring);
                
                /* Handle different message types */
                if (strcmp(type->valuestring, "hello") == 0) {
                    /* Server hello message - handle internally */
                    LOG_DEBUG("处理服务器hello消息");
                    char* json_string = cJSON_Print(json);
                    if (json_string) {
                        linx_websocket_parse_server_hello(ws_protocol, json_string);
                        LOG_INFO("服务器hello消息处理成功");
                        free(json_string);
                    } else {
                        LOG_ERROR("hello消息序列化失败");
                    }
                } 

                /* Other message types - call user callback */
                if (ws_protocol->base.on_incoming_json) {
                    LOG_DEBUG("调用用户JSON回调函数，消息类型: %s", type->valuestring);
                    ws_protocol->base.on_incoming_json(json, ws_protocol->base.user_data);
                } else {
                    LOG_DEBUG("未注册用户JSON回调函数");
                }

                cJSON_Delete(json);
            } else if (wm->flags & WEBSOCKET_OP_BINARY) {
                /* Binary message - parse as audio data based on protocol version */
                LOG_DEBUG("收到二进制消息 (长度: %zu, 协议版本: %d)", wm->data.len, ws_protocol->version);
                
                if (ws_protocol->base.on_incoming_audio) {
                    if (ws_protocol->version == 2) {
                        /* Use binary protocol v2 */
                        if (wm->data.len >= sizeof(linx_binary_protocol2_t)) {
                            linx_binary_protocol2_t* bp2 = (linx_binary_protocol2_t*)wm->data.buf;
                            uint16_t version = ntohs(bp2->version);
                            uint16_t type = ntohs(bp2->type);
                            uint32_t timestamp = ntohl(bp2->timestamp);
                            uint32_t payload_size = ntohl(bp2->payload_size);
                            
                            LOG_DEBUG("协议v2解析 - 版本: %d, 类型: %d, 时间戳: %u, 载荷大小: %u", 
                                     version, type, timestamp, payload_size);
                            
                            if (type == 0 && payload_size > 0) { /* Audio data */
                                linx_audio_stream_packet_t* packet = linx_audio_stream_packet_create(payload_size);
                                if (packet) {
                                    packet->sample_rate = ws_protocol->base.server_sample_rate;
                                    packet->frame_duration = ws_protocol->base.server_frame_duration;
                                    packet->timestamp = timestamp;
                                    memcpy(packet->payload, bp2->payload, payload_size);
                                    
                                    LOG_DEBUG("调用音频回调函数 (协议v2)");
                                    ws_protocol->base.on_incoming_audio(packet, ws_protocol->base.user_data);
                                    linx_audio_stream_packet_destroy(packet);
                                } else {
                                    LOG_ERROR("音频包创建失败 (协议v2)");
                                }
                            }
                        } else {
                            LOG_WARN("协议v2消息长度不足: %zu < %zu", wm->data.len, sizeof(linx_binary_protocol2_t));
                        }
                    } else if (ws_protocol->version == 3) {
                        /* Use binary protocol v3 */
                        if (wm->data.len >= sizeof(linx_binary_protocol3_t)) {
                            linx_binary_protocol3_t* bp3 = (linx_binary_protocol3_t*)wm->data.buf;
                            uint8_t type = bp3->type;
                            uint16_t payload_size = ntohs(bp3->payload_size);
                            
                            LOG_DEBUG("协议v3解析 - 类型: %d, 载荷大小: %d", type, payload_size);
                            
                            if (type == 0 && payload_size > 0) { /* Audio data */
                                linx_audio_stream_packet_t* packet = linx_audio_stream_packet_create(payload_size);
                                if (packet) {
                                    packet->sample_rate = ws_protocol->base.server_sample_rate;
                                    packet->frame_duration = ws_protocol->base.server_frame_duration;
                                    packet->timestamp = 0; /* v3 protocol doesn't include timestamp */
                                    memcpy(packet->payload, bp3->payload, payload_size);
                                    
                                    LOG_DEBUG("调用音频回调函数 (协议v3)");
                                    ws_protocol->base.on_incoming_audio(packet, ws_protocol->base.user_data);
                                    linx_audio_stream_packet_destroy(packet);
                                } else {
                                    LOG_ERROR("音频包创建失败 (协议v3)");
                                }
                            }
                        } else {
                            LOG_WARN("协议v3消息长度不足: %zu < %zu", wm->data.len, sizeof(linx_binary_protocol3_t));
                        }
                    } else {
                        /* Fallback for unsupported protocol versions - treat as raw audio data */
                        LOG_DEBUG("使用原始音频数据处理 (协议v%d)", ws_protocol->version);
                        linx_audio_stream_packet_t* packet = linx_audio_stream_packet_create(wm->data.len);
                        if (packet) {
                            packet->sample_rate = ws_protocol->base.server_sample_rate;
                            packet->frame_duration = ws_protocol->base.server_frame_duration;
                            packet->timestamp = 0;
                            memcpy(packet->payload, wm->data.buf, wm->data.len);
                            
                            LOG_DEBUG("调用音频回调函数 (原始数据)");
                            ws_protocol->base.on_incoming_audio(packet, ws_protocol->base.user_data);
                            linx_audio_stream_packet_destroy(packet);
                        } else {
                            LOG_ERROR("音频包创建失败 (原始数据)");
                        }
                    }
                } else {
                    LOG_DEBUG("未注册音频回调函数，忽略二进制消息");
                }
            }
            break;
        }
        
        case MG_EV_CLOSE: {
            /* Connection closed */
            LOG_INFO("WebSocket连接已关闭");
            ws_protocol->connected = false;
            ws_protocol->audio_channel_opened = false;
            ws_protocol->conn = NULL;
            
            if (ws_protocol->base.on_disconnected) {
                LOG_DEBUG("调用连接断开回调函数");
                ws_protocol->base.on_disconnected(ws_protocol->base.user_data);
            }
            break;
        }
        
        case MG_EV_ERROR: {
            /* Connection error */
            char* error_msg = (char*)ev_data;
            LOG_ERROR("WebSocket连接错误: %s", error_msg ? error_msg : "未知错误");
            linx_protocol_set_error(&ws_protocol->base, error_msg ? error_msg : "WebSocket connection error");
            break;
        }
        
        default:
            LOG_DEBUG("收到未处理的WebSocket事件: %d", ev);
            break;
    }
}

/* Protocol implementation functions */
bool linx_websocket_start(linx_protocol_t* protocol) {
    linx_websocket_protocol_t* ws_protocol = (linx_websocket_protocol_t*)protocol;
    
    if (!ws_protocol || !ws_protocol->server_url) {
        return false;
    }
    
    /* Create WebSocket connection with headers */
    char headers[1024] = "";
    
    /* Build headers string */
    if (ws_protocol->auth_token) {
        char auth_header[512];
        /* Add "Bearer " prefix if token doesn't have a space */
        if (strchr(ws_protocol->auth_token, ' ') == NULL) {
            snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s\r\n", ws_protocol->auth_token);
        } else {
            snprintf(auth_header, sizeof(auth_header), "Authorization: %s\r\n", ws_protocol->auth_token);
        }
        strncat(headers, auth_header, sizeof(headers) - strlen(headers) - 1);
    }
    
    if (ws_protocol->version > 0) {
        char version_header[64];
        snprintf(version_header, sizeof(version_header), "Protocol-Version: %d\r\n", ws_protocol->version);
        strncat(headers, version_header, sizeof(headers) - strlen(headers) - 1);
    }
    
    if (ws_protocol->device_id) {
        char device_header[256];
        snprintf(device_header, sizeof(device_header), "Device-Id: %s\r\n", ws_protocol->device_id);
        strncat(headers, device_header, sizeof(headers) - strlen(headers) - 1);
    }
    
    if (ws_protocol->client_id) {
        char client_header[256];
        snprintf(client_header, sizeof(client_header), "Client-Id: %s\r\n", ws_protocol->client_id);
        strncat(headers, client_header, sizeof(headers) - strlen(headers) - 1);
    }
    
    ws_protocol->conn = mg_ws_connect(&ws_protocol->mgr, ws_protocol->server_url, 
                                     linx_websocket_event_handler, ws_protocol, 
                                     strlen(headers) > 0 ? "%s" : NULL, headers);
    
    if (!ws_protocol->conn) {
        return false;
    }
    
    ws_protocol->running = true;
    ws_protocol->should_stop = false;
    
    return true;
}





bool linx_websocket_send_audio(linx_protocol_t* protocol, linx_audio_stream_packet_t* packet) {
    linx_websocket_protocol_t* ws_protocol = (linx_websocket_protocol_t*)protocol;
    
    if (!ws_protocol || !ws_protocol->conn || !ws_protocol->connected || !packet) {
        printf("Linx: Invalid websocket protocol\n");
        return false;
    }
    printf("Linx: Sending audio packet\n");
    printf("   - Sample Rate: %d\n", packet->sample_rate);
    printf("   - Frame Duration: %d\n", packet->frame_duration);
    printf("   - Timestamp: %u\n", packet->timestamp);
    printf("   - Payload Size: %zu\n", packet->payload_size);
    printf("   - version: %d\n",ws_protocol->version);
    
    if (ws_protocol->version == 2) {

        /* Use binary protocol v2 */
        size_t total_size = sizeof(linx_binary_protocol2_t) + packet->payload_size;
        uint8_t* buffer = malloc(total_size);
        if (!buffer) {
            printf("❌ WebSocket发送失败: 内存分配失败 (协议v2)\n");
            return false;
        }
        
        linx_binary_protocol2_t* bp2 = (linx_binary_protocol2_t*)buffer;
        bp2->version = htons(ws_protocol->version);
        bp2->type = htons(0); /* Audio type */
        bp2->reserved = 0;
        bp2->timestamp = htonl(packet->timestamp);
        bp2->payload_size = htonl(packet->payload_size);
        memcpy(bp2->payload, packet->payload, packet->payload_size);
        
        int send_result = mg_ws_send(ws_protocol->conn, buffer, total_size, WEBSOCKET_OP_BINARY);
        free(buffer);
        
        if (send_result > 0) {
            printf("✅ WebSocket发送成功: %zu 字节 (协议v2, 总大小: %zu)\n", packet->payload_size, total_size);
        } else {
            printf("❌ WebSocket发送失败: mg_ws_send返回 %d (协议v2)\n", send_result);
        }
        
        return send_result > 0;
    } else if (ws_protocol->version == 3) {
        /* Use binary protocol v3 */
        size_t total_size = sizeof(linx_binary_protocol3_t) + packet->payload_size;
        uint8_t* buffer = malloc(total_size);
        if (!buffer) {
            printf("❌ WebSocket发送失败: 内存分配失败 (协议v3)\n");
            return false;
        }
        
        linx_binary_protocol3_t* bp3 = (linx_binary_protocol3_t*)buffer;
        bp3->type = 0; /* Audio type */
        bp3->reserved = 0;
        bp3->payload_size = htons(packet->payload_size);
        memcpy(bp3->payload, packet->payload, packet->payload_size);
        
        int send_result = mg_ws_send(ws_protocol->conn, buffer, total_size, WEBSOCKET_OP_BINARY);
        free(buffer);
        
        if (send_result > 0) {
            printf("✅ WebSocket发送成功: %zu 字节 (协议v3, 总大小: %zu)\n", packet->payload_size, total_size);
        } else {
            printf("❌ WebSocket发送失败: mg_ws_send返回 %d (协议v3)\n", send_result);
        }
        
        return send_result > 0;
    } else {
        /* Fallback for unsupported protocol versions - send raw payload */
        int send_result = mg_ws_send(ws_protocol->conn, packet->payload, packet->payload_size, WEBSOCKET_OP_BINARY);
        
        if (send_result > 0) {
            printf("✅ WebSocket发送成功: %zu 字节 (原始数据, 协议v%d)\n", packet->payload_size, ws_protocol->version);
        } else {
            printf("❌ WebSocket发送失败: mg_ws_send返回 %d (原始数据, 协议v%d)\n", send_result, ws_protocol->version);
        }
        
        return send_result > 0;
    }
}

bool linx_websocket_send_text(linx_protocol_t* protocol, const char* text) {
    linx_websocket_protocol_t* ws_protocol = (linx_websocket_protocol_t*)protocol;
    
    if (!ws_protocol || !ws_protocol->conn || !ws_protocol->connected || !text) {
        printf("❌ WebSocket发送文本失败: 无效的协议或连接或未连接或文本为空\n");
        return false;
    }
    printf("🚀 WebSocket发送文本: %s\n", text);
    mg_ws_send(ws_protocol->conn, text, strlen(text), WEBSOCKET_OP_TEXT);
    return true;
}

void linx_websocket_destroy(linx_protocol_t* protocol) {
    linx_websocket_protocol_t* ws_protocol = (linx_websocket_protocol_t*)protocol;
    linx_websocket_protocol_destroy(ws_protocol);
}

void linx_websocket_destroy_direct(linx_websocket_protocol_t* protocol) {
    linx_websocket_protocol_destroy(protocol);
}

/* Event loop functions */
void linx_websocket_poll(linx_websocket_protocol_t* ws_protocol, int timeout_ms) {
    if (!ws_protocol) {
        return;
    }
    
    mg_mgr_poll(&ws_protocol->mgr, timeout_ms);
}

void linx_websocket_stop(linx_websocket_protocol_t* ws_protocol) {
    if (!ws_protocol) {
        return;
    }
    
    ws_protocol->should_stop = true;
    ws_protocol->running = false;
    
    if (ws_protocol->conn) {
        ws_protocol->conn->is_closing = 1;
        ws_protocol->conn = NULL;
    }
}

/* cJSON-based JSON value extraction helpers */
static char* extract_json_string_value(const cJSON* json, const char* key) {
    const cJSON* item = cJSON_GetObjectItemCaseSensitive(json, key);
    if (!cJSON_IsString(item) || (item->valuestring == NULL)) {
        return NULL;
    }
    
    size_t len = strlen(item->valuestring);
    char* result = malloc(len + 1);
    if (!result) return NULL;
    
    strcpy(result, item->valuestring);
    return result;
}

static int extract_json_int_value(const cJSON* json, const char* key) {
    const cJSON* item = cJSON_GetObjectItemCaseSensitive(json, key);
    if (!cJSON_IsNumber(item)) {
        return -1;
    }
    
    return item->valueint;
}

/* Helper functions */
bool linx_websocket_parse_server_hello(linx_websocket_protocol_t* ws_protocol, const char* json_str) {
    if (!ws_protocol || !json_str) {
        return false;
    }
    
    cJSON* root = cJSON_Parse(json_str);
    if (!root) {
        return false;
    }
    
    /* Check transport type */
    char* transport = extract_json_string_value(root, "transport");
    if (transport) {
        if (strcmp(transport, "websocket") != 0) {
            free(transport);
            cJSON_Delete(root);
            return false;
        }
        free(transport);
    }
    
    /* Parse session ID */
    char* session_id = extract_json_string_value(root, "session_id");
    if (session_id) {
        if (ws_protocol->session_id) {
            free(ws_protocol->session_id);
        }
        ws_protocol->session_id = session_id;
    }
    
    /* Parse audio_params section */
    const cJSON* audio_params = cJSON_GetObjectItemCaseSensitive(root, "audio_params");
    if (cJSON_IsObject(audio_params)) {
        int sample_rate = extract_json_int_value(audio_params, "sample_rate");
        if (sample_rate > 0) {
            ws_protocol->server_sample_rate = sample_rate;
        }
        
        int frame_duration = extract_json_int_value(audio_params, "frame_duration");
        if (frame_duration > 0) {
            ws_protocol->server_frame_duration = frame_duration;
        }
    }
    
    ws_protocol->server_hello_received = true;
    cJSON_Delete(root);
    return true;
}

char* linx_websocket_get_hello_message(linx_websocket_protocol_t* ws_protocol) {
    if (!ws_protocol) {
        return NULL;
    }
    
    /* Build JSON using cJSON for better structure */
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "hello");
    cJSON_AddNumberToObject(root, "version", ws_protocol->version);
    
    /* Add features object */
    cJSON* features = cJSON_CreateObject();
    cJSON_AddBoolToObject(features, "mcp", true);
    /* Note: AEC feature would be added here if supported */
    cJSON_AddItemToObject(root, "features", features);
    
    cJSON_AddStringToObject(root, "transport", "websocket");
    
    /* Add audio_params object */
    cJSON* audio_params = cJSON_CreateObject();
    cJSON_AddStringToObject(audio_params, "format", LINX_WEBSOCKET_AUDIO_FORMAT);
    cJSON_AddNumberToObject(audio_params, "sample_rate", LINX_WEBSOCKET_AUDIO_SAMPLE_RATE);
    cJSON_AddNumberToObject(audio_params, "channels", LINX_WEBSOCKET_AUDIO_CHANNELS);
    cJSON_AddNumberToObject(audio_params, "frame_duration", LINX_WEBSOCKET_AUDIO_FRAME_DURATION);
    cJSON_AddItemToObject(root, "audio_params", audio_params);
    
    char* json_string = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    
    return json_string;
}

/* Utility functions */
bool linx_websocket_is_connected(const linx_websocket_protocol_t* ws_protocol) {
    return ws_protocol ? ws_protocol->connected : false;
}

/* Additional WebSocket functions */
int linx_websocket_get_reconnect_attempts(const linx_websocket_protocol_t* protocol) {
    /* TODO: Add reconnect attempts tracking */
    return 0;
}

void linx_websocket_reset_reconnect_attempts(linx_websocket_protocol_t* protocol) {
    /* TODO: Reset reconnect attempts counter */
}

void linx_websocket_process_events(linx_websocket_protocol_t* protocol) {
    if (!protocol) return;
    linx_websocket_poll(protocol, 10);
}

bool linx_websocket_send_ping(linx_websocket_protocol_t* protocol) {
    if (!protocol || !protocol->conn) {
        return false;
    }
    
    mg_ws_send(protocol->conn, "", 0, WEBSOCKET_OP_PING);
    return true;
}

bool linx_websocket_is_connection_timeout(const linx_websocket_protocol_t* protocol) {
    /* TODO: Implement connection timeout check */
    return false;
}

/* WebSocket create function with config */
linx_websocket_protocol_t* linx_websocket_create(const linx_websocket_config_t* config) {
    if (!config || !config->url) {
        return NULL;
    }
    
    linx_websocket_protocol_t* ws_protocol = linx_websocket_protocol_create();
    if (!ws_protocol) {
        return NULL;
    }
    
    /* Set server URL */
    if (!linx_websocket_protocol_set_server_url(ws_protocol, config->url)) {
        linx_websocket_protocol_destroy(ws_protocol);
        return NULL;
    }
    
    /* Set authentication token if provided */
    if (config->auth_token && !linx_websocket_protocol_set_auth_token(ws_protocol, config->auth_token)) {
        linx_websocket_protocol_destroy(ws_protocol);
        return NULL;
    }
    
    /* Set device ID if provided */
    if (config->device_id && !linx_websocket_protocol_set_device_id(ws_protocol, config->device_id)) {
        linx_websocket_protocol_destroy(ws_protocol);
        return NULL;
    }
    
    /* Set client ID if provided */
    if (config->client_id && !linx_websocket_protocol_set_client_id(ws_protocol, config->client_id)) {
        linx_websocket_protocol_destroy(ws_protocol);
        return NULL;
    }
    
    /* Set protocol version if provided */
    if (config->protocol_version > 0) {
        ws_protocol->version = config->protocol_version;
    }
    
    return ws_protocol;
}

/* Start the WebSocket protocol */
int linx_websocket_protocol_start(linx_protocol_t* protocol) {
    if (!protocol) {
        LOG_ERROR("启动WebSocket协议失败: 协议实例为空");
        return -1;
    }
    
    LOG_DEBUG("开始启动WebSocket协议");
    
    linx_websocket_protocol_t* ws_protocol = (linx_websocket_protocol_t*)protocol;
    
    if (!ws_protocol->server_url) {
        LOG_ERROR("启动WebSocket协议失败: 服务器URL未设置");
        return -1;
    }
    
    LOG_INFO("启动WebSocket协议，连接到: %s", ws_protocol->server_url);
    
    /* Initialize mongoose manager */
    mg_mgr_init(&ws_protocol->mgr);
    LOG_DEBUG("Mongoose管理器初始化完成");
    
    /* Create WebSocket connection */
    ws_protocol->conn = mg_ws_connect(&ws_protocol->mgr, ws_protocol->server_url, 
                                      linx_websocket_event_handler, ws_protocol, NULL);
    
    if (!ws_protocol->conn) {
        LOG_ERROR("WebSocket连接创建失败");
        mg_mgr_free(&ws_protocol->mgr);
        return -1;
    }
    
    LOG_DEBUG("WebSocket连接创建成功，开始事件循环");
    ws_protocol->running = true;
    
    /* Start event loop in a separate thread */
    if (pthread_create(&ws_protocol->thread, NULL, linx_websocket_thread_func, ws_protocol) != 0) {
        LOG_ERROR("WebSocket线程创建失败");
        mg_mgr_free(&ws_protocol->mgr);
        ws_protocol->running = false;
        return -1;
    }
    
    LOG_INFO("WebSocket协议启动成功");
    return 0;
}

/* Stop the WebSocket protocol */
int linx_websocket_protocol_stop(linx_protocol_t* protocol) {
    if (!protocol) {
        LOG_ERROR("停止WebSocket协议失败: 协议实例为空");
        return -1;
    }
    
    LOG_DEBUG("开始停止WebSocket协议");
    
    linx_websocket_protocol_t* ws_protocol = (linx_websocket_protocol_t*)protocol;
    
    if (!ws_protocol->running) {
        LOG_WARN("WebSocket协议未在运行中");
        return 0;
    }
    
    LOG_INFO("停止WebSocket协议");
    
    /* Stop the event loop */
    ws_protocol->running = false;
    
    /* Close connection if still open */
    if (ws_protocol->conn) {
        LOG_DEBUG("关闭WebSocket连接");
        ws_protocol->conn->is_closing = 1;
    }
    
    /* Wait for thread to finish */
    if (pthread_join(ws_protocol->thread, NULL) != 0) {
        LOG_WARN("等待WebSocket线程结束失败");
    } else {
        LOG_DEBUG("WebSocket线程已结束");
    }
    
    /* Clean up mongoose manager */
    mg_mgr_free(&ws_protocol->mgr);
    LOG_DEBUG("Mongoose管理器已清理");
    
    ws_protocol->connected = false;
    ws_protocol->audio_channel_opened = false;
    ws_protocol->conn = NULL;
    
    LOG_INFO("WebSocket协议停止完成");
    return 0;
}

/* Send audio data through WebSocket */
int linx_websocket_protocol_send_audio(linx_protocol_t* protocol, const linx_audio_stream_packet_t* packet) {
    if (!protocol || !packet) {
        LOG_ERROR("发送音频数据失败: 参数为空");
        return -1;
    }
    
    linx_websocket_protocol_t* ws_protocol = (linx_websocket_protocol_t*)protocol;
    
    if (!ws_protocol->connected) {
        LOG_WARN("发送音频数据失败: WebSocket未连接");
        return -1;
    }
    
    if (!ws_protocol->audio_channel_opened) {
        LOG_WARN("发送音频数据失败: 音频通道未打开");
        return -1;
    }
    
    LOG_DEBUG("发送音频数据 (大小: %zu, 协议版本: %d)", packet->payload_size, ws_protocol->version);
    
    if (ws_protocol->version == 2) {
        /* Use binary protocol v2 */
        size_t total_size = sizeof(linx_binary_protocol2_t) + packet->payload_size;
        uint8_t* buffer = malloc(total_size);
        if (!buffer) {
            LOG_ERROR("音频数据发送失败: 内存分配失败 (协议v2)");
            return -1;
        }
        
        linx_binary_protocol2_t* bp2 = (linx_binary_protocol2_t*)buffer;
        bp2->version = htons(2);
        bp2->type = htons(0); /* Audio data */
        bp2->timestamp = htonl(packet->timestamp);
        bp2->payload_size = htonl(packet->payload_size);
        memcpy(bp2->payload, packet->payload, packet->payload_size);
        
        mg_ws_send(ws_protocol->conn, buffer, total_size, WEBSOCKET_OP_BINARY);
        LOG_DEBUG("音频数据发送成功 (协议v2, 总大小: %zu)", total_size);
        free(buffer);
        
    } else if (ws_protocol->version == 3) {
        /* Use binary protocol v3 */
        size_t total_size = sizeof(linx_binary_protocol3_t) + packet->payload_size;
        uint8_t* buffer = malloc(total_size);
        if (!buffer) {
            LOG_ERROR("音频数据发送失败: 内存分配失败 (协议v3)");
            return -1;
        }
        
        linx_binary_protocol3_t* bp3 = (linx_binary_protocol3_t*)buffer;
        bp3->type = 0; /* Audio data */
        bp3->payload_size = htons(packet->payload_size);
        memcpy(bp3->payload, packet->payload, packet->payload_size);
        
        mg_ws_send(ws_protocol->conn, buffer, total_size, WEBSOCKET_OP_BINARY);
        LOG_DEBUG("音频数据发送成功 (协议v3, 总大小: %zu)", total_size);
        free(buffer);
        
    } else {
        /* Fallback - send raw audio data */
        mg_ws_send(ws_protocol->conn, packet->payload, packet->payload_size, WEBSOCKET_OP_BINARY);
        LOG_DEBUG("音频数据发送成功 (原始数据, 大小: %zu)", packet->payload_size);
    }
    
    return 0;
}