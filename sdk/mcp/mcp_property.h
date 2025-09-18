#ifndef MCP_PROPERTY_H
#define MCP_PROPERTY_H

#include "mcp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Property structure */
typedef struct mcp_property {
    char name[MCP_MAX_NAME_LENGTH];
    mcp_property_type_t type;
    mcp_property_value_t value;
    bool has_default_value;
    bool has_range;
    int min_value;
    int max_value;
} mcp_property_t;

/* Property list structure */
typedef struct mcp_property_list {
    mcp_property_t properties[MCP_MAX_PROPERTIES];
    size_t count;
} mcp_property_list_t;

/* Property functions */
mcp_property_t* mcp_property_create_boolean(const char* name, bool default_value, bool has_default);
mcp_property_t* mcp_property_create_integer(const char* name, int default_value, bool has_default, 
                                           bool has_range, int min_value, int max_value);
mcp_property_t* mcp_property_create_string(const char* name, const char* default_value, bool has_default);

bool mcp_property_set_bool_value(mcp_property_t* prop, bool value);
bool mcp_property_set_int_value(mcp_property_t* prop, int value);
bool mcp_property_set_string_value(mcp_property_t* prop, const char* value);

bool mcp_property_get_bool_value(const mcp_property_t* prop);
int mcp_property_get_int_value(const mcp_property_t* prop);
const char* mcp_property_get_string_value(const mcp_property_t* prop);

char* mcp_property_to_json(const mcp_property_t* prop);
void mcp_property_destroy(mcp_property_t* prop);

/* Property list functions */
mcp_property_list_t* mcp_property_list_create(void);
void mcp_property_list_destroy(mcp_property_list_t* list);
bool mcp_property_list_add(mcp_property_list_t* list, const mcp_property_t* prop);
const mcp_property_t* mcp_property_list_find(const mcp_property_list_t* list, const char* name);
mcp_property_t* mcp_property_list_find_mutable(mcp_property_list_t* list, const char* name);
char* mcp_property_list_to_json(const mcp_property_list_t* list);
char* mcp_property_list_get_required_json(const mcp_property_list_t* list);

#ifdef __cplusplus
}
#endif

#endif /* MCP_PROPERTY_H */