#include "test_framework.h"
#include "../mcp_tool.h"
#include "../mcp_types.h"
#include <string.h>
#include <stdlib.h>

// 测试工具回调函数
mcp_return_value_t test_callback_simple(const mcp_property_list_t* properties) {
    mcp_return_value_t result;
    result.type = MCP_RETURN_TYPE_STRING;
    result.string_val = mcp_strdup("test result");
    return result;
}

mcp_return_value_t test_callback_with_params(const mcp_property_list_t* properties) {
    mcp_return_value_t result;
    result.type = MCP_RETURN_TYPE_STRING;
    
    if (properties && properties->count > 0) {
        const mcp_property_t* prop = mcp_property_list_find(properties, "input");
        if (prop && prop->type == MCP_PROPERTY_TYPE_STRING) {
            char* output = malloc(256);
            snprintf(output, 256, "Processed: %s", mcp_property_get_string_value(prop));
            result.string_val = output;
        } else {
            result.string_val = mcp_strdup("No valid input parameter");
        }
    } else {
        result.string_val = mcp_strdup("No parameters provided");
    }
    
    return result;
}

mcp_return_value_t test_callback_integer(const mcp_property_list_t* properties) {
    mcp_return_value_t result;
    result.type = MCP_RETURN_TYPE_INT;
    result.int_val = 42;
    return result;
}

mcp_return_value_t test_callback_boolean(const mcp_property_list_t* properties) {
    mcp_return_value_t result;
    result.type = MCP_RETURN_TYPE_BOOL;
    result.bool_val = true;
    return result;
}

// 测试工具创建和销毁
void test_tool_create_destroy() {
    printf("Testing tool create and destroy...\n");
    
    // 测试简单工具创建
    mcp_tool_t* tool = mcp_tool_create("test_tool", "A test tool", test_callback_simple);
    TEST_ASSERT(tool != NULL, "Tool creation failed");
    TEST_ASSERT(strcmp(tool->name, "test_tool") == 0, "Tool name mismatch");
    TEST_ASSERT(strcmp(tool->description, "A test tool") == 0, "Tool description mismatch");
    TEST_ASSERT(tool->callback == test_callback_simple, "Tool callback mismatch");
    TEST_ASSERT(tool->property_list != NULL, "Tool property list should be initialized");
    TEST_ASSERT(tool->property_list->count == 0, "Tool should start with no properties");
    
    mcp_tool_destroy(tool);
    
    // 测试NULL参数
    tool = mcp_tool_create(NULL, "description", test_callback_simple);
    TEST_ASSERT(tool == NULL, "Tool creation with NULL name should fail");
    
    tool = mcp_tool_create("name", NULL, test_callback_simple);
    TEST_ASSERT(tool == NULL, "Tool creation with NULL description should fail");
    
    tool = mcp_tool_create("name", "description", NULL);
    TEST_ASSERT(tool == NULL, "Tool creation with NULL callback should fail");
    
    // 测试空字符串参数
    tool = mcp_tool_create("", "description", test_callback_simple);
    TEST_ASSERT(tool == NULL, "Tool creation with empty name should fail");
    
    tool = mcp_tool_create("name", "", test_callback_simple);
    TEST_ASSERT(tool != NULL, "Tool creation with empty description should succeed");
    mcp_tool_destroy(tool);
}

// 测试工具属性管理
void test_tool_property_management() {
    printf("Testing tool property management...\n");
    
    mcp_tool_t* tool = mcp_tool_create("test_tool", "Test tool", test_callback_with_params);
    TEST_ASSERT(tool != NULL, "Tool creation failed");
    
    // 添加字符串属性
    mcp_property_t* str_prop = mcp_property_create_string("input", NULL, false);
    bool result = mcp_tool_add_property(tool, str_prop);
    TEST_ASSERT(result == true, "Adding string property failed");
    TEST_ASSERT(tool->property_list->count == 1, "Tool should have 1 property");
    
    // 添加整数属性
    mcp_property_t* int_prop = mcp_property_create_integer("count", 10, true, true, 1, 100);
    result = mcp_tool_add_property(tool, int_prop);
    TEST_ASSERT(result == true, "Adding integer property failed");
    TEST_ASSERT(tool->property_list->count == 2, "Tool should have 2 properties");
    
    // 添加布尔属性
    mcp_property_t* bool_prop = mcp_property_create_boolean("enabled", true, true);
    result = mcp_tool_add_property(tool, bool_prop);
    TEST_ASSERT(result == true, "Adding boolean property failed");
    TEST_ASSERT(tool->property_list->count == 3, "Tool should have 3 properties");
    
    // 查找属性
    const mcp_property_t* found = mcp_tool_find_property(tool, "input");
    TEST_ASSERT(found != NULL, "Input property not found");
    TEST_ASSERT(found->type == MCP_PROPERTY_TYPE_STRING, "Input property type mismatch");
    
    found = mcp_tool_find_property(tool, "count");
    TEST_ASSERT(found != NULL, "Count property not found");
    TEST_ASSERT(found->type == MCP_PROPERTY_TYPE_INTEGER, "Count property type mismatch");
    
    found = mcp_tool_find_property(tool, "enabled");
    TEST_ASSERT(found != NULL, "Enabled property not found");
    TEST_ASSERT(found->type == MCP_PROPERTY_TYPE_BOOLEAN, "Enabled property type mismatch");
    
    found = mcp_tool_find_property(tool, "nonexistent");
    TEST_ASSERT(found == NULL, "Nonexistent property should not be found");
    
    // 测试重复属性名
    mcp_property_t* duplicate_prop = mcp_property_create_string("input", "default", true);
    result = mcp_tool_add_property(tool, duplicate_prop);
    TEST_ASSERT(result == false, "Adding duplicate property should fail");
    TEST_ASSERT(tool->property_list->count == 3, "Tool should still have 3 properties");
    
    // 清理
    mcp_property_destroy(str_prop);
    mcp_property_destroy(int_prop);
    mcp_property_destroy(bool_prop);
    mcp_property_destroy(duplicate_prop);
    mcp_tool_destroy(tool);
}

