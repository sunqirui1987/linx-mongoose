/*
 * MCP计算器服务器示例
 * 
 * 这个示例展示了如何使用MCP C SDK创建一个简单的计算器服务器，
 * 支持基本的数学运算：加法、减法、乘法、除法和幂运算。
 * 
 * 编译命令：
 * gcc -o calculator_server calculator_server.c ../../*.c -lcjson -lm
 * 
 * 运行：
 * ./calculator_server
 */

#include "../../mcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// 全局服务器实例
static mcp_server_t* g_server = NULL;

// 消息发送回调函数
void send_message(const char* message) {
    printf("SEND: %s\n", message);
    fflush(stdout);
}

// 加法工具回调
mcp_return_value_t add_callback(const mcp_property_list_t* properties) {
    mcp_return_value_t result;
    result.type = MCP_RETURN_TYPE_STRING;
    
    if (!properties || properties->count < 2) {
        result.string_val = mcp_strdup("Error: Addition requires two numbers (a and b)");
        return result;
    }
    
    const mcp_property_t* a_prop = mcp_property_list_find(properties, "a");
    const mcp_property_t* b_prop = mcp_property_list_find(properties, "b");
    
    if (!a_prop || !b_prop || 
        a_prop->type != MCP_PROPERTY_TYPE_INTEGER || 
        b_prop->type != MCP_PROPERTY_TYPE_INTEGER) {
        result.string_val = mcp_strdup("Error: Both parameters must be integers");
        return result;
    }
    
    int a = mcp_property_get_int_value(a_prop);
    int b = mcp_property_get_int_value(b_prop);
    int sum = a + b;
    
    char* response = malloc(128);
    snprintf(response, 128, "Result: %d + %d = %d", a, b, sum);
    result.string_val = response;
    
    return result;
}

// 减法工具回调
mcp_return_value_t subtract_callback(const mcp_property_list_t* properties) {
    mcp_return_value_t result;
    result.type = MCP_RETURN_TYPE_STRING;
    
    if (!properties || properties->count < 2) {
        result.string_val = mcp_strdup("Error: Subtraction requires two numbers (a and b)");
        return result;
    }
    
    const mcp_property_t* a_prop = mcp_property_list_find(properties, "a");
    const mcp_property_t* b_prop = mcp_property_list_find(properties, "b");
    
    if (!a_prop || !b_prop || 
        a_prop->type != MCP_PROPERTY_TYPE_INTEGER || 
        b_prop->type != MCP_PROPERTY_TYPE_INTEGER) {
        result.string_val = mcp_strdup("Error: Both parameters must be integers");
        return result;
    }
    
    int a = mcp_property_get_int_value(a_prop);
    int b = mcp_property_get_int_value(b_prop);
    int diff = a - b;
    
    char* response = malloc(128);
    snprintf(response, 128, "Result: %d - %d = %d", a, b, diff);
    result.string_val = response;
    
    return result;
}

// 乘法工具回调
mcp_return_value_t multiply_callback(const mcp_property_list_t* properties) {
    mcp_return_value_t result;
    result.type = MCP_RETURN_TYPE_STRING;
    
    if (!properties || properties->count < 2) {
        result.string_val = mcp_strdup("Error: Multiplication requires two numbers (a and b)");
        return result;
    }
    
    const mcp_property_t* a_prop = mcp_property_list_find(properties, "a");
    const mcp_property_t* b_prop = mcp_property_list_find(properties, "b");
    
    if (!a_prop || !b_prop || 
        a_prop->type != MCP_PROPERTY_TYPE_INTEGER || 
        b_prop->type != MCP_PROPERTY_TYPE_INTEGER) {
        result.string_val = mcp_strdup("Error: Both parameters must be integers");
        return result;
    }
    
    int a = mcp_property_get_int_value(a_prop);
    int b = mcp_property_get_int_value(b_prop);
    int product = a * b;
    
    char* response = malloc(128);
    snprintf(response, 128, "Result: %d × %d = %d", a, b, product);
    result.string_val = response;
    
    return result;
}

// 除法工具回调
mcp_return_value_t divide_callback(const mcp_property_list_t* properties) {
    mcp_return_value_t result;
    result.type = MCP_RETURN_TYPE_STRING;
    
    if (!properties || properties->count < 2) {
        result.string_val = mcp_strdup("Error: Division requires two numbers (a and b)");
        return result;
    }
    
    const mcp_property_t* a_prop = mcp_property_list_find(properties, "a");
    const mcp_property_t* b_prop = mcp_property_list_find(properties, "b");
    
    if (!a_prop || !b_prop || 
        a_prop->type != MCP_PROPERTY_TYPE_INTEGER || 
        b_prop->type != MCP_PROPERTY_TYPE_INTEGER) {
        result.string_val = mcp_strdup("Error: Both parameters must be integers");
        return result;
    }
    
    int a = mcp_property_get_int_value(a_prop);
    int b = mcp_property_get_int_value(b_prop);
    
    if (b == 0) {
        result.string_val = mcp_strdup("Error: Division by zero is not allowed");
        return result;
    }
    
    double quotient = (double)a / (double)b;
    
    char* response = malloc(128);
    snprintf(response, 128, "Result: %d ÷ %d = %.2f", a, b, quotient);
    result.string_val = response;
    
    return result;
}

