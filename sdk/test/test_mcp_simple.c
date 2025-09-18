#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "sdk/mcp/mcp.h"

int main() {
    printf("Testing MCP C Implementation - Basic Tests\n");
    printf("==========================================\n\n");
    
    /* Test 1: Property creation */
    printf("Test 1: Property creation\n");
    printf("-------------------------\n");
    
    mcp_property_t* bool_prop = mcp_property_create_boolean("test_bool", true, false);
    mcp_property_t* int_prop = mcp_property_create_integer("test_int", 42, false, true, 0, 100);
    mcp_property_t* str_prop = mcp_property_create_string("test_string", "Hello, MCP!", false);
    
    if (bool_prop && int_prop && str_prop) {
        printf("✓ Properties created successfully\n");
        printf("  Boolean: %s = %s\n", bool_prop->name, bool_prop->value.bool_val ? "true" : "false");
        printf("  Integer: %s = %d\n", int_prop->name, int_prop->value.int_val);
        printf("  String: %s = \"%s\"\n", str_prop->name, str_prop->value.string_val ? str_prop->value.string_val : "null");
    } else {
        printf("✗ Property creation failed\n");
        return 1;
    }
    
    /* Test 2: Property list */
    printf("\nTest 2: Property list\n");
    printf("---------------------\n");
    
    mcp_property_list_t* prop_list = mcp_property_list_create();
    if (prop_list) {
        printf("✓ Property list created\n");
        
        bool added = mcp_property_list_add(prop_list, bool_prop) &&
                    mcp_property_list_add(prop_list, int_prop) &&
                    mcp_property_list_add(prop_list, str_prop);
        
        if (added) {
            printf("✓ Properties added to list (count: %zu)\n", prop_list->count);
        } else {
            printf("✗ Failed to add properties to list\n");
        }
    } else {
        printf("✗ Property list creation failed\n");
        return 1;
    }
    
    /* Test 3: MCP Server creation */
    printf("\nTest 3: MCP Server creation\n");
    printf("---------------------------\n");
    
    mcp_server_t* server = mcp_server_create("Test MCP Server", "1.0.0");
    if (server) {
        printf("✓ MCP Server '%s' created\n", server->server_name);
    } else {
        printf("✗ MCP Server creation failed\n");
        return 1;
    }
    
    /* Test 4: JSON conversion */
    printf("\nTest 4: JSON conversion\n");
    printf("-----------------------\n");
    
    char* prop_json = mcp_property_to_json(str_prop);
    if (prop_json) {
        printf("✓ Property to JSON: %s\n", prop_json);
        free(prop_json);
    } else {
        printf("✗ Property to JSON conversion failed\n");
    }
    
    /* Cleanup */
    printf("\nTest 5: Cleanup\n");
    printf("---------------\n");
    
    mcp_server_destroy(server);
    mcp_property_list_destroy(prop_list);
    mcp_property_destroy(bool_prop);
    mcp_property_destroy(int_prop);
    mcp_property_destroy(str_prop);
    
    printf("✓ All resources cleaned up\n");
    
    printf("\n==========================================\n");
    printf("Basic tests completed successfully!\n");
    
    return 0;
}