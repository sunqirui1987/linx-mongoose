#include "mcp_tool.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

mcp_tool_t* mcp_tool_create(const char* name, const char* description,
                           mcp_property_list_t* properties, mcp_tool_callback_t callback) {
    if (!name || !description || !callback) {
        return NULL;
    }
    
    if (strlen(name) >= MCP_MAX_NAME_LENGTH || strlen(description) >= MCP_MAX_DESCRIPTION_LENGTH) {
        return NULL;
    }
    
    mcp_tool_t* tool = malloc(sizeof(mcp_tool_t));
    if (!tool) {
        return NULL;
    }
    
    strncpy(tool->name, name, MCP_MAX_NAME_LENGTH - 1);
    tool->name[MCP_MAX_NAME_LENGTH - 1] = '\0';
    
    strncpy(tool->description, description, MCP_MAX_DESCRIPTION_LENGTH - 1);
    tool->description[MCP_MAX_DESCRIPTION_LENGTH - 1] = '\0';
    
    tool->properties = properties;
    tool->callback = callback;
    tool->user_only = false;
    
    return tool;
}

void mcp_tool_destroy(mcp_tool_t* tool) {
    if (tool) {
        if (tool->properties) {
            mcp_property_list_destroy(tool->properties);
        }
        free(tool);
    }
}

void mcp_tool_set_user_only(mcp_tool_t* tool, bool user_only) {
    if (tool) {
        tool->user_only = user_only;
    }
}

char* mcp_tool_to_json(const mcp_tool_t* tool) {
    if (!tool) {
        return NULL;
    }
    
    cJSON* json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }
    
    cJSON_AddStringToObject(json, "name", tool->name);
    cJSON_AddStringToObject(json, "description", tool->description);
    
    /* Create input schema */
    cJSON* input_schema = cJSON_CreateObject();
    cJSON_AddStringToObject(input_schema, "type", "object");
    
    if (tool->properties) {
        char* properties_json_str = mcp_property_list_to_json(tool->properties);
        if (properties_json_str) {
            cJSON* properties_json = cJSON_Parse(properties_json_str);
            if (properties_json) {
                cJSON_AddItemToObject(input_schema, "properties", properties_json);
            }
            free(properties_json_str);
        }
        
        char* required_json_str = mcp_property_list_get_required_json(tool->properties);
        if (required_json_str) {
            cJSON* required_json = cJSON_Parse(required_json_str);
            if (required_json && cJSON_GetArraySize(required_json) > 0) {
                cJSON_AddItemToObject(input_schema, "required", required_json);
            } else {
                cJSON_Delete(required_json);
            }
            free(required_json_str);
        }
    }
    
    cJSON_AddItemToObject(json, "inputSchema", input_schema);
    
    /* Add audience annotation if user only */
    if (tool->user_only) {
        cJSON* annotations = cJSON_CreateObject();
        cJSON* audience = cJSON_CreateArray();
        cJSON_AddItemToArray(audience, cJSON_CreateString("user"));
        cJSON_AddItemToObject(annotations, "audience", audience);
        cJSON_AddItemToObject(json, "annotations", annotations);
    }
    
    char* json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    
    return json_str;
}

