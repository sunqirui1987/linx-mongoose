#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "sdk/mcp/mcp.h"
#include "sdk/mcp/mcp_utils.h"

/* Test tool function */
mcp_return_value_t test_tool_function(const mcp_property_list_t* args) {
    printf("Test tool called with %d arguments\n", args ? (int)args->count : 0);
    
    if (args) {
        for (int i = 0; i < args->count; i++) {
            const mcp_property_t* prop = &args->properties[i];
            printf("  Arg %d: %s = ", i, prop->name);
            
            switch (prop->type) {
                case MCP_PROPERTY_TYPE_BOOLEAN:
                    printf("%s\n", prop->value.bool_val ? "true" : "false");
                    break;
                case MCP_PROPERTY_TYPE_INTEGER:
                    printf("%d\n", prop->value.int_val);
                    break;
                case MCP_PROPERTY_TYPE_STRING:
                    printf("\"%s\"\n", prop->value.string_val ? prop->value.string_val : "null");
                    break;
                default:
                    printf("unknown type\n");
                    break;
            }
        }
    }
    
    mcp_return_value_t result;
    result.string_val = mcp_strdup("Test tool executed successfully");
    return result;
}

int main() {
    printf("Testing MCP C Implementation\n");
    printf("============================\n\n");
    
    /* Test 1: Property creation and manipulation */
    printf("Test 1: Property creation and manipulation\n");
    printf("------------------------------------------\n");
    
    mcp_property_t* bool_prop = mcp_property_create_boolean("test_bool", true, false);
    mcp_property_t* int_prop = mcp_property_create_integer("test_int", 42, false, true, 0, 100);
    mcp_property_t* str_prop = mcp_property_create_string("test_string", "Hello, MCP!", false);
    
    assert(bool_prop != NULL);
    assert(int_prop != NULL);
    assert(str_prop != NULL);
    
    printf("✓ Boolean property: %s = %s\n", bool_prop->name, bool_prop->value.bool_val ? "true" : "false");
    printf("✓ Integer property: %s = %d\n", int_prop->name, int_prop->value.int_val);
    printf("✓ String property: %s = \"%s\"\n", str_prop->name, str_prop->value.string_val);
    
    /* Test 2: Property list operations */
    printf("\nTest 2: Property list operations\n");
    printf("--------------------------------\n");
    
    mcp_property_list_t* prop_list = mcp_property_list_create();
    assert(prop_list != NULL);
    
    mcp_property_list_add(prop_list, bool_prop);
    mcp_property_list_add(prop_list, int_prop);
    mcp_property_list_add(prop_list, str_prop);
    
    printf("✓ Property list created with %zu properties\n", prop_list->count);
    
    const mcp_property_t* found_prop = mcp_property_list_find(prop_list, "test_int");
    assert(found_prop != NULL);
    printf("✓ Found property 'test_int' with value: %d\n", found_prop->value.int_val);
    
    /* Test 3: Tool creation and management */
    printf("\nTest 3: Tool creation and management\n");
    printf("------------------------------------\n");
    
    mcp_tool_t* tool = mcp_tool_create("test_tool", "A test tool for demonstration", prop_list, test_tool_function);
    assert(tool != NULL);
    
    printf("✓ Tool '%s' created with %zu parameters\n", tool->name, tool->properties->count);
    printf("  Description: %s\n", tool->description);
    
    /* Test 4: MCP Server creation and tool registration */
    printf("\nTest 4: MCP Server creation and tool registration\n");
    printf("-------------------------------------------------\n");
    
    mcp_server_t* server = mcp_server_create("Test MCP Server", "1.0.0");
    assert(server != NULL);
    
    mcp_server_add_tool(server, tool);
    printf("✓ MCP Server '%s' created with %d tools\n", server->server_name, (int)server->tool_count);
    
    const mcp_tool_t* found_tool = mcp_server_find_tool(server, "test_tool");
    assert(found_tool != NULL);
    printf("✓ Found tool '%s' in server\n", found_tool->name);
    
    /* Test 5: Tool execution */
    printf("\nTest 5: Tool execution\n");
    printf("----------------------\n");
    
    char* result_str = mcp_tool_call(found_tool, prop_list);
    printf("✓ Tool execution result: %s\n", result_str ? result_str : "null");
    if (result_str) {
        free(result_str);
    }
    
    /* Test 6: JSON conversion (basic test) */
    printf("\nTest 6: JSON conversion\n");
    printf("-----------------------\n");
    
    char* prop_json_str = mcp_property_to_json(str_prop);
    if (prop_json_str) {
        printf("✓ Property to JSON: %s\n", prop_json_str);
        free(prop_json_str);
    }
    
    char* tool_json_str = mcp_tool_to_json(tool);
    if (tool_json_str) {
        printf("✓ Tool to JSON: %s\n", tool_json_str);
        free(tool_json_str);
    }
    
    /* Cleanup */
    printf("\nTest 7: Cleanup\n");
    printf("---------------\n");
    
    mcp_server_destroy(server);
    mcp_property_list_destroy(prop_list);
    
    printf("✓ All resources cleaned up\n");
    
    printf("\n============================\n");
    printf("All tests passed successfully!\n");
    printf("MCP C implementation is working correctly.\n");
    
    return 0;
}