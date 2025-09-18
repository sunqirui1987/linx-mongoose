#ifndef MCP_SERVER_H
#define MCP_SERVER_H

#include "mcp_types.h"
#include "mcp_tool.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Server structure */
typedef struct mcp_server {
    mcp_tool_t* tools[MCP_MAX_TOOLS];
    size_t tool_count;
    char server_name[MCP_MAX_NAME_LENGTH];
    char server_version[64];
    mcp_capability_callbacks_t capability_callbacks;
} mcp_server_t;

/* Message sending callback type */
typedef void (*mcp_send_message_callback_t)(const char* message);

/* Server functions */
mcp_server_t* mcp_server_create(const char* server_name, const char* server_version);
void mcp_server_destroy(mcp_server_t* server);

/* Tool management */
bool mcp_server_add_tool(mcp_server_t* server, mcp_tool_t* tool);
bool mcp_server_add_simple_tool(mcp_server_t* server, const char* name, const char* description,
                                mcp_property_list_t* properties, mcp_tool_callback_t callback);
bool mcp_server_add_user_only_tool(mcp_server_t* server, const char* name, const char* description,
                                   mcp_property_list_t* properties, mcp_tool_callback_t callback);
const mcp_tool_t* mcp_server_find_tool(const mcp_server_t* server, const char* name);

/* Message processing */
void mcp_server_set_send_callback(mcp_send_message_callback_t callback);
void mcp_server_parse_message(mcp_server_t* server, const char* message);
void mcp_server_parse_json_message(mcp_server_t* server, const cJSON* json);

/* Response functions */
void mcp_server_reply_result(int id, const char* result);
void mcp_server_reply_error(int id, const char* message);

/* Handler functions */
void mcp_server_handle_initialize(mcp_server_t* server, int id, const cJSON* params);
void mcp_server_handle_tools_list(mcp_server_t* server, int id, const cJSON* params);
void mcp_server_handle_tools_call(mcp_server_t* server, int id, const cJSON* params);

/* Capability parsing */
void mcp_server_parse_capabilities(mcp_server_t* server, const cJSON* capabilities);

/* Capability configuration */
void mcp_server_set_capability_callbacks(mcp_server_t* server, const mcp_capability_callbacks_t* callbacks);

/* Utility functions */
char* mcp_server_get_tools_list_json(const mcp_server_t* server, const char* cursor, bool list_user_only_tools);

#ifdef __cplusplus
}
#endif

#endif /* MCP_SERVER_H */