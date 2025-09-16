/*
 * LINX WebSocket SDK - 日志模块实现
 * 版权所有 (C) 2024
 */

#include "linx_log.h"
#include <time.h>
#include <string.h>

/* 全局日志级别，默认为INFO */
int g_linx_log_level = 2; /* LINX_LOG_INFO */

/* 日志级别字符串 */
static const char* log_level_strings[] = {
    "ERROR",
    "WARN",
    "INFO",
    "DEBUG",
    "TRACE"
};

/* 日志函数实现已移至linx_utils.c和linx_websocket_sdk.c */
/* 此文件保留为向后兼容 */