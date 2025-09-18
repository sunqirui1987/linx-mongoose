#ifndef MCP_TYPES_H
#define MCP_TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <cJSON.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Property types */
typedef enum {
    MCP_PROPERTY_TYPE_BOOLEAN,
    MCP_PROPERTY_TYPE_INTEGER,
    MCP_PROPERTY_TYPE_STRING
} mcp_property_type_t;

/* Return value types */
typedef enum {
    MCP_RETURN_TYPE_BOOL,
    MCP_RETURN_TYPE_INT,
    MCP_RETURN_TYPE_STRING,
    MCP_RETURN_TYPE_JSON,
    MCP_RETURN_TYPE_IMAGE
} mcp_return_type_t;

/* Property value union */
typedef union {
    bool bool_val;
    int int_val;
    char* string_val;
} mcp_property_value_t;

/* Return value structure with type information */
typedef struct {
    mcp_return_type_t type;
    union {
        bool bool_val;
        int int_val;
        char* string_val;
        cJSON* json_val;
        struct mcp_image_content* image_val;
    } value;
} mcp_return_value_t;

/* Forward declarations */
struct mcp_property;
struct mcp_property_list;
struct mcp_tool;
struct mcp_server;
struct mcp_image_content;

/* Tool callback function type */
typedef mcp_return_value_t (*mcp_tool_callback_t)(const struct mcp_property_list* properties);

/* 能力配置回调函数类型 */
/* 摄像头解释URL设置回调 */
typedef void (*mcp_camera_set_explain_url_callback_t)(const char* url, const char* token);

/* 能力配置结构体 */
typedef struct {
    mcp_camera_set_explain_url_callback_t camera_set_explain_url;
    /* 可以在这里添加其他硬件能力的回调函数指针 */
} mcp_capability_callbacks_t;

/* Constants */
#define MCP_MAX_NAME_LENGTH 256
#define MCP_MAX_DESCRIPTION_LENGTH 1024
#define MCP_MAX_TOOLS 64
#define MCP_MAX_PROPERTIES 32
#define MCP_MAX_URL_LENGTH 512

#ifdef __cplusplus
}
#endif

#endif /* MCP_TYPES_H */