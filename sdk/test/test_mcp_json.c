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

/* Test message sending callback */
void test_send_message_callback(const char* message) {
    printf("Sending message: %s\n", message);
}

int main() {
    printf("Testing MCP JSON Message Processing\n");
    printf("===================================\n\n");
    
    /* Test 1: Create server and add tools */
    printf("Test 1: Server setup\n");
    printf("--------------------\n");
    
    mcp_server_t* server = mcp_server_create("JSON Test Server", "1.0.0");
    assert(server != NULL);
    
    /* Create a simple tool */
    mcp_property_list_t* tool_props = mcp_property_list_create();
    bool tool_added = mcp_server_add_simple_tool(server, "simple_tool", 
                                                 "A simple test tool", 
                                                 tool_props, simple_test_tool);
    
    if (tool_added) {
        printf("✓ Tool added to server\n");
    } else {
        printf("✗ Failed to add tool to server\n");
        return 1;
    }
    
    /* Test 2: Set message callback */
    printf("\nTest 2: Message callback setup\n");
    printf("------------------------------\n");
    
    mcp_server_set_send_callback(test_send_message_callback);
    printf("✓ Message callback set\n");
    
    /* Test 3: Test tools list JSON generation */
    printf("\nTest 3: Tools list JSON\n");
    printf("-----------------------\n");
    
    char* tools_json = mcp_server_get_tools_list_json(server, NULL, false);
    if (tools_json) {
        printf("✓ Tools list JSON generated:\n%s\n", tools_json);
        free(tools_json);
    } else {
        printf("✗ Failed to generate tools list JSON\n");
    }
    
    /* Test 4: Test initialize message handling */
    printf("\nTest 4: Initialize message handling\n");
    printf("-----------------------------------\n");
    
    const char* init_message = "{"
        "\"jsonrpc\": \"2.0\","
        "\"id\": 1,"
        "\"method\": \"initialize\","
        "\"params\": {"
            "\"protocolVersion\": \"2024-11-05\","
            "\"capabilities\": {"
                "\"tools\": {}"
            "},"
            "\"clientInfo\": {"
                "\"name\": \"test-client\","
                "\"version\": \"1.0.0\""
            "}"
        "}"
    "}";
    
    printf("Processing initialize message...\n");
    mcp_server_parse_message(server, init_message);
    
    /* Test 5: Test tools/list message handling */
    printf("\nTest 5: Tools list message handling\n");
    printf("-----------------------------------\n");
    
    const char* tools_list_message = "{"
        "\"jsonrpc\": \"2.0\","
        "\"id\": 2,"
        "\"method\": \"tools/list\","
        "\"params\": {}"
    "}";
    
    printf("Processing tools/list message...\n");
    mcp_server_parse_message(server, tools_list_message);
    
    /* Test 6: Test tools/call message handling */
    printf("\nTest 6: Tools call message handling\n");
    printf("-----------------------------------\n");
    
    const char* tools_call_message = "{"
        "\"jsonrpc\": \"2.0\","
        "\"id\": 3,"
        "\"method\": \"tools/call\","
        "\"params\": {"
            "\"name\": \"simple_tool\","
            "\"arguments\": {}"
        "}"
    "}";
    
    printf("Processing tools/call message...\n");
    mcp_server_parse_message(server, tools_call_message);
    
    /* Test 7: Test error handling with invalid JSON */
    printf("\nTest 7: Error handling\n");
    printf("----------------------\n");
    
    const char* invalid_message = "{ invalid json }";
    printf("Processing invalid JSON message...\n");
    mcp_server_parse_message(server, invalid_message);
    
    /* Cleanup */
    printf("\nTest 8: Cleanup\n");
    printf("---------------\n");
    
    mcp_server_destroy(server); /* This will destroy tool_props */
    
    printf("✓ All resources cleaned up\n");
    
    printf("\n===================================\n");
    printf("JSON message processing tests completed!\n");
    
    return 0;
}