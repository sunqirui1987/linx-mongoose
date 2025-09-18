#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "sdk/mcp/mcp.h"

/* Simple test tool function */
mcp_return_value_t simple_test_tool(const mcp_property_list_t* args) {
    printf("Simple test tool called\n");
    return mcp_return_string("Simple test completed");
}

int main() {
    printf("Testing Tool Call\n");
    printf("=================\n\n");
    
    /* Create a simple tool */
    mcp_property_list_t* tool_props = mcp_property_list_create();
    assert(tool_props != NULL);
    
    mcp_tool_t* tool = mcp_tool_create("simple_tool", "A simple test tool", 
                                      tool_props, simple_test_tool);
    assert(tool != NULL);
    printf("✓ Tool created\n");
    
    /* Create separate properties for calling */
    mcp_property_list_t* call_props = mcp_property_list_create();
    assert(call_props != NULL);
    
    /* Call the tool directly */
    printf("Calling tool directly...\n");
    char* result = mcp_tool_call(tool, call_props);
    
    if (result) {
        printf("✓ Tool call result: %s\n", result);
        free(result);
    } else {
        printf("✗ Tool call failed\n");
    }
    
    /* Cleanup */
    mcp_property_list_destroy(call_props);
    mcp_tool_destroy(tool); /* This will destroy tool_props */
    
    printf("✓ Cleanup completed\n");
    
    return 0;
}