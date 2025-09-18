#include "mcp_property.h"
#include "mcp_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

mcp_property_t* mcp_property_create_boolean(const char* name, bool default_value, bool has_default) {
    if (!name || strlen(name) >= MCP_MAX_NAME_LENGTH) {
        return NULL;
    }
    
    mcp_property_t* prop = malloc(sizeof(mcp_property_t));
    if (!prop) {
        return NULL;
    }
    
    strncpy(prop->name, name, MCP_MAX_NAME_LENGTH - 1);
    prop->name[MCP_MAX_NAME_LENGTH - 1] = '\0';
    prop->type = MCP_PROPERTY_TYPE_BOOLEAN;
    prop->has_default_value = has_default;
    prop->has_range = false;
    prop->min_value = 0;
    prop->max_value = 0;
    
    if (has_default) {
        prop->value.bool_val = default_value;
    }
    
    return prop;
}

mcp_property_t* mcp_property_create_integer(const char* name, int default_value, bool has_default,
                                           bool has_range, int min_value, int max_value) {
    if (!name || strlen(name) >= MCP_MAX_NAME_LENGTH) {
        return NULL;
    }
    
    if (has_range && min_value > max_value) {
        return NULL;
    }
    
    if (has_default && has_range && (default_value < min_value || default_value > max_value)) {
        return NULL;
    }
    
    mcp_property_t* prop = malloc(sizeof(mcp_property_t));
    if (!prop) {
        return NULL;
    }
    
    strncpy(prop->name, name, MCP_MAX_NAME_LENGTH - 1);
    prop->name[MCP_MAX_NAME_LENGTH - 1] = '\0';
    prop->type = MCP_PROPERTY_TYPE_INTEGER;
    prop->has_default_value = has_default;
    prop->has_range = has_range;
    prop->min_value = min_value;
    prop->max_value = max_value;
    
    if (has_default) {
        prop->value.int_val = default_value;
    }
    
    return prop;
}

mcp_property_t* mcp_property_create_string(const char* name, const char* default_value, bool has_default) {
    if (!name || strlen(name) >= MCP_MAX_NAME_LENGTH) {
        return NULL;
    }
    
    mcp_property_t* prop = malloc(sizeof(mcp_property_t));
    if (!prop) {
        return NULL;
    }
    
    strncpy(prop->name, name, MCP_MAX_NAME_LENGTH - 1);
    prop->name[MCP_MAX_NAME_LENGTH - 1] = '\0';
    prop->type = MCP_PROPERTY_TYPE_STRING;
    prop->has_default_value = has_default;
    prop->has_range = false;
    prop->min_value = 0;
    prop->max_value = 0;
    prop->value.string_val = NULL;
    
    if (has_default && default_value) {
        prop->value.string_val = mcp_strdup(default_value);
        if (!prop->value.string_val) {
            free(prop);
            return NULL;
        }
    }
    
    return prop;
}

bool mcp_property_set_bool_value(mcp_property_t* prop, bool value) {
    if (!prop || prop->type != MCP_PROPERTY_TYPE_BOOLEAN) {
        return false;
    }
    
    prop->value.bool_val = value;
    return true;
}

bool mcp_property_set_int_value(mcp_property_t* prop, int value) {
    if (!prop || prop->type != MCP_PROPERTY_TYPE_INTEGER) {
        return false;
    }
    
    if (prop->has_range && (value < prop->min_value || value > prop->max_value)) {
        return false;
    }
    
    prop->value.int_val = value;
    return true;
}

bool mcp_property_set_string_value(mcp_property_t* prop, const char* value) {
    if (!prop || prop->type != MCP_PROPERTY_TYPE_STRING || !value) {
        return false;
    }
    
    mcp_free_string(prop->value.string_val);
    prop->value.string_val = mcp_strdup(value);
    return prop->value.string_val != NULL;
}

bool mcp_property_get_bool_value(const mcp_property_t* prop) {
    if (!prop || prop->type != MCP_PROPERTY_TYPE_BOOLEAN) {
        return false;
    }
    return prop->value.bool_val;
}

int mcp_property_get_int_value(const mcp_property_t* prop) {
    if (!prop || prop->type != MCP_PROPERTY_TYPE_INTEGER) {
        return 0;
    }
    return prop->value.int_val;
}

const char* mcp_property_get_string_value(const mcp_property_t* prop) {
    if (!prop || prop->type != MCP_PROPERTY_TYPE_STRING) {
        return NULL;
    }
    return prop->value.string_val;
}

