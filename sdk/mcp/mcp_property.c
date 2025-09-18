/*
 * MCP属性管理实现文件
 * 实现属性的创建、设置、获取和管理功能
 */

#include "mcp_property.h"
#include "mcp_utils.h"      // 包含工具函数
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * 创建布尔类型属性
 */
mcp_property_t* mcp_property_create_boolean(const char* name, bool default_value, bool has_default) {
    // 检查参数有效性
    if (!name || strlen(name) >= MCP_MAX_NAME_LENGTH) {
        return NULL;
    }
    
    // 分配内存
    mcp_property_t* prop = malloc(sizeof(mcp_property_t));
    if (!prop) {
        return NULL;
    }
    
    // 初始化属性
    strncpy(prop->name, name, MCP_MAX_NAME_LENGTH - 1);
    prop->name[MCP_MAX_NAME_LENGTH - 1] = '\0';
    prop->type = MCP_PROPERTY_TYPE_BOOLEAN;
    prop->has_default_value = has_default;
    prop->has_range = false;
    prop->min_value = 0;
    prop->max_value = 0;
    
    // 设置默认值
    if (has_default) {
        prop->value.bool_val = default_value;
    }
    
    return prop;
}

/**
 * 创建整数类型属性
 */
mcp_property_t* mcp_property_create_integer(const char* name, int default_value, bool has_default,
                                           bool has_range, int min_value, int max_value) {
    // 检查参数有效性
    if (!name || strlen(name) >= MCP_MAX_NAME_LENGTH) {
        return NULL;
    }
    
    // 检查范围有效性
    if (has_range && min_value > max_value) {
        return NULL;
    }
    
    // 检查默认值是否在范围内
    if (has_default && has_range && (default_value < min_value || default_value > max_value)) {
        return NULL;
    }
    
    // 分配内存
    mcp_property_t* prop = malloc(sizeof(mcp_property_t));
    if (!prop) {
        return NULL;
    }
    
    // 初始化属性
    strncpy(prop->name, name, MCP_MAX_NAME_LENGTH - 1);
    prop->name[MCP_MAX_NAME_LENGTH - 1] = '\0';
    prop->type = MCP_PROPERTY_TYPE_INTEGER;
    prop->has_default_value = has_default;
    prop->has_range = has_range;
    prop->min_value = min_value;
    prop->max_value = max_value;
    
    // 设置默认值
    if (has_default) {
        prop->value.int_val = default_value;
    }
    
    return prop;
}

/**
 * 创建字符串类型属性
 */
mcp_property_t* mcp_property_create_string(const char* name, const char* default_value, bool has_default) {
    // 检查参数有效性
    if (!name || strlen(name) >= MCP_MAX_NAME_LENGTH) {
        return NULL;
    }
    
    // 分配内存
    mcp_property_t* prop = malloc(sizeof(mcp_property_t));
    if (!prop) {
        return NULL;
    }
    
    // 初始化属性
    strncpy(prop->name, name, MCP_MAX_NAME_LENGTH - 1);
    prop->name[MCP_MAX_NAME_LENGTH - 1] = '\0';
    prop->type = MCP_PROPERTY_TYPE_STRING;
    prop->has_default_value = has_default;
    prop->has_range = false;
    prop->min_value = 0;
    prop->max_value = 0;
    prop->value.string_val = NULL;
    
    // 设置默认值
    if (has_default && default_value) {
        prop->value.string_val = mcp_strdup(default_value);
        if (!prop->value.string_val) {
            free(prop);
            return NULL;
        }
    }
    
    return prop;
}

/**
 * 设置布尔属性值
 */
bool mcp_property_set_bool_value(mcp_property_t* prop, bool value) {
    // 检查参数有效性
    if (!prop || prop->type != MCP_PROPERTY_TYPE_BOOLEAN) {
        return false;
    }
    
    // 设置值
    prop->value.bool_val = value;
    return true;
}

/**
 * 设置整数属性值
 */
