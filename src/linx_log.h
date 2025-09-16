/*
 * LINX WebSocket SDK - 日志模块
 * 版权所有 (C) 2024
 */

#ifndef LINX_LOG_H
#define LINX_LOG_H

#include <stdio.h>
#include <stdarg.h>
#include "linx_websocket_sdk.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 日志级别定义在linx_websocket_sdk.h中 */

/* 全局日志级别 */
extern int g_linx_log_level;

/* 日志宏定义在linx_internal.h中 */

/* 日志函数声明 */
void linx_log(linx_log_level_t level, const char *file, int line, 
              const char *func, const char *format, ...);
void linx_set_log_level(linx_log_level_t level);

#ifdef __cplusplus
}
#endif

#endif /* LINX_LOG_H */