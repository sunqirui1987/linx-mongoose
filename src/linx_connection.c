/**
 * @file linx_connection.c
 * @brief WebSocket连接管理模块实现
 * @version 1.0.0
 * @date 2024
 */

#include "linx_internal.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* ========== WebSocket连接管理函数 ========== */

int linx_establish_connection(void) {
    if (g_linx_context.conn) {
        LINX_LOGW("Connection already exists");
        return LINX_OK;
    }
    
    /* 构建WebSocket URL */
    char url[LINX_MAX_URL_LEN + 256];
    snprintf(url, sizeof(url), "%s", g_linx_context.config.server_url);
    
    LINX_LOGI("Connecting to: %s", url);
    
    /* 建立WebSocket连接 */
    g_linx_context.conn = mg_ws_connect(&g_linx_context.mgr, url, 
                                       linx_websocket_event_handler, NULL, NULL);
    
    if (!g_linx_context.conn) {
        linx_set_error(LINX_ERROR_NETWORK, "Failed to create WebSocket connection");
        return LINX_ERROR_NETWORK;
    }
    
    /* 设置请求头 */
    mg_printf(g_linx_context.conn, 
              "GET %s HTTP/1.1\r\n"
              "Host: %s\r\n"
              "Upgrade: websocket\r\n"
              "Connection: Upgrade\r\n"
              "Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n"
              "Sec-WebSocket-Version: 13\r\n"
              "Protocol-Version: %s\r\n"
              "Device-Id: %s\r\n"
              "Client-Id: %s\r\n"
              "User-Agent: %s\r\n",
              "/v1/ws/", "xrobo-io.qiniuapi.com",
              LINX_PROTOCOL_VERSION,
              g_linx_context.config.device_id,
              g_linx_context.config.client_id,
              LINX_USER_AGENT);
    
    /* 如果有访问令牌，添加Authorization头 */
    if (strlen(g_linx_context.config.token) > 0) {
        mg_printf(g_linx_context.conn, "Authorization: Bearer %s\r\n", 
                 g_linx_context.config.token);
    }
    
    mg_printf(g_linx_context.conn, "\r\n");
    
    g_linx_context.connection_state.state = LINX_STATE_CONNECTING;
    g_linx_context.connection_state.last_heartbeat = time(NULL);
    
    LINX_LOGI("WebSocket connection initiated");
    return LINX_OK;
}

int linx_send_hello_message(void) {
    char json_buffer[LINX_JSON_BUFFER_SIZE];
    int ret = linx_build_hello_message(json_buffer, sizeof(json_buffer));
    if (ret != LINX_OK) {
        return ret;
    }
    
    /* 发送Hello消息 */
    mg_ws_send(g_linx_context.conn, json_buffer, strlen(json_buffer), WEBSOCKET_OP_TEXT);
    
    g_linx_context.connection_state.state = LINX_STATE_HELLO_SENT;
    
    LINX_LOGI("Hello message sent");
    LINX_LOGD("Hello message: %s", json_buffer);
    
    return LINX_OK;
}

int linx_handle_reconnect(void) {
    if (g_linx_context.connection_state.reconnect_count >= 
        g_linx_context.config.max_reconnect_attempts) {
        LINX_LOGE("Max reconnect attempts reached: %d", 
                 g_linx_context.config.max_reconnect_attempts);
        linx_set_error(LINX_ERROR_NETWORK, "Max reconnect attempts reached");
        return LINX_ERROR_NETWORK;
    }
    
    g_linx_context.connection_state.reconnect_count++;
    
    /* 指数退避重连延迟 */
    int delay_ms = 1000 * (1 << (g_linx_context.connection_state.reconnect_count - 1));
    if (delay_ms > 30000) delay_ms = 30000; /* 最大30秒 */
    
    LINX_LOGI("Reconnecting in %d ms (attempt %d/%d)", 
             delay_ms, 
             g_linx_context.connection_state.reconnect_count,
             g_linx_context.config.max_reconnect_attempts);
    
    /* 重置连接状态 */
    linx_reset_connection_state();
    
    /* 延迟后重连（这里简化处理，实际应用中可能需要定时器） */
    usleep(delay_ms * 1000); /* 转换为微秒 */
    
    return linx_establish_connection();
}

/* ========== WebSocket事件处理函数 ========== */