bool mcp_property_set_int_value(mcp_property_t* prop, int value) {
    // 检查参数有效性
    if (!prop || prop->type != MCP_PROPERTY_TYPE_INTEGER) {
        return false;
    }
    
    // 检查范围
    if (prop->has_range && (value < prop->min_value || value > prop->max_value)) {
        return false;
    }
    
    // 设置值
    prop->value.int_val = value;
    return true;
}

/**
 * 设置字符串属性值
 */
bool mcp_property_set_string_value(mcp_property_t* prop, const char* value) {
    // 检查参数有效性
    if (!prop || prop->type != MCP_PROPERTY_TYPE_STRING || !value) {
        return false;
    }
    
    // 释放旧值
    if (prop->value.string_val) {
        free(prop->value.string_val);
    }
    
    // 设置新值
    prop->value.string_val = mcp_strdup(value);
    return prop->value.string_val != NULL;
}

/**
 * 获取布尔属性值
 */
bool mcp_property_get_bool_value(const mcp_property_t* prop) {
    if (!prop || prop->type != MCP_PROPERTY_TYPE_BOOLEAN) {
        return false;
    }
    return prop->value.bool_val;
}

/**
 * 获取整数属性值
 */
int mcp_property_get_int_value(const mcp_property_t* prop) {
    if (!prop || prop->type != MCP_PROPERTY_TYPE_INTEGER) {
        return 0;
    }
    return prop->value.int_val;
}

/**
 * 获取字符串属性值
 */
const char* mcp_property_get_string_value(const mcp_property_t* prop) {
    if (!prop || prop->type != MCP_PROPERTY_TYPE_STRING) {
        return NULL;
    }
    return prop->value.string_val;
}

/**
 * 将属性转换为JSON字符串
 */
char* mcp_property_to_json(const mcp_property_t* prop) {
    if (!prop) {
        return NULL;
    }
    
    char* json = malloc(1024);  // 分配足够的空间
    if (!json) {
        return NULL;
    }
    
    // 根据属性类型生成JSON
    switch (prop->type) {
        case MCP_PROPERTY_TYPE_BOOLEAN:
            snprintf(json, 1024, 
                "{\n"
                "  \"type\": \"boolean\",\n"
                "  \"description\": \"%s\"%s%s\n"
                "}",
                prop->name,
                prop->has_default_value ? ",\n  \"default\": " : "",
                prop->has_default_value ? (prop->value.bool_val ? "true" : "false") : ""
            );
            break;
            
        case MCP_PROPERTY_TYPE_INTEGER:
            snprintf(json, 1024,
                "{\n"
                "  \"type\": \"integer\",\n"
                "  \"description\": \"%s\"%s%s%s%s%s%s\n"
                "}",
                prop->name,
                prop->has_default_value ? ",\n  \"default\": " : "",
                prop->has_default_value ? mcp_itoa(prop->value.int_val) : "",
                prop->has_range ? ",\n  \"minimum\": " : "",
                prop->has_range ? mcp_itoa(prop->min_value) : "",
                prop->has_range ? ",\n  \"maximum\": " : "",
                prop->has_range ? mcp_itoa(prop->max_value) : ""
            );
            break;
            
        case MCP_PROPERTY_TYPE_STRING:
            snprintf(json, 1024,
                "{\n"
                "  \"type\": \"string\",\n"
                "  \"description\": \"%s\"%s%s%s\n"
                "}",
                prop->name,
                prop->has_default_value && prop->value.string_val ? ",\n  \"default\": \"" : "",
                prop->has_default_value && prop->value.string_val ? prop->value.string_val : "",
                prop->has_default_value && prop->value.string_val ? "\"" : ""
            );
            break;
            
        default:
            free(json);
            return NULL;
    }
    
    return json;
}

/**
 * 销毁属性并释放内存
 */
void mcp_property_destroy(mcp_property_t* prop) {
    if (!prop) {
        return;
    }
    
    // 释放字符串值
    if (prop->type == MCP_PROPERTY_TYPE_STRING && prop->value.string_val) {
        free(prop->value.string_val);
    }
    
    // 释放属性本身
    free(prop);
}

/**
 * 创建属性列表
 */
