#ifndef MCP_UTILS_H
#define MCP_UTILS_H

#include "mcp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Image content structure */
typedef struct mcp_image_content {
    char* mime_type;
    char* encoded_data;
} mcp_image_content_t;

/* Base64 encoding function */
char* mcp_base64_encode(const char* data, size_t data_len);

/* Image content functions */
mcp_image_content_t* mcp_image_content_create(const char* mime_type, const char* data, size_t data_len);
void mcp_image_content_destroy(mcp_image_content_t* image);
char* mcp_image_content_to_json(const mcp_image_content_t* image);

/* String utility functions */
char* mcp_strdup(const char* str);
void mcp_free_string(char* str);

/* JSON utility functions */
char* mcp_json_to_string(const cJSON* json);

#ifdef __cplusplus
}
#endif

#endif /* MCP_UTILS_H */