char* mcp_tool_call(const mcp_tool_t* tool, const mcp_property_list_t* properties) {
    if (!tool || !tool->callback) {
        return NULL;
    }
    
    /* Call the tool callback */
    mcp_return_value_t return_value = tool->callback(properties);
    
    /* Create result JSON */
    cJSON* result = cJSON_CreateObject();
    cJSON* content = cJSON_CreateArray();
    
    if (!result || !content) {
        cJSON_Delete(result);
        cJSON_Delete(content);
        return NULL;
    }
    
    /* Handle different return types */
    cJSON* content_item = cJSON_CreateObject();
    if (!content_item) {
        cJSON_Delete(result);
        cJSON_Delete(content);
        return NULL;
    }
    
    /* Handle return type based on the type field */
    switch (return_value.type) {
        case MCP_RETURN_TYPE_IMAGE:
            if (return_value.value.image_val != NULL) {
                char* image_json = mcp_image_content_to_json(return_value.value.image_val);
                if (image_json) {
                    cJSON_AddStringToObject(content_item, "type", "image");
                    cJSON_AddStringToObject(content_item, "image", image_json);
                    free(image_json);
                }
                mcp_image_content_destroy(return_value.value.image_val);
                return_value.value.image_val = NULL; /* Prevent double free */
            }
            break;
            
        case MCP_RETURN_TYPE_JSON:
            if (return_value.value.json_val != NULL) {
                char* json_str = cJSON_PrintUnformatted(return_value.value.json_val);
                if (json_str) {
                    cJSON_AddStringToObject(content_item, "type", "text");
                    cJSON_AddStringToObject(content_item, "text", json_str);
                    free(json_str);
                }
                cJSON_Delete(return_value.value.json_val);
                return_value.value.json_val = NULL; /* Prevent double free */
            }
            break;
            
        case MCP_RETURN_TYPE_STRING:
            if (return_value.value.string_val != NULL) {
                cJSON_AddStringToObject(content_item, "type", "text");
                cJSON_AddStringToObject(content_item, "text", return_value.value.string_val);
                mcp_free_string(return_value.value.string_val);
                return_value.value.string_val = NULL; /* Prevent double free */
            }
            break;
            
        case MCP_RETURN_TYPE_BOOL:
            cJSON_AddStringToObject(content_item, "type", "text");
            cJSON_AddStringToObject(content_item, "text", return_value.value.bool_val ? "true" : "false");
            break;
            
        case MCP_RETURN_TYPE_INT:
            {
                char buffer[32];
                cJSON_AddStringToObject(content_item, "type", "text");
                snprintf(buffer, sizeof(buffer), "%d", return_value.value.int_val);
                cJSON_AddStringToObject(content_item, "text", buffer);
            }
            break;
            
        default:
            cJSON_AddStringToObject(content_item, "type", "text");
            cJSON_AddStringToObject(content_item, "text", "Unknown return type");
            break;
    }
    
    cJSON_AddItemToArray(content, content_item);
    cJSON_AddItemToObject(result, "content", content);
    cJSON_AddBoolToObject(result, "isError", false);
    
    char* result_str = cJSON_PrintUnformatted(result);
    cJSON_Delete(result);
    
    return result_str;
}

mcp_return_value_t mcp_return_bool(bool value) {
    mcp_return_value_t ret_val = {0};
    ret_val.type = MCP_RETURN_TYPE_BOOL;
    ret_val.value.bool_val = value;
    return ret_val;
}

mcp_return_value_t mcp_return_int(int value) {
    mcp_return_value_t ret_val = {0};
    ret_val.type = MCP_RETURN_TYPE_INT;
    ret_val.value.int_val = value;
    return ret_val;
}

mcp_return_value_t mcp_return_string(const char* value) {
    mcp_return_value_t ret_val = {0};
    ret_val.type = MCP_RETURN_TYPE_STRING;
    ret_val.value.string_val = value ? mcp_strdup(value) : NULL;
    return ret_val;
}

mcp_return_value_t mcp_return_json(cJSON* value) {
    mcp_return_value_t ret_val = {0};
    ret_val.type = MCP_RETURN_TYPE_JSON;
    ret_val.value.json_val = value;
    return ret_val;
}

mcp_return_value_t mcp_return_image(mcp_image_content_t* value) {
    mcp_return_value_t ret_val = {0};
    ret_val.type = MCP_RETURN_TYPE_IMAGE;
    ret_val.value.image_val = value;
    return ret_val;
}

void mcp_return_value_cleanup(mcp_return_value_t* ret_val, mcp_return_type_t type) {
    if (!ret_val) {
        return;
    }
    
    switch (type) {
        case MCP_RETURN_TYPE_STRING:
            mcp_free_string(ret_val->value.string_val);
            ret_val->value.string_val = NULL;
            break;
        case MCP_RETURN_TYPE_JSON:
            cJSON_Delete(ret_val->value.json_val);
            ret_val->value.json_val = NULL;
            break;
        case MCP_RETURN_TYPE_IMAGE:
            mcp_image_content_destroy(ret_val->value.image_val);
            ret_val->value.image_val = NULL;
            break;
        case MCP_RETURN_TYPE_BOOL:
        case MCP_RETURN_TYPE_INT:
        default:
            /* No cleanup needed for primitive types */
            break;
    }
}