mcp_property_list_t* mcp_property_list_create(void) {
    mcp_property_list_t* list = malloc(sizeof(mcp_property_list_t));
    if (!list) {
        return NULL;
    }
    
    list->count = 0;
    return list;
}

/**
 * 销毁属性列表并释放内存
 */
void mcp_property_list_destroy(mcp_property_list_t* list) {
    if (!list) {
        return;
    }
    
    // 销毁所有属性
    for (size_t i = 0; i < list->count; i++) {
        if (list->properties[i].type == MCP_PROPERTY_TYPE_STRING && 
            list->properties[i].value.string_val) {
            free(list->properties[i].value.string_val);
        }
    }
    
    // 释放列表本身
    free(list);
}

/**
 * 向属性列表添加属性
 */
bool mcp_property_list_add(mcp_property_list_t* list, const mcp_property_t* prop) {
    // 检查参数有效性
    if (!list || !prop || list->count >= MCP_MAX_PROPERTIES) {
        return false;
    }
    
    // 检查是否已存在同名属性
    for (size_t i = 0; i < list->count; i++) {
        if (strcmp(list->properties[i].name, prop->name) == 0) {
            return false;  // 属性名已存在
        }
    }
    
    // 复制属性
    mcp_property_t* dest = &list->properties[list->count];
    memcpy(dest, prop, sizeof(mcp_property_t));
    
    // 如果是字符串类型，需要复制字符串值
    if (prop->type == MCP_PROPERTY_TYPE_STRING && prop->value.string_val) {
        dest->value.string_val = mcp_strdup(prop->value.string_val);
        if (!dest->value.string_val) {
            return false;
        }
    }
    
    list->count++;
    return true;
}

/**
 * 在属性列表中查找属性（只读）
 */
const mcp_property_t* mcp_property_list_find(const mcp_property_list_t* list, const char* name) {
    if (!list || !name) {
        return NULL;
    }
    
    // 遍历查找
    for (size_t i = 0; i < list->count; i++) {
        if (strcmp(list->properties[i].name, name) == 0) {
            return &list->properties[i];
        }
    }
    
    return NULL;
}

/**
 * 在属性列表中查找属性（可修改）
 */
mcp_property_t* mcp_property_list_find_mutable(mcp_property_list_t* list, const char* name) {
    if (!list || !name) {
        return NULL;
    }
    
    // 遍历查找
    for (size_t i = 0; i < list->count; i++) {
        if (strcmp(list->properties[i].name, name) == 0) {
            return &list->properties[i];
        }
    }
    
    return NULL;
}

/**
 * 将属性列表转换为JSON字符串
 */
char* mcp_property_list_to_json(const mcp_property_list_t* list) {
    if (!list) {
        return NULL;
    }
    
    // 分配足够的空间
    char* json = malloc(4096);
    if (!json) {
        return NULL;
    }
    
    strcpy(json, "{\n  \"properties\": {\n");
    
    // 添加每个属性
    for (size_t i = 0; i < list->count; i++) {
        char* prop_json = mcp_property_to_json(&list->properties[i]);
        if (prop_json) {
            strcat(json, "    \"");
            strcat(json, list->properties[i].name);
            strcat(json, "\": ");
            strcat(json, prop_json);
            if (i < list->count - 1) {
                strcat(json, ",");
            }
            strcat(json, "\n");
            free(prop_json);
        }
    }
    
    strcat(json, "  }\n}");
    return json;
}

/**
 * 获取属性列表中必需属性的JSON字符串
 */
char* mcp_property_list_get_required_json(const mcp_property_list_t* list) {
    if (!list) {
        return NULL;
    }
    
    // 分配足够的空间
    char* json = malloc(1024);
    if (!json) {
        return NULL;
    }
    
    strcpy(json, "[");
    bool first = true;
    
    // 添加没有默认值的属性（必需属性）
    for (size_t i = 0; i < list->count; i++) {
        if (!list->properties[i].has_default_value) {
            if (!first) {
                strcat(json, ", ");
            }
            strcat(json, "\"");
            strcat(json, list->properties[i].name);
            strcat(json, "\"");
            first = false;
        }
    }
    
    strcat(json, "]");
    return json;
}