#include "mcp_server.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Global send message callback */
static mcp_send_message_callback_t g_send_callback = NULL;

mcp_server_t* mcp_server_create(const char* server_name, const char* server_version) {
    if (!server_name || !server_version) {
        return NULL;
    }
    
    mcp_server_t* server = malloc(sizeof(mcp_server_t));
    if (!server) {
        return NULL;
    }
    
    memset(server->tools, 0, sizeof(server->tools));
    server->tool_count = 0;
    
    strncpy(server->server_name, server_name, MCP_MAX_NAME_LENGTH - 1);
    server->server_name[MCP_MAX_NAME_LENGTH - 1] = '\0';
    
    strncpy(server->server_version, server_version, sizeof(server->server_version) - 1);
    server->server_version[sizeof(server->server_version) - 1] = '\0';
    
    return server;
}

void mcp_server_destroy(mcp_server_t* server) {
    if (server) {
        for (size_t i = 0; i < server->tool_count; i++) {
            mcp_tool_destroy(server->tools[i]);
        }
        free(server);
    }
}

bool mcp_server_add_tool(mcp_server_t* server, mcp_tool_t* tool) {
    if (!server || !tool || server->tool_count >= MCP_MAX_TOOLS) {
        return false;
    }
    
    /* Check for duplicate tool names */
    for (size_t i = 0; i < server->tool_count; i++) {
        if (strcmp(server->tools[i]->name, tool->name) == 0) {
            return false;
        }
    }
    
    server->tools[server->tool_count] = tool;
    server->tool_count++;
    return true;
}

bool mcp_server_add_simple_tool(mcp_server_t* server, const char* name, const char* description,
                                mcp_property_list_t* properties, mcp_tool_callback_t callback) {
    mcp_tool_t* tool = mcp_tool_create(name, description, properties, callback);
    if (!tool) {
        return false;
    }
    
    if (!mcp_server_add_tool(server, tool)) {
        mcp_tool_destroy(tool);
        return false;
    }
    
    return true;
}

bool mcp_server_add_user_only_tool(mcp_server_t* server, const char* name, const char* description,
                                   mcp_property_list_t* properties, mcp_tool_callback_t callback) {
    mcp_tool_t* tool = mcp_tool_create(name, description, properties, callback);
    if (!tool) {
        return false;
    }
    
    mcp_tool_set_user_only(tool, true);
    
    if (!mcp_server_add_tool(server, tool)) {
        mcp_tool_destroy(tool);
        return false;
    }
    
    return true;
}

const mcp_tool_t* mcp_server_find_tool(const mcp_server_t* server, const char* name) {
    if (!server || !name) {
        return NULL;
    }
    
    for (size_t i = 0; i < server->tool_count; i++) {
        if (strcmp(server->tools[i]->name, name) == 0) {
            return server->tools[i];
        }
    }
    
    return NULL;
}

void mcp_server_set_send_callback(mcp_send_message_callback_t callback) {
    g_send_callback = callback;
}

void mcp_server_parse_message(mcp_server_t* server, const char* message) {
    if (!server || !message) {
        return;
    }
    
    cJSON* json = cJSON_Parse(message);
    if (!json) {
        return;
    }
    
    mcp_server_parse_json_message(server, json);
    cJSON_Delete(json);
}

void mcp_server_parse_json_message(mcp_server_t* server, const cJSON* json) {
    if (!server || !json) {
        return;
    }
    
    /* Check JSONRPC version */
    const cJSON* version = cJSON_GetObjectItem(json, "jsonrpc");
    if (!version || !cJSON_IsString(version) || strcmp(version->valuestring, "2.0") != 0) {
        return;
    }
    
    /* Check method */
    const cJSON* method = cJSON_GetObjectItem(json, "method");
    if (!method || !cJSON_IsString(method)) {
        return;
    }
    
    /* Skip notifications */
    if (strstr(method->valuestring, "notifications") == method->valuestring) {
        return;
    }
    
    /* Check params */
    const cJSON* params = cJSON_GetObjectItem(json, "params");
    if (params && !cJSON_IsObject(params)) {
        return;
    }
    
    /* Check id */
    const cJSON* id = cJSON_GetObjectItem(json, "id");
    if (!id || !cJSON_IsNumber(id)) {
        return;
    }
    
    int id_int = id->valueint;
    const char* method_str = method->valuestring;
    
    if (strcmp(method_str, "initialize") == 0) {
        mcp_server_handle_initialize(server, id_int, params);
    } else if (strcmp(method_str, "tools/list") == 0) {
        mcp_server_handle_tools_list(server, id_int, params);
    } else if (strcmp(method_str, "tools/call") == 0) {
        mcp_server_handle_tools_call(server, id_int, params);
    } else {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Method not implemented: %s", method_str);
        mcp_server_reply_error(id_int, error_msg);
    }
}