void linx_websocket_event_handler(struct mg_connection *c, int ev, void *ev_data) {
    switch (ev) {
        case MG_EV_WS_OPEN:
            linx_handle_websocket_open(c);
            break;
            
        case MG_EV_WS_MSG:
            linx_handle_websocket_message(c, (struct mg_ws_message *)ev_data);
            break;
            
        case MG_EV_CLOSE:
            linx_handle_websocket_close(c);
            break;
            
        case MG_EV_ERROR:
            linx_handle_websocket_error(c, (char *)ev_data);
            break;
            
        case MG_EV_POLL:
            /* 心跳检测 */
            if (g_linx_context.connection_state.state >= LINX_STATE_CONNECTED) {
                time_t now = time(NULL);
                if (now - g_linx_context.connection_state.last_heartbeat > 
                    g_linx_context.config.heartbeat_interval_ms / 1000) {
                    LINX_LOGD("Heartbeat check");
                    g_linx_context.connection_state.last_heartbeat = now;
                }
            }
            break;
            
        default:
            break;
    }
}

void linx_handle_websocket_open(struct mg_connection *c) {
    LINX_LOGI("WebSocket connection opened");
    
    g_linx_context.connection_state.state = LINX_STATE_CONNECTED;
    g_linx_context.connection_state.reconnect_count = 0; /* 重置重连计数 */
    
    /* 发送Hello握手消息 */
    int ret = linx_send_hello_message();
    if (ret != LINX_OK) {
        LINX_LOGE("Failed to send hello message: %d", ret);
        linx_handle_websocket_error(c, "Failed to send hello message");
        return;
    }
    
    /* 调用连接成功回调 */
    if (g_linx_context.config.callbacks.on_connected) {
        g_linx_context.config.callbacks.on_connected(&g_linx_context);
    }
}

void linx_handle_websocket_message(struct mg_connection *c, struct mg_ws_message *wm) {
    if (!wm || !wm->data.buf) {
        LINX_LOGW("Received empty WebSocket message");
        return;
    }
    
    LINX_LOGD("Received WebSocket message, size: %zu, opcode: %d", 
             wm->data.len, wm->flags & 0x0f);
    
    /* 处理文本消息（JSON） */
    if ((wm->flags & 0x0f) == WEBSOCKET_OP_TEXT) {
        /* 确保字符串以null结尾 */
        char *json_str = malloc(wm->data.len + 1);
        if (!json_str) {
            LINX_LOGE("Failed to allocate memory for JSON message");
            linx_set_error(LINX_ERROR_MEMORY, "Failed to allocate memory for JSON message");
            return;
        }
        
        memcpy(json_str, wm->data.buf, wm->data.len);
        json_str[wm->data.len] = '\0';
        
        LINX_LOGD("Received JSON message: %s", json_str);
        
        /* 解析JSON消息 */
        int ret = linx_parse_json_message(json_str, wm->data.len);
        if (ret != LINX_OK) {
            LINX_LOGW("Failed to parse JSON message: %d", ret);
        }
        
        free(json_str);
    }
    /* 处理二进制消息（音频数据） */
    else if ((wm->flags & 0x0f) == WEBSOCKET_OP_BINARY) {
        LINX_LOGD("Received audio data, size: %zu", wm->data.len);
        linx_handle_audio_data((const uint8_t *)wm->data.buf, wm->data.len);
    }
    else {
        LINX_LOGW("Received unknown message type: %d", wm->flags & 0x0f);
    }
}

void linx_handle_websocket_close(struct mg_connection *c) {
    LINX_LOGI("WebSocket connection closed");
    
    linx_connection_state_t old_state = g_linx_context.connection_state.state;
    linx_reset_connection_state();
    
    /* 调用断开连接回调 */
    if (g_linx_context.config.callbacks.on_disconnected) {
        g_linx_context.config.callbacks.on_disconnected(&g_linx_context, 0);
    }
    
    /* 如果之前是连接状态，尝试重连 */
    if (old_state >= LINX_STATE_CONNECTED) {
        LINX_LOGI("Attempting to reconnect...");
        int ret = linx_handle_reconnect();
        if (ret != LINX_OK) {
            LINX_LOGE("Reconnection failed: %d", ret);
        }
    }
}

void linx_handle_websocket_error(struct mg_connection *c, const char *error_msg) {
    LINX_LOGE("WebSocket error: %s", error_msg ? error_msg : "Unknown error");
    
    linx_set_error(LINX_ERROR_NETWORK, error_msg);
    g_linx_context.connection_state.state = LINX_STATE_ERROR;
    
    /* 调用错误回调 */
    if (g_linx_context.config.callbacks.on_error) {
        g_linx_context.config.callbacks.on_error(&g_linx_context, 
                                                 LINX_ERROR_NETWORK, 
                                                 error_msg);
    }
    
    /* 尝试重连 */
    if (g_linx_context.connection_state.reconnect_count < 
        g_linx_context.config.max_reconnect_attempts) {
        LINX_LOGI("Attempting to reconnect after error...");
        linx_handle_reconnect();
    }
}