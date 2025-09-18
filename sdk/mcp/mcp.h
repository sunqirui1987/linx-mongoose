#ifndef MCP_H
#define MCP_H

/*
 * MCP (Model Context Protocol) C Implementation
 * Reference: https://modelcontextprotocol.io/specification/2024-11-05
 * 
 * This is a C99 implementation of the MCP server functionality,
 * ported from the original C++ implementation.
 */

#include "mcp_types.h"
#include "mcp_utils.h"
#include "mcp_property.h"
#include "mcp_tool.h"
#include "mcp_server.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Version information */
#define MCP_VERSION_MAJOR 1
#define MCP_VERSION_MINOR 0
#define MCP_VERSION_PATCH 0
#define MCP_VERSION_STRING "1.0.0"

/* Protocol version */
#define MCP_PROTOCOL_VERSION "2024-11-05"

/* Convenience macros for creating properties */
#define MCP_PROPERTY_BOOL(name, default_val) \
    mcp_property_create_boolean(name, default_val, true)

#define MCP_PROPERTY_BOOL_REQUIRED(name) \
    mcp_property_create_boolean(name, false, false)

#define MCP_PROPERTY_INT(name, default_val) \
    mcp_property_create_integer(name, default_val, true, false, 0, 0)

#define MCP_PROPERTY_INT_REQUIRED(name) \
    mcp_property_create_integer(name, 0, false, false, 0, 0)

#define MCP_PROPERTY_INT_RANGE(name, default_val, min_val, max_val) \
    mcp_property_create_integer(name, default_val, true, true, min_val, max_val)

#define MCP_PROPERTY_INT_RANGE_REQUIRED(name, min_val, max_val) \
    mcp_property_create_integer(name, 0, false, true, min_val, max_val)

#define MCP_PROPERTY_STRING(name, default_val) \
    mcp_property_create_string(name, default_val, true)

#define MCP_PROPERTY_STRING_REQUIRED(name) \
    mcp_property_create_string(name, NULL, false)

/* Convenience macros for return values */
#define MCP_RETURN_TRUE() mcp_return_bool(true)
#define MCP_RETURN_FALSE() mcp_return_bool(false)
#define MCP_RETURN_SUCCESS() mcp_return_bool(true)
#define MCP_RETURN_FAILURE() mcp_return_bool(false)

#ifdef __cplusplus
}
#endif

#endif /* MCP_H */