// 幂运算工具回调
mcp_return_value_t power_callback(const mcp_property_list_t* properties) {
    mcp_return_value_t result;
    result.type = MCP_RETURN_TYPE_STRING;
    
    if (!properties || properties->count < 2) {
        result.string_val = mcp_strdup("Error: Power operation requires base and exponent");
        return result;
    }
    
    const mcp_property_t* base_prop = mcp_property_list_find(properties, "base");
    const mcp_property_t* exp_prop = mcp_property_list_find(properties, "exponent");
    
    if (!base_prop || !exp_prop || 
        base_prop->type != MCP_PROPERTY_TYPE_INTEGER || 
        exp_prop->type != MCP_PROPERTY_TYPE_INTEGER) {
        result.string_val = mcp_strdup("Error: Both base and exponent must be integers");
        return result;
    }
    
    int base = mcp_property_get_int_value(base_prop);
    int exponent = mcp_property_get_int_value(exp_prop);
    
    double power_result = pow((double)base, (double)exponent);
    
    char* response = malloc(128);
    snprintf(response, 128, "Result: %d^%d = %.2f", base, exponent, power_result);
    result.string_val = response;
    
    return result;
}

// 阶乘工具回调
mcp_return_value_t factorial_callback(const mcp_property_list_t* properties) {
    mcp_return_value_t result;
    result.type = MCP_RETURN_TYPE_STRING;
    
    if (!properties || properties->count < 1) {
        result.string_val = mcp_strdup("Error: Factorial requires one number (n)");
        return result;
    }
    
    const mcp_property_t* n_prop = mcp_property_list_find(properties, "n");
    
    if (!n_prop || n_prop->type != MCP_PROPERTY_TYPE_INTEGER) {
        result.string_val = mcp_strdup("Error: Parameter must be an integer");
        return result;
    }
    
    int n = mcp_property_get_int_value(n_prop);
    
    if (n < 0) {
        result.string_val = mcp_strdup("Error: Factorial is not defined for negative numbers");
        return result;
    }
    
    if (n > 20) {
        result.string_val = mcp_strdup("Error: Factorial calculation limited to n <= 20");
        return result;
    }
    
    long long factorial = 1;
    for (int i = 1; i <= n; i++) {
        factorial *= i;
    }
    
    char* response = malloc(128);
    snprintf(response, 128, "Result: %d! = %lld", n, factorial);
    result.string_val = response;
    
    return result;
}