void mcp_server_reply_result(int id, const char* result) {
    if (!g_send_callback || !result) {
        return;
    }
    
    char* payload = malloc(strlen(result) + 128);
    if (!payload) {
        return;
    }
    
    sprintf(payload, "{\"jsonrpc\":\"2.0\",\"id\":%d,\"result\":%s}", id, result);
    g_send_callback(payload);
    free(payload);
}

void mcp_server_reply_error(int id, const char* message) {
    if (!g_send_callback || !message) {
        return;
    }
    
    char* payload = malloc(strlen(message) + 128);
    if (!payload) {
        return;
    }
    
    sprintf(payload, "{\"jsonrpc\":\"2.0\",\"id\":%d,\"error\":{\"message\":\"%s\"}}", id, message);
    g_send_callback(payload);
    free(payload);
}

void mcp_server_handle_initialize(mcp_server_t* server, int id, const cJSON* params) {
    if (!server) {
        return;
    }
    
    /* TODO: Parse capabilities if needed */
    
    char result[512];
    snprintf(result, sizeof(result),
        "{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{\"tools\":{}},\"serverInfo\":{\"name\":\"%s\",\"version\":\"%s\"}}",
        server->server_name, server->server_version);
    
    mcp_server_reply_result(id, result);
}

void mcp_server_handle_tools_list(mcp_server_t* server, int id, const cJSON* params) {
    if (!server) {
        return;
    }
    
    const char* cursor = "";
    bool list_user_only_tools = false;
    
    if (params) {
        const cJSON* cursor_json = cJSON_GetObjectItem(params, "cursor");
        if (cursor_json && cJSON_IsString(cursor_json)) {
            cursor = cursor_json->valuestring;
        }
        
        const cJSON* with_user_tools = cJSON_GetObjectItem(params, "withUserTools");
        if (with_user_tools && cJSON_IsBool(with_user_tools)) {
            list_user_only_tools = (with_user_tools->valueint == 1);
        }
    }
    
    char* tools_json = mcp_server_get_tools_list_json(server, cursor, list_user_only_tools);
    if (tools_json) {
        mcp_server_reply_result(id, tools_json);
        free(tools_json);
    } else {
        mcp_server_reply_error(id, "Failed to generate tools list");
    }
}