// 测试工具执行
void test_tool_execution() {
    printf("Testing tool execution...\n");
    
    // 测试简单工具执行
    mcp_tool_t* simple_tool = mcp_tool_create("simple", "Simple tool", test_callback_simple);
    TEST_ASSERT(simple_tool != NULL, "Simple tool creation failed");
    
    mcp_return_value_t result = mcp_tool_execute(simple_tool, NULL);
    TEST_ASSERT(result.type == MCP_RETURN_TYPE_STRING, "Simple tool return type mismatch");
    TEST_ASSERT(result.string_val != NULL, "Simple tool return value is NULL");
    TEST_ASSERT(strcmp(result.string_val, "test result") == 0, "Simple tool return value mismatch");
    
    // 清理返回值
    mcp_return_value_cleanup(&result);
    mcp_tool_destroy(simple_tool);
    
    // 测试带参数的工具执行
    mcp_tool_t* param_tool = mcp_tool_create("param_tool", "Tool with parameters", test_callback_with_params);
    TEST_ASSERT(param_tool != NULL, "Parameter tool creation failed");
    
    // 添加输入属性
    mcp_property_t* input_prop = mcp_property_create_string("input", NULL, false);
    mcp_tool_add_property(param_tool, input_prop);
    
    // 创建参数列表
    mcp_property_list_t* params = mcp_property_list_create();
    mcp_property_t* param = mcp_property_create_string("input", "Hello World", true);
    mcp_property_list_add(params, param);
    
    result = mcp_tool_execute(param_tool, params);
    TEST_ASSERT(result.type == MCP_RETURN_TYPE_STRING, "Parameter tool return type mismatch");
    TEST_ASSERT(result.string_val != NULL, "Parameter tool return value is NULL");
    TEST_ASSERT(strstr(result.string_val, "Processed: Hello World") != NULL, "Parameter tool return value mismatch");
    
    // 清理
    mcp_return_value_cleanup(&result);
    mcp_property_destroy(input_prop);
    mcp_property_destroy(param);
    mcp_property_list_destroy(params);
    mcp_tool_destroy(param_tool);
    
    // 测试整数返回值工具
    mcp_tool_t* int_tool = mcp_tool_create("int_tool", "Integer tool", test_callback_integer);
    result = mcp_tool_execute(int_tool, NULL);
    TEST_ASSERT(result.type == MCP_RETURN_TYPE_INT, "Integer tool return type mismatch");
    TEST_ASSERT(result.int_val == 42, "Integer tool return value mismatch");
    
    mcp_tool_destroy(int_tool);
    
    // 测试布尔返回值工具
    mcp_tool_t* bool_tool = mcp_tool_create("bool_tool", "Boolean tool", test_callback_boolean);
    result = mcp_tool_execute(bool_tool, NULL);
    TEST_ASSERT(result.type == MCP_RETURN_TYPE_BOOL, "Boolean tool return type mismatch");
    TEST_ASSERT(result.bool_val == true, "Boolean tool return value mismatch");
    
    mcp_tool_destroy(bool_tool);
}

// 测试工具序列化
void test_tool_serialization() {
    printf("Testing tool serialization...\n");
    
    mcp_tool_t* tool = mcp_tool_create("test_tool", "A test tool for serialization", test_callback_simple);
    TEST_ASSERT(tool != NULL, "Tool creation failed");
    
    // 添加一些属性
    mcp_property_t* str_prop = mcp_property_create_string("name", "default_name", true);
    mcp_tool_add_property(tool, str_prop);
    
    mcp_property_t* int_prop = mcp_property_create_integer("age", 25, true, true, 0, 100);
    mcp_tool_add_property(tool, int_prop);
    
    // 序列化工具
    char* json = mcp_tool_to_json(tool);
    TEST_ASSERT(json != NULL, "Tool serialization failed");
    TEST_ASSERT(strstr(json, "test_tool") != NULL, "Tool name not in JSON");
    TEST_ASSERT(strstr(json, "A test tool for serialization") != NULL, "Tool description not in JSON");
    TEST_ASSERT(strstr(json, "name") != NULL, "Property name not in JSON");
    TEST_ASSERT(strstr(json, "age") != NULL, "Property age not in JSON");
    
    free(json);
    
    // 清理
    mcp_property_destroy(str_prop);
    mcp_property_destroy(int_prop);
    mcp_tool_destroy(tool);
}