// 初始化计算器服务器
bool init_calculator_server() {
    // 创建服务器
    g_server = mcp_server_create("Calculator Server", "1.0.0");
    if (!g_server) {
        fprintf(stderr, "Failed to create server\n");
        return false;
    }
    
    // 设置消息发送回调
    mcp_server_set_send_callback(send_message);
    
    // 创建加法工具
    mcp_property_list_t* add_props = mcp_property_list_create();
    mcp_property_t* a_prop = mcp_property_create_integer("a", 0, false, false, 0, 0);
    mcp_property_t* b_prop = mcp_property_create_integer("b", 0, false, false, 0, 0);
    mcp_property_list_add(add_props, a_prop);
    mcp_property_list_add(add_props, b_prop);
    
    if (!mcp_server_add_simple_tool(g_server, "add", "Add two integers", add_props, add_callback)) {
        fprintf(stderr, "Failed to add addition tool\n");
        return false;
    }
    
    // 创建减法工具
    mcp_property_list_t* sub_props = mcp_property_list_create();
    mcp_property_t* sub_a_prop = mcp_property_create_integer("a", 0, false, false, 0, 0);
    mcp_property_t* sub_b_prop = mcp_property_create_integer("b", 0, false, false, 0, 0);
    mcp_property_list_add(sub_props, sub_a_prop);
    mcp_property_list_add(sub_props, sub_b_prop);
    
    if (!mcp_server_add_simple_tool(g_server, "subtract", "Subtract two integers", sub_props, subtract_callback)) {
        fprintf(stderr, "Failed to add subtraction tool\n");
        return false;
    }
    
    // 创建乘法工具
    mcp_property_list_t* mul_props = mcp_property_list_create();
    mcp_property_t* mul_a_prop = mcp_property_create_integer("a", 0, false, false, 0, 0);
    mcp_property_t* mul_b_prop = mcp_property_create_integer("b", 0, false, false, 0, 0);
    mcp_property_list_add(mul_props, mul_a_prop);
    mcp_property_list_add(mul_props, mul_b_prop);
    
    if (!mcp_server_add_simple_tool(g_server, "multiply", "Multiply two integers", mul_props, multiply_callback)) {
        fprintf(stderr, "Failed to add multiplication tool\n");
        return false;
    }
    
    // 创建除法工具
    mcp_property_list_t* div_props = mcp_property_list_create();
    mcp_property_t* div_a_prop = mcp_property_create_integer("a", 0, false, false, 0, 0);
    mcp_property_t* div_b_prop = mcp_property_create_integer("b", 0, false, false, 0, 0);
    mcp_property_list_add(div_props, div_a_prop);
    mcp_property_list_add(div_props, div_b_prop);
    
    if (!mcp_server_add_simple_tool(g_server, "divide", "Divide two integers", div_props, divide_callback)) {
        fprintf(stderr, "Failed to add division tool\n");
        return false;
    }
    
    // 创建幂运算工具
    mcp_property_list_t* pow_props = mcp_property_list_create();
    mcp_property_t* base_prop = mcp_property_create_integer("base", 0, false, false, 0, 0);
    mcp_property_t* exp_prop = mcp_property_create_integer("exponent", 0, false, false, 0, 0);
    mcp_property_list_add(pow_props, base_prop);
    mcp_property_list_add(pow_props, exp_prop);
    
    if (!mcp_server_add_simple_tool(g_server, "power", "Calculate base^exponent", pow_props, power_callback)) {
        fprintf(stderr, "Failed to add power tool\n");
        return false;
    }
    
    // 创建阶乘工具
    mcp_property_list_t* fact_props = mcp_property_list_create();
    mcp_property_t* n_prop = mcp_property_create_integer("n", 0, false, true, 0, 20);
    mcp_property_list_add(fact_props, n_prop);
    
    if (!mcp_server_add_simple_tool(g_server, "factorial", "Calculate n! (factorial)", fact_props, factorial_callback)) {
        fprintf(stderr, "Failed to add factorial tool\n");
        return false;
    }
    
    // 清理属性对象（服务器会复制它们）
    mcp_property_destroy(a_prop);
    mcp_property_destroy(b_prop);
    mcp_property_destroy(sub_a_prop);
    mcp_property_destroy(sub_b_prop);
    mcp_property_destroy(mul_a_prop);
    mcp_property_destroy(mul_b_prop);
    mcp_property_destroy(div_a_prop);
    mcp_property_destroy(div_b_prop);
    mcp_property_destroy(base_prop);
    mcp_property_destroy(exp_prop);
    mcp_property_destroy(n_prop);
    
    mcp_property_list_destroy(add_props);
    mcp_property_list_destroy(sub_props);
    mcp_property_list_destroy(mul_props);
    mcp_property_list_destroy(div_props);
    mcp_property_list_destroy(pow_props);
    mcp_property_list_destroy(fact_props);
    
    printf("Calculator server initialized with %zu tools\n", g_server->tool_count);
    return true;
}

// 清理服务器
void cleanup_calculator_server() {
    if (g_server) {
        mcp_server_destroy(g_server);
        g_server = NULL;
    }
}

// 处理输入消息
void process_message(const char* message) {
    if (!g_server || !message) {
        return;
    }
    
    printf("RECV: %s\n", message);
    mcp_server_parse_message(g_server, message);
}

// 主函数
int main() {
    printf("=== MCP Calculator Server Example ===\n");
    printf("This server provides basic mathematical operations.\n");
    printf("Available tools: add, subtract, multiply, divide, power, factorial\n");
    printf("Send JSON-RPC messages to interact with the server.\n");
    printf("Type 'quit' to exit.\n\n");
    
    // 初始化服务器
    if (!init_calculator_server()) {
        fprintf(stderr, "Failed to initialize calculator server\n");
        return 1;
    }
    
    // 示例消息
    printf("Example messages:\n");
    printf("Initialize: {\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"initialize\",\"params\":{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{}}}\n");
    printf("List tools: {\"jsonrpc\":\"2.0\",\"id\":2,\"method\":\"tools/list\",\"params\":{}}\n");
    printf("Add 5+3: {\"jsonrpc\":\"2.0\",\"id\":3,\"method\":\"tools/call\",\"params\":{\"name\":\"add\",\"arguments\":{\"a\":5,\"b\":3}}}\n");
    printf("Calculate 2^8: {\"jsonrpc\":\"2.0\",\"id\":4,\"method\":\"tools/call\",\"params\":{\"name\":\"power\",\"arguments\":{\"base\":2,\"exponent\":8}}}\n");
    printf("Factorial 5!: {\"jsonrpc\":\"2.0\",\"id\":5,\"method\":\"tools/call\",\"params\":{\"name\":\"factorial\",\"arguments\":{\"n\":5}}}\n\n");
    
    // 主循环
    char input[1024];
    while (1) {
        printf("> ");
        fflush(stdout);
        
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }
        
        // 移除换行符
        size_t len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
        }
        
        // 检查退出命令
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            break;
        }
        
        // 处理消息
        if (strlen(input) > 0) {
            process_message(input);
        }
    }
    
    printf("\nShutting down calculator server...\n");
    cleanup_calculator_server();
    
    return 0;
}