char* mcp_property_to_json(const mcp_property_t* prop) {
    if (!prop) {
        return NULL;
    }
    
    cJSON* json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }
    
    switch (prop->type) {
        case MCP_PROPERTY_TYPE_BOOLEAN:
            cJSON_AddStringToObject(json, "type", "boolean");
            if (prop->has_default_value) {
                cJSON_AddBoolToObject(json, "default", prop->value.bool_val);
            }
            break;
            
        case MCP_PROPERTY_TYPE_INTEGER:
            cJSON_AddStringToObject(json, "type", "integer");
            if (prop->has_default_value) {
                cJSON_AddNumberToObject(json, "default", prop->value.int_val);
            }
            if (prop->has_range) {
                cJSON_AddNumberToObject(json, "minimum", prop->min_value);
                cJSON_AddNumberToObject(json, "maximum", prop->max_value);
            }
            break;
            
        case MCP_PROPERTY_TYPE_STRING:
            cJSON_AddStringToObject(json, "type", "string");
            if (prop->has_default_value && prop->value.string_val) {
                cJSON_AddStringToObject(json, "default", prop->value.string_val);
            }
            break;
    }
    
    char* json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    
    return json_str;
}

void mcp_property_destroy(mcp_property_t* prop) {
    if (prop) {
        if (prop->type == MCP_PROPERTY_TYPE_STRING) {
            mcp_free_string(prop->value.string_val);
        }
        free(prop);
    }
}

mcp_property_list_t* mcp_property_list_create(void) {
    mcp_property_list_t* list = malloc(sizeof(mcp_property_list_t));
    if (list) {
        list->count = 0;
    }
    return list;
}

void mcp_property_list_destroy(mcp_property_list_t* list) {
    if (list) {
        for (size_t i = 0; i < list->count; i++) {
            if (list->properties[i].type == MCP_PROPERTY_TYPE_STRING) {
                mcp_free_string(list->properties[i].value.string_val);
            }
        }
        free(list);
    }
}

bool mcp_property_list_add(mcp_property_list_t* list, const mcp_property_t* prop) {
    if (!list || !prop || list->count >= MCP_MAX_PROPERTIES) {
        return false;
    }
    
    /* Check for duplicate names */
    for (size_t i = 0; i < list->count; i++) {
        if (strcmp(list->properties[i].name, prop->name) == 0) {
            return false;
        }
    }
    
    /* Copy property */
    list->properties[list->count] = *prop;
    
    /* Deep copy string value if needed */
    if (prop->type == MCP_PROPERTY_TYPE_STRING && prop->value.string_val) {
        list->properties[list->count].value.string_val = mcp_strdup(prop->value.string_val);
        if (!list->properties[list->count].value.string_val) {
            return false;
        }
    }
    
    list->count++;
    return true;
}

const mcp_property_t* mcp_property_list_find(const mcp_property_list_t* list, const char* name) {
    if (!list || !name) {
        return NULL;
    }
    
    for (size_t i = 0; i < list->count; i++) {
        if (strcmp(list->properties[i].name, name) == 0) {
            return &list->properties[i];
        }
    }
    
    return NULL;
}

mcp_property_t* mcp_property_list_find_mutable(mcp_property_list_t* list, const char* name) {
    if (!list || !name) {
        return NULL;
    }
    
    for (size_t i = 0; i < list->count; i++) {
        if (strcmp(list->properties[i].name, name) == 0) {
            return &list->properties[i];
        }
    }
    
    return NULL;
}

char* mcp_property_list_to_json(const mcp_property_list_t* list) {
    if (!list) {
        return NULL;
    }
    
    cJSON* json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }
    
    for (size_t i = 0; i < list->count; i++) {
        char* prop_json_str = mcp_property_to_json(&list->properties[i]);
        if (prop_json_str) {
            cJSON* prop_json = cJSON_Parse(prop_json_str);
            if (prop_json) {
                cJSON_AddItemToObject(json, list->properties[i].name, prop_json);
            }
            free(prop_json_str);
        }
    }
    
    char* json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    
    return json_str;
}

char* mcp_property_list_get_required_json(const mcp_property_list_t* list) {
    if (!list) {
        return NULL;
    }
    
    cJSON* required_array = cJSON_CreateArray();
    if (!required_array) {
        return NULL;
    }
    
    for (size_t i = 0; i < list->count; i++) {
        if (!list->properties[i].has_default_value) {
            cJSON_AddItemToArray(required_array, cJSON_CreateString(list->properties[i].name));
        }
    }
    
    char* json_str = cJSON_PrintUnformatted(required_array);
    cJSON_Delete(required_array);
    
    return json_str;
}