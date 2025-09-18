#ifndef MCP_TOOL_H
#define MCP_TOOL_H

#include "mcp_types.h"
#include "mcp_property.h"
#include "mcp_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Tool structure */
typedef struct mcp_tool {
    char name[MCP_MAX_NAME_LENGTH];
    char description[MCP_MAX_DESCRIPTION_LENGTH];
    mcp_property_list_t* properties;
    mcp_tool_callback_t callback;
    bool user_only;
} mcp_tool_t;

/* Tool functions */
mcp_tool_t* mcp_tool_create(const char* name, const char* description, 
                           mcp_property_list_t* properties, mcp_tool_callback_t callback);
void mcp_tool_destroy(mcp_tool_t* tool);
void mcp_tool_set_user_only(mcp_tool_t* tool, bool user_only);
char* mcp_tool_to_json(const mcp_tool_t* tool);
char* mcp_tool_call(const mcp_tool_t* tool, const mcp_property_list_t* properties);

/* Return value helper functions */
mcp_return_value_t mcp_return_bool(bool value);
mcp_return_value_t mcp_return_int(int value);
mcp_return_value_t mcp_return_string(const char* value);
mcp_return_value_t mcp_return_json(cJSON* value);
mcp_return_value_t mcp_return_image(mcp_image_content_t* value);

/* Return value cleanup */
void mcp_return_value_cleanup(mcp_return_value_t* ret_val, mcp_return_type_t type);

#ifdef __cplusplus
}
#endif

#endif /* MCP_TOOL_H */