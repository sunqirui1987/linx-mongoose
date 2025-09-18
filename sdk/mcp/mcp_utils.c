#include "mcp_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Base64 encoding table */
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* mcp_base64_encode(const char* data, size_t data_len) {
    if (!data || data_len == 0) {
        return NULL;
    }
    
    size_t encoded_len = 4 * ((data_len + 2) / 3);
    char* encoded = malloc(encoded_len + 1);
    if (!encoded) {
        return NULL;
    }
    
    size_t i, j;
    for (i = 0, j = 0; i < data_len;) {
        uint32_t octet_a = i < data_len ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < data_len ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < data_len ? (unsigned char)data[i++] : 0;
        
        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
        
        encoded[j++] = base64_chars[(triple >> 3 * 6) & 0x3F];
        encoded[j++] = base64_chars[(triple >> 2 * 6) & 0x3F];
        encoded[j++] = base64_chars[(triple >> 1 * 6) & 0x3F];
        encoded[j++] = base64_chars[(triple >> 0 * 6) & 0x3F];
    }
    
    /* Add padding */
    for (i = 0; i < (3 - data_len % 3) % 3; i++) {
        encoded[encoded_len - 1 - i] = '=';
    }
    
    encoded[encoded_len] = '\0';
    return encoded;
}

mcp_image_content_t* mcp_image_content_create(const char* mime_type, const char* data, size_t data_len) {
    if (!mime_type || !data || data_len == 0) {
        return NULL;
    }
    
    mcp_image_content_t* image = malloc(sizeof(mcp_image_content_t));
    if (!image) {
        return NULL;
    }
    
    image->mime_type = mcp_strdup(mime_type);
    image->encoded_data = mcp_base64_encode(data, data_len);
    
    if (!image->mime_type || !image->encoded_data) {
        mcp_image_content_destroy(image);
        return NULL;
    }
    
    return image;
}

void mcp_image_content_destroy(mcp_image_content_t* image) {
    if (image) {
        mcp_free_string(image->mime_type);
        mcp_free_string(image->encoded_data);
        free(image);
    }
}

char* mcp_image_content_to_json(const mcp_image_content_t* image) {
    if (!image) {
        return NULL;
    }
    
    cJSON* json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }
    
    cJSON_AddStringToObject(json, "type", "image");
    cJSON_AddStringToObject(json, "mimeType", image->mime_type);
    cJSON_AddStringToObject(json, "data", image->encoded_data);
    
    char* json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    
    return json_str;
}

char* mcp_strdup(const char* str) {
    if (!str) {
        return NULL;
    }
    
    size_t len = strlen(str);
    char* copy = malloc(len + 1);
    if (copy) {
        memcpy(copy, str, len + 1);
    }
    return copy;
}

void mcp_free_string(char* str) {
    if (str) {
        free(str);
    }
}

char* mcp_json_to_string(const cJSON* json) {
    if (!json) {
        return NULL;
    }
    return cJSON_PrintUnformatted(json);
}