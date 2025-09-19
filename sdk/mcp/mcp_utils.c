/**
 * @file mcp_utils.c
 * @brief MCP工具函数库实现文件
 * 
 * 实现MCP协议中常用的工具函数，包括Base64编码、图像处理、
 * 字符串操作和JSON处理等功能。
 */
#include "mcp_utils.h"
#include <stdlib.h>        // 内存管理函数
#include <string.h>        // 字符串操作函数
#include <stdio.h>         // 标准输入输出函数

/**
 * @brief Base64编码字符表
 * 
 * 标准Base64编码使用的64个字符，包括大小写字母、数字和特殊字符
 */
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * @brief 将二进制数据编码为Base64字符串
 * @param data 要编码的二进制数据
 * @param data_len 数据长度
 * @return 编码后的Base64字符串，失败返回NULL
 */
char* mcp_base64_encode(const char* data, size_t data_len) {
    // 参数验证
    if (!data) {
        return NULL;
    }
    
    // 处理空数据的情况
    if (data_len == 0) {
        char* empty_result = malloc(1);
        if (empty_result) {
            empty_result[0] = '\0';
        }
        return empty_result;
    }
    
    // 计算编码后的长度：每3个字节编码为4个字符
    size_t encoded_len = 4 * ((data_len + 2) / 3);
    char* encoded = malloc(encoded_len + 1);  // +1为字符串结束符
    if (!encoded) {
        return NULL;  // 内存分配失败
    }
    
    size_t i, j;
    // 按3字节为一组进行编码
    for (i = 0, j = 0; i < data_len;) {
        // 读取3个字节（不足时用0填充）
        uint32_t octet_a = i < data_len ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < data_len ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < data_len ? (unsigned char)data[i++] : 0;
        
        // 将3个字节合并为24位整数
        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
        
        // 将24位分为4组，每组6位，映射到Base64字符
        encoded[j++] = base64_chars[(triple >> 3 * 6) & 0x3F];
        encoded[j++] = base64_chars[(triple >> 2 * 6) & 0x3F];
        encoded[j++] = base64_chars[(triple >> 1 * 6) & 0x3F];
        encoded[j++] = base64_chars[(triple >> 0 * 6) & 0x3F];
    }
    
    // 添加填充字符'='
    for (i = 0; i < (3 - data_len % 3) % 3; i++) {
        encoded[encoded_len - 1 - i] = '=';
    }
    
    encoded[encoded_len] = '\0';  // 添加字符串结束符
    return encoded;
}

/**
 * @brief 创建图像内容对象
 * @param mime_type 图像MIME类型
 * @param data 原始图像数据
 * @param data_len 数据长度
 * @return 创建的图像内容对象，失败返回NULL
 */
mcp_image_content_t* mcp_image_content_create(const char* mime_type, const char* data, size_t data_len) {
    // 参数验证
    if (!mime_type || !data || data_len == 0) {
        return NULL;
    }
    
    // 分配图像内容结构体内存
    mcp_image_content_t* image = malloc(sizeof(mcp_image_content_t));
    if (!image) {
        return NULL;  // 内存分配失败
    }
    
    // 复制MIME类型并编码图像数据
    image->mime_type = mcp_strdup(mime_type);
    image->encoded_data = mcp_base64_encode(data, data_len);
    
    // 检查内存分配是否成功
    if (!image->mime_type || !image->encoded_data) {
        mcp_image_content_destroy(image);  // 清理已分配的内存
        return NULL;
    }
    
    return image;
}

/**
 * @brief 销毁图像内容对象
 * @param image 要销毁的图像内容对象
 */
void mcp_image_content_destroy(mcp_image_content_t* image) {
    if (image) {
        // 释放MIME类型字符串
        mcp_free_string(image->mime_type);
        // 释放编码数据字符串
        mcp_free_string(image->encoded_data);
        // 释放结构体本身
        free(image);
    }
}

/**
 * @brief 将图像内容转换为JSON字符串
 * @param image 图像内容对象
 * @return JSON字符串，失败返回NULL
 */
char* mcp_image_content_to_json(const mcp_image_content_t* image) {
    if (!image) {
        return NULL;
    }
    
    // 创建JSON对象
    cJSON* json = cJSON_CreateObject();
    if (!json) {
        return NULL;  // JSON对象创建失败
    }
    
    // 添加JSON字段
    cJSON_AddStringToObject(json, "type", "image");           // 内容类型
    cJSON_AddStringToObject(json, "mimeType", image->mime_type);  // MIME类型
    cJSON_AddStringToObject(json, "data", image->encoded_data);   // Base64编码数据
    
    // 将JSON对象转换为字符串
    char* json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);  // 释放JSON对象
    
    return json_str;
}

/**
 * @brief 复制字符串
 * @param str 要复制的字符串
 * @return 复制的字符串，失败返回NULL
 */
char* mcp_strdup(const char* str) {
    if (!str) {
        return NULL;
    }
    
    // 计算字符串长度并分配内存
    size_t len = strlen(str);
    char* copy = malloc(len + 1);  // +1为字符串结束符
    if (copy) {
        // 复制字符串内容（包括结束符）
        memcpy(copy, str, len + 1);
    }
    return copy;
}

/**
 * @brief 释放字符串内存
 * @param str 要释放的字符串
 */
void mcp_free_string(char* str) {
    if (str) {
        free(str);  // 释放字符串内存
    }
}

/**
 * @brief 将整数转换为字符串
 * @param value 要转换的整数值
 * @return 转换后的字符串，失败返回NULL
 */
char* mcp_itoa(int value) {
    // 计算所需的缓冲区大小
    // 最大整数需要11个字符（包括负号和结束符）：-2147483648\0
    char* buffer = malloc(12);
    if (!buffer) {
        return NULL;  // 内存分配失败
    }
    
    // 使用sprintf将整数转换为字符串
    int result = snprintf(buffer, 12, "%d", value);
    if (result < 0) {
        free(buffer);  // 转换失败，释放内存
        return NULL;
    }
    
    return buffer;
}

/**
 * @brief 将cJSON对象转换为字符串
 * @param json cJSON对象
 * @return JSON字符串，失败返回NULL
 */
char* mcp_json_to_string(const cJSON* json) {
    if (!json) {
        return NULL;
    }
    // 将JSON对象转换为紧凑格式的字符串
    return cJSON_PrintUnformatted(json);
}