// 测试工具验证
void test_tool_validation() {
    printf("Testing tool validation...\n");
    
    mcp_tool_t* tool = mcp_tool_create("validation_tool", "Tool for validation testing", test_callback_with_params);
    TEST_ASSERT(tool != NULL, "Tool creation failed");
    
    // 添加必需属性
    mcp_property_t* required_prop = mcp_property_create_string("required_input", NULL, false);
    mcp_tool_add_property(tool, required_prop);
    
    // 添加可选属性
    mcp_property_t* optional_prop = mcp_property_create_string("optional_input", "default", true);
    mcp_tool_add_property(tool, optional_prop);
    
    // 测试无参数验证
    bool is_valid = mcp_tool_validate_parameters(tool, NULL);
    TEST_ASSERT(!is_valid, "Tool should be invalid without required parameters");
    
    // 测试缺少必需参数
    mcp_property_list_t* incomplete_params = mcp_property_list_create();
    mcp_property_t* optional_param = mcp_property_create_string("optional_input", "test", true);
    mcp_property_list_add(incomplete_params, optional_param);
    
    is_valid = mcp_tool_validate_parameters(tool, incomplete_params);
    TEST_ASSERT(!is_valid, "Tool should be invalid without required parameter");
    
    // 测试完整参数
    mcp_property_t* required_param = mcp_property_create_string("required_input", "test_value", true);
    mcp_property_list_add(incomplete_params, required_param);
    
    is_valid = mcp_tool_validate_parameters(tool, incomplete_params);
    TEST_ASSERT(is_valid, "Tool should be valid with all required parameters");
    
    // 清理
    mcp_property_destroy(required_prop);
    mcp_property_destroy(optional_prop);
    mcp_property_destroy(optional_param);
    mcp_property_destroy(required_param);
    mcp_property_list_destroy(incomplete_params);
    mcp_tool_destroy(tool);
}

// 测试边界条件和错误处理
void test_tool_edge_cases() {
    printf("Testing tool edge cases...\n");
    
    // 测试NULL工具执行
    mcp_return_value_t result = mcp_tool_execute(NULL, NULL);
    TEST_ASSERT(result.type == MCP_RETURN_TYPE_STRING, "NULL tool should return error string");
    TEST_ASSERT(result.string_val != NULL, "NULL tool error message should not be NULL");
    mcp_return_value_cleanup(&result);
    
    // 测试工具名称长度限制
    char long_name[MCP_MAX_NAME_LENGTH + 10];
    memset(long_name, 'A', sizeof(long_name) - 1);
    long_name[sizeof(long_name) - 1] = '\0';
    
    mcp_tool_t* tool = mcp_tool_create(long_name, "description", test_callback_simple);
    TEST_ASSERT(tool == NULL, "Tool creation with overly long name should fail");
    
    // 测试工具描述长度限制
    char long_desc[MCP_MAX_DESCRIPTION_LENGTH + 10];
    memset(long_desc, 'B', sizeof(long_desc) - 1);
    long_desc[sizeof(long_desc) - 1] = '\0';
    
    tool = mcp_tool_create("test", long_desc, test_callback_simple);
    TEST_ASSERT(tool == NULL, "Tool creation with overly long description should fail");
    
    // 测试属性数量限制
    tool = mcp_tool_create("test", "description", test_callback_simple);
    TEST_ASSERT(tool != NULL, "Tool creation failed");
    
    // 尝试添加超过最大数量的属性
    for (int i = 0; i < MCP_MAX_PROPERTIES + 5; i++) {
        char prop_name[32];
        snprintf(prop_name, sizeof(prop_name), "prop_%d", i);
        mcp_property_t* prop = mcp_property_create_string(prop_name, NULL, false);
        bool added = mcp_tool_add_property(tool, prop);
        
        if (i < MCP_MAX_PROPERTIES) {
            TEST_ASSERT(added == true, "Property addition should succeed within limit");
        } else {
            TEST_ASSERT(added == false, "Property addition should fail beyond limit");
        }
        
        mcp_property_destroy(prop);
    }
    
    TEST_ASSERT(tool->property_list->count == MCP_MAX_PROPERTIES, "Tool should have maximum properties");
    
    mcp_tool_destroy(tool);
}

// 运行所有工具测试
void run_tool_tests() {
    printf("\n=== Running Tool Tests ===\n");
    
    test_tool_create_destroy();
    test_tool_property_management();
    test_tool_execution();
    test_tool_serialization();
    test_tool_validation();
    test_tool_edge_cases();
    
    printf("=== Tool Tests Complete ===\n\n");
}

#ifdef STANDALONE_TEST
int main() {
    test_init();
    run_tool_tests();
    test_summary();
    return test_stats.failed > 0 ? 1 : 0;
}
#endif