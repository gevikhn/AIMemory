# AI记忆系统贡献者指南

本项目致力于构建一个适用于AI的长期记忆系统，采用多层记忆存储架构（短期/中期/长期），集成图数据库和向量搜索能力。

## 编码规范与命名约定

**语言标准**
- 使用C++20标准及现代C++特性
- 遵循RAII原则，优先使用智能指针
- 变量和函数使用`snake_case`命名
- 类和类型使用`PascalCase`命名
- 常量使用`UPPER_SNAKE_CASE`命名

**代码组织**
- 头文件放在`include/`目录
- 源文件放在`src/`目录
- 库代码放在`lib/`目录
- 文档放在`doc/`目录

**构建系统**
- 使用CMake进行跨平台构建
- 支持MSVC、GCC、MinGW编译器
- 维护模块化的CMakeLists.txt文件

## 测试规范

**框架与要求**
- 使用Google Test (gtest)测试框架
- 提交PR前必须通过所有现有测试
- 新功能需要相应的测试覆盖
- 测试文件命名格式：`test_<组件名>.cpp`

**测试组织**
- 单元测试针对独立组件
- 集成测试验证系统交互
- 性能测试覆盖关键路径

## 提交与PR规范

**提交信息格式**
- 基于现有历史，使用清晰的描述性信息
- 以小写动词开头（init, add, fix, update, refactor）
- 首行保持在50字符以内
- 使用现在时态（"add feature"而非"added feature"）

**PR流程**
- 提交前确保所有测试通过
- 包含清晰的变更描述
- 适当时引用相关issue
- 更新新功能的相关文档
- 遵循`doc/ai_中长期记忆系统架构设计_v1.0.md`中的架构指导

## 架构概览

系统实现三层记忆架构：
- **短期记忆**：原始对话片段（0-7天）
- **中期记忆**：带向量嵌入的摘要内容（7-30天）
- **长期记忆**：概念化知识模式（30天以上）

核心组件包括图存储、向量索引和语义检索管道，详见系统设计文档。

## 开发环境

**环境要求**
- 支持C++20的编译器（MSVC/GCC/MinGW）
- CMake 3.20或更高版本
- Google Test测试框架

**构建流程**
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

**运行测试**
```bash
ctest --output-on-failure
```

## TODO List管理

项目采用严格的TODO list管理制度，确保开发进度可追踪、任务明确且有序完成。

### 管理原则

**强制要求**
- 每完成一项任务或功能，**必须立即更新TODO list**
- 新增功能前，先在TODO list中规划分解任务
- PR提交前，确保相关TODO项目状态正确更新
- 任何人都可以查看TODO list了解项目当前进度

**TODO项目状态**
- `pending`: 待开始的任务
- `in_progress`: 正在进行中的任务（同时只能有一个）
- `completed`: 已完成的任务

### 使用方式

**查看当前TODO**
```bash
# 查看项目根目录下的TODO.md文件
cat TODO.md
```

**更新TODO状态**
```bash
# 编辑TODO.md文件
vim TODO.md
```

**TODO文件格式示例**
```markdown
# AI Memory System TODO

## 当前进度

### 已完成 (Completed)
- [x] 创建CMake项目骨架和基础目录结构
- [x] 设置基础日志和配置系统
- [x] 创建简单CLI工具(memctl)
- [x] 设置单元测试框架

### 进行中 (In Progress)
- [ ] 实现SearchIndex倒排索引 (当前负责人: @developer)

### 待开始 (Pending)
- [ ] 实现GraphStore图存储
- [ ] 实现VectorIndex向量索引
- [ ] 实现RecallPipeline召回管道
- [ ] 实现Consolidator巩固服务
```

### 任务分解原则

**细粒度分解**
- 单个任务完成时间不超过2-4小时
- 每个任务有明确的完成标准（DoD）
- 复杂功能按模块、按接口分解

**依赖关系管理**
- 明确标注任务间的依赖关系
- 优先完成基础模块（core → search → graph → pipeline）
- 避免循环依赖

### 完成标准(DoD)

每个TODO项目必须包含明确的完成定义：

```markdown
- [ ] 实现SearchIndex倒排索引
  - DoD: BM25检索功能正常，单元测试通过，CLI可调用
  - 预计工作量: 4-6小时
  - 依赖: memory_core模块
```

### 协作规范

**多人协作**
- 领取任务前在TODO中标记负责人
- 每日更新进度状态
- 遇到阻塞及时在TODO中标注

**代码审查**
- PR必须包含TODO状态更新
- 审查者验证TODO更新的准确性
- 合并前确认所有相关任务状态正确

通过严格的TODO list管理，确保项目按架构设计文档有序推进，每个里程碑都有明确的交付标准。