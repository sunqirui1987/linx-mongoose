# MCP C SDK 测试套件

这个目录包含了 MCP (Model Context Protocol) C SDK 的完整测试套件和示例程序。

## 目录结构

```
test/
├── Makefile                    # 构建脚本
├── README.md                   # 本文档
├── test_types.c               # 类型定义测试
├── test_utils.c               # 工具函数测试
├── test_property.c            # 属性管理测试
├── test_tool.c                # 工具管理测试
├── test_server.c              # 服务器功能测试
├── test_integration.c         # 集成测试
├── examples/                  # 示例程序目录
│   ├── calculator_server.c   # 计算器服务器示例
│   ├── file_manager_server.c # 文件管理服务器示例
│   └── weather_server.c      # 天气服务器示例
└── build/                     # 编译输出目录（自动创建）
```

## 快速开始

### 1. 安装依赖

确保系统已安装以下依赖：

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install gcc libcjson-dev make
```

**CentOS/RHEL:**
```bash
sudo yum install gcc cjson-devel make
```

**macOS:**
```bash
brew install gcc cjson make
```

### 2. 编译测试

```bash
# 编译所有测试和示例
make all

# 或者只编译测试
make test-types test-utils test-property test-tool test-server test-integration

# 或者只编译示例
make examples
```

### 3. 运行测试

```bash
# 运行所有测试
make test

# 运行单个测试
make test-types      # 类型定义测试
make test-utils      # 工具函数测试
make test-property   # 属性管理测试
make test-tool       # 工具管理测试
make test-server     # 服务器功能测试
make test-integration # 集成测试
```

### 4. 运行示例

```bash
# 运行计算器服务器示例
make run-calculator

# 运行文件管理服务器示例
make run-file-manager

# 运行天气服务器示例
make run-weather
```

## 测试说明

### 单元测试

#### test_types.c - 类型定义测试
- 测试 MCP 基本数据类型
- 验证枚举值和结构体定义
- 检查类型转换和验证函数

#### test_utils.c - 工具函数测试
- 测试字符串处理函数
- 验证内存管理函数
- 检查 JSON 解析和生成函数

#### test_property.c - 属性管理测试
- 测试属性创建和销毁
- 验证属性值的设置和获取
- 检查属性列表操作
- 测试属性序列化和反序列化

#### test_tool.c - 工具管理测试
- 测试工具创建和销毁
- 验证工具属性管理
- 检查工具执行和回调
- 测试工具序列化

#### test_server.c - 服务器功能测试
- 测试服务器创建和销毁
- 验证工具注册和管理
- 检查消息处理和路由
- 测试 JSON-RPC 协议实现

### 集成测试

#### test_integration.c - 集成测试
- 测试完整的 MCP 工作流程
- 验证各模块之间的协作
- 检查内存管理和错误处理
- 测试并发安全性

## 示例程序

### calculator_server.c - 计算器服务器
一个简单的计算器服务器，支持基本数学运算：
- 加法 (add)
- 减法 (subtract)
- 乘法 (multiply)
- 除法 (divide)
- 幂运算 (power)
- 阶乘 (factorial)

**使用示例：**
```json
{"jsonrpc":"2.0","id":1,"method":"tools/call","params":{"name":"add","arguments":{"a":5,"b":3}}}
```

### file_manager_server.c - 文件管理服务器
一个安全的文件管理服务器，支持文件操作：
- 读取文件 (read_file)
- 写入文件 (write_file)
- 列出目录 (list_directory)
- 删除文件 (delete_file)
- 获取文件信息 (file_info)

**安全特性：**
- 限制在沙盒目录内操作
- 防止目录遍历攻击
- 文件大小限制

### weather_server.c - 天气服务器
一个模拟的天气信息服务器，支持天气查询：
- 获取当前天气 (get_current_weather)
- 获取天气预报 (get_weather_forecast)
- 获取天气统计 (get_weather_stats)
- 列出支持的城市 (list_cities)
- 比较城市天气 (compare_weather)

## 高级功能

### 代码覆盖率测试

```bash
# 运行覆盖率测试（需要 gcov）
make coverage

# 查看覆盖率报告
ls *.gcov
```

### 内存泄漏检测

```bash
# 运行内存泄漏检测（需要 valgrind）
make valgrind
```

### 静态代码分析

```bash
# 运行静态代码分析（需要 cppcheck）
make static-analysis
```

### 代码格式化

```bash
# 格式化代码（需要 clang-format）
make format
```

## 测试结果解读

### 成功输出示例
```
========================================
运行 MCP C SDK 测试套件
========================================

1. 运行类型定义测试...
✓ 所有类型定义测试通过 (5/5)

2. 运行工具函数测试...
✓ 所有工具函数测试通过 (8/8)

3. 运行属性管理测试...
✓ 所有属性管理测试通过 (12/12)

4. 运行工具管理测试...
✓ 所有工具管理测试通过 (10/10)

5. 运行服务器功能测试...
✓ 所有服务器功能测试通过 (15/15)

6. 运行集成测试...
✓ 所有集成测试通过 (8/8)

========================================
所有测试完成！
========================================
```

### 失败处理
如果测试失败，会显示详细的错误信息：
- 失败的测试用例名称
- 期望值和实际值
- 错误发生的位置
- 建议的修复方法

## 常见问题

### Q: 编译时出现 "cjson/cJSON.h: No such file or directory" 错误
A: 请安装 libcjson 开发包：
```bash
# Ubuntu/Debian
sudo apt-get install libcjson-dev

# CentOS/RHEL
sudo yum install cjson-devel

# macOS
brew install cjson
```

### Q: 测试运行时出现段错误
A: 可能是内存管理问题，建议：
1. 使用 valgrind 检测内存泄漏
2. 检查指针是否正确初始化
3. 确保在使用前检查指针是否为 NULL

### Q: 示例程序无法启动
A: 检查以下几点：
1. 确保编译成功
2. 检查依赖库是否正确安装
3. 确保有足够的权限运行程序

### Q: JSON-RPC 消息格式错误
A: 确保消息格式符合 JSON-RPC 2.0 规范：
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "method_name",
  "params": {}
}
```

## 贡献指南

### 添加新测试
1. 在相应的测试文件中添加测试函数
2. 在 `run_tests()` 函数中调用新测试
3. 更新测试计数器
4. 运行测试确保通过

### 添加新示例
1. 在 `examples/` 目录创建新的 `.c` 文件
2. 在 `Makefile` 中添加编译规则
3. 更新本文档的示例说明
4. 添加使用示例和说明

### 代码规范
- 使用 C99 标准
- 遵循现有的命名约定
- 添加适当的注释
- 确保内存正确管理
- 处理所有错误情况

## 许可证

本测试套件遵循与 MCP C SDK 相同的许可证。

## 联系方式

如有问题或建议，请通过以下方式联系：
- 提交 Issue
- 发送 Pull Request
- 邮件联系维护者

---

**注意：** 本测试套件仅用于验证 MCP C SDK 的功能，不应在生产环境中直接使用示例代码。