void mcp_server_handle_tools_call(mcp_server_t* server, int id, const cJSON* params) {
    if (!server || !params) {
        mcp_server_reply_error(id, "Missing params");
        return;
    }
    
    const cJSON* tool_name = cJSON_GetObjectItem(params, "name");
    if (!tool_name || !cJSON_IsString(tool_name)) {
        mcp_server_reply_error(id, "Missing name");
        return;
    }
    
    const cJSON* tool_arguments = cJSON_GetObjectItem(params, "arguments");
    if (tool_arguments && !cJSON_IsObject(tool_arguments)) {
        mcp_server_reply_error(id, "Invalid arguments");
        return;
    }
    
    const mcp_tool_t* tool = mcp_server_find_tool(server, tool_name->valuestring);
    if (!tool) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Unknown tool: %s", tool_name->valuestring);
        mcp_server_reply_error(id, error_msg);
        return;
    }
    
    /* Parse arguments into property list */
    mcp_property_list_t* arguments = mcp_property_list_create();
    if (!arguments) {
        mcp_server_reply_error(id, "Failed to create arguments list");
        return;
    }
    
    /* Copy tool properties and set values from arguments */
    if (tool->properties) {
        for (size_t i = 0; i < tool->properties->count; i++) {
            mcp_property_t* prop = &tool->properties->properties[i];
            mcp_property_list_add(arguments, prop);
            
            /* Find corresponding argument value */
            if (tool_arguments) {
                const cJSON* value = cJSON_GetObjectItem(tool_arguments, prop->name);
                mcp_property_t* arg_prop = mcp_property_list_find_mutable(arguments, prop->name);
                
                if (value && arg_prop) {
                    bool found = false;
                    if (prop->type == MCP_PROPERTY_TYPE_BOOLEAN && cJSON_IsBool(value)) {
                        mcp_property_set_bool_value(arg_prop, value->valueint == 1);
                        found = true;
                    } else if (prop->type == MCP_PROPERTY_TYPE_INTEGER && cJSON_IsNumber(value)) {
                        mcp_property_set_int_value(arg_prop, value->valueint);
                        found = true;
                    } else if (prop->type == MCP_PROPERTY_TYPE_STRING && cJSON_IsString(value)) {
                        mcp_property_set_string_value(arg_prop, value->valuestring);
                        found = true;
                    }
                    
                    if (!prop->has_default_value && !found) {
                        char error_msg[256];
                        snprintf(error_msg, sizeof(error_msg), "Missing valid argument: %s", prop->name);
                        mcp_server_reply_error(id, error_msg);
                        mcp_property_list_destroy(arguments);
                        return;
                    }
                }
            }
        }
    }
    
    /* Call the tool */
    char* result = mcp_tool_call(tool, arguments);
    mcp_property_list_destroy(arguments);
    
    if (result) {
        mcp_server_reply_result(id, result);
        free(result);
    } else {
        mcp_server_reply_error(id, "Tool call failed");
    }
}

char* mcp_server_get_tools_list_json(const mcp_server_t* server, const char* cursor, bool list_user_only_tools) {
    if (!server) {
        return NULL;
    }
    
    const int max_payload_size = 8000;
    cJSON* json = cJSON_CreateObject();
    cJSON* tools_array = cJSON_CreateArray();
    
    if (!json || !tools_array) {
        cJSON_Delete(json);
        cJSON_Delete(tools_array);
        return NULL;
    }
    
    bool found_cursor = (cursor == NULL || strlen(cursor) == 0);
    size_t start_index = 0;
    const char* next_cursor = NULL;
    
    /* Find starting position if cursor is provided */
    if (!found_cursor) {
        for (size_t i = 0; i < server->tool_count; i++) {
            if (strcmp(server->tools[i]->name, cursor) == 0) {
                found_cursor = true;
                start_index = i;
                break;
            }
        }
    }
    
    /* Add tools to array */
    for (size_t i = start_index; i < server->tool_count; i++) {
        const mcp_tool_t* tool = server->tools[i];
        
        if (!list_user_only_tools && tool->user_only) {
            continue;
        }
        
        char* tool_json_str = mcp_tool_to_json(tool);
        if (tool_json_str) {
            /* Check if adding this tool would exceed size limit */
            char* current_json_str = cJSON_PrintUnformatted(json);
            size_t current_size = current_json_str ? strlen(current_json_str) : 0;
            size_t tool_size = strlen(tool_json_str);
            
            if (current_size + tool_size + 100 > max_payload_size) {
                /* Set next cursor and break */
                next_cursor = tool->name;
                free(tool_json_str);
                free(current_json_str);
                break;
            }
            
            cJSON* tool_json = cJSON_Parse(tool_json_str);
            if (tool_json) {
                cJSON_AddItemToArray(tools_array, tool_json);
            }
            
            free(tool_json_str);
            free(current_json_str);
        }
    }
    
    cJSON_AddItemToObject(json, "tools", tools_array);
    
    if (next_cursor) {
        cJSON_AddStringToObject(json, "nextCursor", next_cursor);
    }
    
    char* result = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    
    return result;
}