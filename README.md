# AI Memory System

AI长期记忆系统，实现三层记忆架构（短期/中期/长期），集成图数据库和向量搜索功能。

## 项目特性

- **三层记忆架构**: 短期记忆（0-7天）、中期记忆（7-30天）、长期记忆（30天+）
- **混合检索**: BM25关键词搜索 + 向量语义搜索 + 图结构扩散
- **图式记忆**: 节点表示概念/事实/实体，边表示关系
- **自研存储引擎**: 图存储、倒排索引、向量索引全部自主实现
- **模块化设计**: 可插拔的组件架构，支持功能降级

## 快速开始

### 构建项目

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译
cmake --build .
```

### 基础使用

```bash
# 查看帮助
./memctl --help

# 查看版本
./memctl version

# 配置管理
./memctl config set log_level DEBUG
./memctl config get log_level
```

### 运行测试

```bash
# 运行所有单元测试
ctest --output-on-failure

# 运行特定测试
./test_config
./test_types
```

## 项目结构

```
AIMemory/
├── CMakeLists.txt           # 主CMake配置文件
├── config.yaml              # 默认配置文件
├── TODO.md                  # 项目进度跟踪
├── AGENTS.md               # AI专用
├── doc/                    # 详细设计文档
│   └── ai_中长期记忆系统架构设计_v1.0.md
├── src/                   # 源代码目录
│   ├── core/              # 核心模块（配置、日志、类型）
│   ├── cli/               # 命令行工具
│   ├── tests/             # 单元测试
│   ├── search/            # 倒排索引（待实现）
│   ├── graph/             # 图存储（待实现）
│   ├── vector/            # 向量索引（待实现）
│   ├── pipeline/          # 召回管道（待实现）
│   └── jobs/              # 后台任务（待实现）
└── examples/              # 使用示例
    └── basic_usage.bat
```

## 开发指南

### 环境要求

- C++20 兼容编译器 (MSVC/GCC/MinGW)
- CMake 3.20 或更高版本
- Google Test (自动下载)

### 编码规范

- 使用 C++20 标准特性
- 变量/函数: `snake_case`
- 类/类型: `PascalCase`
- 常量: `UPPER_SNAKE_CASE`
- 遵循 RAII 原则

### TODO管理

项目使用严格的TODO list管理制度：

1. **查看进度**: `cat TODO.md`
2. **任务完成后立即更新TODO状态**
3. **新功能前先规划分解任务**

详见 [AGENTS.md](AGENTS.md) 中的TODO管理章节。

## 架构概览

系统基于 `doc/ai_中长期记忆系统架构设计_v1.0.md` 中的详细设计实现：

### 核心组件

1. **SearchIndex (倒排索引)**: BM25检索，分词，段式存储
2. **GraphStore (图存储)**: 节点/边存储，k-hop扩展，PPR算法
3. **VectorIndex (向量索引)**: HNSW算法，可选模块
4. **RecallPipeline (召回管道)**: 多阶段检索与重排
5. **Jobs (后台任务)**: 巩固、衰减、优化

### 数据模型

```cpp
// 节点类型
enum class NodeType {
    EPISODE,    // 对话片段
    FACT,       // 事实陈述
    CONCEPT,    // 抽象概念
    ENTITY,     // 具名实体
    // ...
};

// 边类型
enum class EdgeType {
    ABOUT,          // 主题归属
    MENTIONS,       // 提及关系
    SUPPORTS,       // 支持关系
    TEMPORAL_NEXT,  // 时序关系
    // ...
};
```

## 当前状态

**M0阶段** (项目骨架) - ✅ **已完成**
- [x] CMake项目结构
- [x] 基础日志配置系统
- [x] CLI工具框架
- [x] 单元测试框架

**下一阶段**: M1 SearchIndex倒排索引实现

查看 [TODO.md](TODO.md) 了解详细进度和后续计划。

## 许可证

本项目采用 MIT 许可证。详见 LICENSE 文件。

## 贡献

欢迎贡献代码！请先阅读 [AGENTS.md](AGENTS.md) 了解开发规范和流程。