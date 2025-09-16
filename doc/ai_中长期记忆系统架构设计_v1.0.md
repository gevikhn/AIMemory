# Graph Memory：AI 中长期记忆系统架构设计（v1）

> 角色设定：你（首席架构师）统筹全局；其他 Agent 负责实现与运行。本文档作为工程实现与协作协议的权威来源（SSOT）。

---

## 0. 目标与范围

**目标**：为大模型提供接近“无限上下文”的中长期记忆能力，支持多会话、跨主题的稳健回忆与持续学习。

**关键边界（按你的最新要求约束）**：

- **只聚焦记忆/回忆能力**：不实现任何训练/微调/推理相关的 ML 算法；摘要与向量由外部已存在模型提供。
- **自研存储**：**图数据库与向量数据库均由本系统实现**，以降低外部依赖、确保长期可控可演进；倒排索引（BM25）亦内置实现。
- **C++ 主框架**：核心服务、索引与检索管线以 C++ 实现；通过可插拔模块暴露“向量/语义搜索”等能力，支持在 embedding 不可用时**能力退化**（关键词+图结构回忆）。
- **多 Agent 协作**：本系统只提供存储/检索 API；外部 Agent 负责摘要生成与 embedding 生成并调用本系统接口。

**关键能力**：

- 语义检索（可选向量）× 关键词检索（符号）× 图式联想（多跳）
- 时间脉络、主题关联、因果/支持/矛盾等关系建模
- 记忆权重、情绪/重要度加权、时间衰减与巩固（Consolidation）
- 召回→扩散→重排→摘要打包的“回忆流水线”
- 多 Agent 协作协议与可观测性

**不在本期范围**：端到端强化学习、复杂主动学习策略、跨组织多租户共享知识的合规联邦学习（留待 v2/v3）。

---

## 1. 核心理念

- **混合检索**：BM25/倒排索引 + 向量 ANN + 图扩散（k-hop/PPR）。
- **图式记忆**：记忆以节点（Nodes）承载事实/事件/偏好/概念，以边（Edges）表达时序、主题、因果、支持/矛盾等关系。
- **分层存储**：短期（原始片段）→ 中期（摘要+链接）→ 长期（概念/模式化知识）。
- **权重驱动**：重要性、频次、情绪强度、近期性、用户反馈共同决定“易取度”。
- **可证伪与溯源**：每条记忆附带来源、时间戳、置信度与引用路径，便于冲突化解与审计。

---

## 2. 数据模型（Schema）

### 2.1 节点类型（Node.type）

- **Episode**：对话/经历/一次性事件（带时间戳，含原文片段引用）。
- **Fact**：陈述性事实（可被验证，可能随时间更新）。
- **Preference**：用户偏好/习惯/约束（如“优先中文回答”）。
- **Concept**：抽象概念/主题/术语（长期巩固生成，作为“中枢”节点）。
- **Entity**：具名实体（人/地/物/组织/产品）。
- **Task**：任务/待办/目标状态。
- **Meta**：系统/流程/策略类节点（用于运行时控制与审计）。

### 2.2 边类型（Edge.type）

- **TEMPORAL\_NEXT**（时间顺序）
- **ABOUT**（主题/话题归属）
- **MENTIONS**（提及实体）
- **CAUSES / CAUSED\_BY**（因果）
- **SUPPORTS / CONTRADICTS**（支持/矛盾）
- **DERIVED\_FROM**（摘要/抽象的来源）
- **REFERS\_TO**（跨会话引用）
- **SIMILAR\_TO**（近义/相似语义）

### 2.3 Node Schema（JSON）

```json
{
  "id": "node_uuid",
  "type": "Episode|Fact|Preference|Concept|Entity|Task|Meta",
  "title": "可读标题",
  "text": "核心内容（摘要或事实）",
  "raw_refs": [
    {"source_id": "conv_2025-09-15#34", "offset": [102, 215]}
  ],
  "embeddings": [
    {"name": "text", "model": "text-embed-X", "vector_id": "vec_...", "dim": 1536},
    {"name": "emotion", "model": "affect-embed-Y", "vector_id": "vec_...", "dim": 64}
  ],
  "keywords": ["蓝牙", "Win11", "驱动"],
  "entities": ["Windows 11", "Bluetooth"],
  "sentiment": {"valence": 0.2, "arousal": 0.6},
  "importance": 0.85,
  "confidence": 0.9,
  "recency": "2025-09-15T10:35:10Z",
  "frequency": 3,
  "privacy": {"pii": false, "scope": "private|team|public"},
  "provenance": {"creator": "SummarizerAgent@v1", "version": 3},
  "tags": ["debug", "os"],
  "tenant_id": "user_or_team_id"
}
```

### 2.4 Edge Schema（JSON）

```json
{
  "id": "edge_uuid",
  "src": "node_uuid_A",
  "dst": "node_uuid_B",
  "type": "ABOUT|TEMPORAL_NEXT|CAUSES|SUPPORTS|CONTRADICTS|...",
  "weight": 0.73,
  "temporal": {"since": "2025-09-14T09:00:00Z", "until": null},
  "provenance": {"creator": "LinkerAgent@v1"},
  "confidence": 0.88,
  "tenant_id": "user_or_team_id"
}
```

> 说明：所有节点与边均应具备 `created_at/updated_at`、软删标识与审计字段。

---

## 3. 存储选型与布局（自研实现）

### 3.0 原始内容（Raw）存储设计

> 原始内容（对话输入/输出、附件、日志片段等）**不直接存入 Episode 文本**，而是以\*\*不可变块（RawChunk）\*\*形式保存在对象存储，并由 Episode/Fact 通过 `DERIVED_FROM` 与 `raw_refs` 引用。这样保证可追溯、去重、易于归档/删除。

**RawChunk 模型**

```json
{
  "id": "raw_<sha256_prefix>",
  "mime": "text/plain; charset=utf-8",
  "len_bytes": 2048,
  "sha256": "...",
  "created_at": "...",
  "meta": {"conv_id":"...","role":"user|assistant","ts":"...","lang":"zh-CN"}
}
```

- **存放位置**：`/tenants/{tenant}/raw/YYYY/MM/DD/sha256[0:2]/raw_sha256.blob.zst`
- **压缩/加密**：Zstd 压缩；可选租户级静态加密（AES-GCM，KMS 管理密钥）。
- **去重**：以 `sha256` 做内容寻址（CAS）；多 Episode 可共享同一 RawChunk。
- **切分策略**：
  - 文本：按 **token 1k–2k** 或自然段切块；
  - 多模态：保持原文件为单块，需时再做区间引用。

**Episode 与 Raw 的关系**

- Episode `text` 字段仅存**摘要**；
- `raw_refs` 数组保存 *引用列表*：`[{"raw_id":"raw_...","offset":0,"length":1024}]`；
- `DERIVED_FROM` 边指向支撑的 RawChunk 或中间事实。

**生命周期**

- 短期（0–7 天）：保留完整原始块（可热读）；
- 中期（7–30 天）：原始块转为冷存，仅按需回页；
- 长期（30 天 +）：默认只保留摘要与最强证据路径，原始块可物理归档/删除（受策略与选择性遗忘影响）。

**检索与呈现**

- 回忆打包可选择“带引用”：按 `raw_refs` 抽取原文片段生成可复制的引用卡片（带 blob 路径/偏移校验）；
- 选择性遗忘会删除 RawChunk 并修复 `raw_refs`/`DERIVED_FROM`，留下墓碑与审计记录。

本系统自研三类索引/存储引擎，并提供统一事务/快照框架：

### 3.1 基础存储层（Storage Engine）

- **文件布局**：分租户目录；采用 **WAL + Checkpoint + Snapshot** 架构；页大小 4KB；所有段文件均带魔数/版本/CRC32 校验。
- **写入路径**：先写 WAL（fsync），后台合并到段文件（append-only），周期性快照生成 `MANIFEST` 指向当前一致视图；读者基于快照做到无锁读取（只读 mmap）。
- **并发控制**：多写多读，写入采用轻量级锁+批量提交；读侧快照隔离；后台 **Compactor** 做段合并与索引重排。

### 3.2 自研图存储（GraphStore）

- **模型**：Property Graph（节点/边/属性/标签）。
- **结构**：
  - 顶点表：`vertices.seg`（变长记录，含 label、属性指针）。
  - 边表：`edges.seg`（src/dst/type/weight/ts/props），出入度索引分离。
  - **邻接索引**：CSR/Adjacency 列存 + LSM 段式增量；高频点度数分层桶化以控制热点。
  - **二级索引**：label 索引、`(type, src)`、`(type, dst)` B+Tree（段式）。
- **查询算子**：按边型的 k-hop、PPR（个性化 PageRank）、最短路（可选）、社区检测（离线巩固时）。

### 3.3 自研倒排索引（SearchIndex，BM25）

- **分词/归一化**：Tokenizer 插件（中文可接入外部分词；无可用时退化为空白/标点切分）。
- **字典**：FST/前缀压缩；
- **倒排表**：按 term → postings（docID 差分+varint、分块跳表）；支持位置信息（短语/邻近）。
- **排序**：BM25+、近因与字段加权；支持布尔过滤（label/type/时间窗）。

### 3.4 自研向量索引（VectorIndex，HNSW 为基线）

- **算法**：HNSW（M、efConstruction、efSearch 可调），可插拔实现 IVF-PQ（可选）。
- **持久化**：图层级结构以段文件存储，节点向量块化（row-major），索引头含超参数；
- **更新**：支持 Upsert/Logical Delete，后台重建小批量合并；
- **加载**：mmap + 懒加载；崩溃后一致性由 WAL + 快照恢复；
- **降级**：当 embedding 不可用或索引未就绪时，禁用向量检索分支。

### 3.5 统一元数据与对象存储

- **元数据**：轻量 KV（同引擎实现）存放配置、统计、阈值、租户配额。
- **对象存储**：原始片段快照与附件以段式 Blob 存储（本地/分布式），支持引用计数。

### 3.6 部署拓扑（逻辑）

- Gateway（gRPC/REST）→ Service 层（Ingestion/Linker/Indexer/Retriever/Consolidator/Decay/Privacy/Audit）→ 自研 GraphStore/VectorIndex/SearchIndex → 存储文件

---

## 4. 处理流水线（从对话到记忆）

1. **Ingestion**：捕获对话 I/O，切分为可引用的 `Chunk`。
2. **Summarization**（摘要/要点提取）：
   - 生成 Episode/Fact/Preference 候选节点；
   - 提取关键词、术语、情绪、实体、时间；
3. **Embedding**：为文本与情绪生成多路 Embedding；
4. **Linking**：
   - 主题归属（ABOUT → Concept）、实体提及（MENTIONS）、时序（TEMPORAL\_NEXT）；
   - 相似性链接（SIMILAR\_TO，阈值/邻居数限制）、因果/支持/矛盾判定；
5. **Indexing**：写入向量库、倒排索引、图数据库；
6. **Consolidation（批/准实时）**：
   - 社区发现/聚类 → 生成/更新 Concept 节点；
   - 事实合并（去重与版本化），维护 DERIVED\_FROM；
7. **Decay & GC**：时间衰减、低权重归档、冷存/删除；
8. **Audit/Observability**：事件流与指标上报。

### 4.x 原始→图数据转换定义（R2G Spec）

> 本小节严格定义\*\*从原始数据（RawChunk/对话轮次）到图数据（节点/边）\*\*的转换流程、规则与降级策略。

#### R2G-0 范围

- **输入**：文本对话（必须）、可选附件（图片/音频/文件，作为 RawChunk 暂存）。
- **输出**：`Episode/Fact/Preference/Entity/Concept` 节点与各类边；所有节点保留 `DERIVED_FROM` 与 `raw_refs` 溯源。

#### R2G-1 预处理

- **分句/切块**：中文按标点与长度（≈1k–2k tokens）切 `RawChunk`；
- **基础 NLP**（可插拔，缺失时降级见 R2G-6）：
  - 分词/词形归一（lemma）、词性（POS）；
  - 命名实体识别（NER：Person/Org/Location/Object/Time/...）；
  - 指代消解（Coref）；
  - 时间归一（TimeNorm：绝对/相对 → ISO8601 + 粒度）；
  - 语义角色标注（SRL）/依存（可选）。

#### R2G-2 节点创建规则

- **Episode**（必建）：
  - 粒度：默认“每轮对话”1个 Episode；长文本可按小节/话题拆分；
  - `text`：摘要（不放原文）；`raw_refs`：覆盖该轮所有 RawChunk 片段；
  - `recency/frequency/importance` 初值：`recency=now`，`frequency=1`，`importance` 由 Summarizer 评分。
- **Entity**：从 NER/规则抽取，按 `name+type+tenant` 去重；记录别名/标准名；
- **Fact（事件/陈述）**：
  - **事件中心表示**：为每个谓词性陈述创建 Fact，字段含 `predicate.normalized`、`negation`、`modality`、`time`；
  - 多句指向同一事件时合并（见 R2G-5 SAME\_AS）。
- **Preference**：从指令/偏好语言触发（“以后优先中文回答”）；
- **Concept**：若已有显式主题词或命中词典则即建；否则由巩固阶段生成。

#### R2G-3 边创建规则

- **TEMPORAL\_NEXT**：按时间将 Episode 串联；
- **MENTIONS**：Episode/Fact → Entity（每次提及）；
- **ROLES（语义角色）**：Fact → Entity 的角色边：`AGENT/PATIENT/LOCATION/TIME/MANNER/INSTRUMENT/RECIPIENT/CAUSE/BENEFICIARY`；
- **ABOUT**：Episode/Fact/Preference → Concept（主题归属）；
- **SUPPORTS/CONTRADICTS**：Fact ↔ Fact（同一主题内证据支持或矛盾）；
- **SAME\_AS**：跨句/跨轮的同一实体/事件对齐（由 Coref/匹配触发）；
- **DERIVED\_FROM**：Episode/Fact → RawChunk 证据；
- **SIMILAR\_TO**：同类型节点间，语义/关键词相似度超过阈值时建（可延迟到离线）。

#### R2G-4 规范化与打分

- **谓词归一**：动词/动宾短语 → 词干或预设概念（如 "踢球"→`play_soccer`）；

- **时间**：缺省 `granularity="unspecified"`；相对时间需解析到绝对时间并记录原表达；

- **置信度**：对每个抽取单元给出 `conf`（0–1），用于边/节点初权重：

  `edge.weight = 0.5*conf_extract + 0.3*source_trust + 0.2*recency_boost`

- **去重规则**：Entity/Facts 使用 `(normalized_text, type, time_bucket)` 进行 LSH/指纹去重；命中即合并并提升 `frequency`。

#### R2G-5 合并与版本

- **事件合并**：若 `predicate.normalized`、主要角色（AGENT/PATIENT）与时间窗口相近（如 ±1d）且相似度≥θ，则视为同一事件，使用 `SAME_AS` 合并；
- **事实版本链**：同一主体/属性在时间上演化 → 建立 `VERSION_NEXT`，并维护 `ACTIVE_OF`（见 §7）。

#### R2G-6 降级策略（无 SRL/向量时）

- **最小规则**：
  - 主语（名词短语/人名）→ `AGENT`；
  - 介词“在/于/里/上”+ 地点 → `LOCATION`；
  - 动词 + 直接宾语 → `predicate` + `PATIENT`；
  - 时间词（今天/明天/上周）→ `TIME`；
- **相似关系**：`SIMILAR_TO` 先基于关键词与共现统计构造（向量可用后再补齐）。

#### R2G-7 示例（“小明在公园里踢足球”）

- **RawChunk**：收录原句；
- **Entity**：`小明(Person)`, `公园(Location)`, `足球(Object)`；
- **Fact(evt\_123)**：`predicate.normalized = play_soccer`，`negation=false`；
- **Edges**：`evt_123 -AGENT-> 小明`，`evt_123 -PATIENT-> 足球`，`evt_123 -LOCATION-> 公园`；
- **Episode**：摘要“提到一次在公园踢球”，`DERIVED_FROM` 指向 RawChunk；
- **ABOUT**：若存在 `Concept: 球类运动/足球活动` 则建立归属。

#### R2G-8 伪代码

```python
for chunk in raw_chunks:
  sents = split_sentences(chunk.text)
  for sent in sents:
    ner, pos, deps, srl = nlp(sent)
    ep = ensure_episode(chunk)
    ents = upsert_entities(ner)
    fact = extract_fact(sent, srl or rules)
    if fact:
      fact = upsert_fact(normalize_predicate(fact), time_norm(sent))
      link_roles(fact, ents, srl_or_rules)
      link(ep, fact, ABOUT=topic_detect(sent))
      link_mentions(ep, ents)
      derive_from(ep, chunk)
      derive_from(fact, chunk, span=locate_span(sent))
consolidate_if_needed()
```

### 4.y 最小化工作流（MVP，无向量）

> 你的最小化链路：**输入原始内容 → 生成图数据 → 输入查询 → 提取关键词 → 在图中检索 → 条件降级到原文关键词查找**。以下为可直接落地的运行规范。

**W1. 输入原始内容 → RawChunk**

- `memctl ingest --raw path_or_text` 生成 RawChunk（Zstd 压缩，CAS 去重）。

**W2. 生成图数据（R2G）**

- 运行 `memctl r2g --from raw --to graph`：
  - 建立 Episode/Entity/Fact 节点与核心边（TEMPORAL\_NEXT/MENTIONS/ABOUT/ROLES）。
  - 为 Episode/Fact/Concept 写入 **NodeIndex（BM25）** 与关键词字段（用于图内筛选）。

**W3. 查询 → 关键词提取**

- `memctl recall --q "..." --extract-keywords`（默认内置规则分词 + 停用词过滤）。

**W4. 在图中检索（Graph-first）**

- **优先模式（推荐）**：
  1. 用 **NodeIndex** 在（Episode/Fact/Concept）上做 BM25 召回得到 `seeds`；
  2. 在 GraphStore 上做 `k=1` 扩展与类型过滤（ABOUT/SUPPORTS/TEMPORAL\_NEXT），得到 `subgraph`；
  3. 以简单打分（BM25×0.7 + 近因×0.3）排序并返回。
- **纯图降级（当 SearchIndex 未就绪）**：
  - 在 GraphStore 内对 `node.keywords/tags/entities` 做布尔筛选得到 `seeds` → `k=1` 扩展；
  - 仅用近因/权重排序（无 BM25）。

**W5. 条件降级到原文关键词查找（RawIndex，可选）**

- 触发条件（任一满足）：
  - NodeIndex 召回空或 TopN 分数集中度 < 阈值；
  - 查询包含 **错误码/长数字/URL/路径/代码样式 Token**；
  - 用户显式要求“找原文/带引用”。
- 执行：在短期/热点 RawChunk 的 **Passage 级倒排** 上做关键词检索，返回命中片段并与图结果去重合并。

**W6. 输出**

- 返回：节点列表 +（可选）`k=1` 证据路径 +（若走 Raw）原文片段引用卡片（`raw_refs`）。

**配置开关（MVP 默认值）**

```yaml
recall:
  vector.enabled: false     # 无向量
  graph.k_hop: 1
  use_node_index: true      # 用 NodeIndex 作为图内入口
  raw.enabled: true         # 允许原文回退
  raw.trigger:
    low_coverage_topn: 10   # TopN < 10 触发
    low_concentration: 0.6  # 分数 Gini/熵阈
    pattern_tokens: [hex, uuid, url, path, code, error_code]
```

**MVP 伪代码**

```python
def recall_mvp(q):
    K = extract_keywords(q)
    if use_node_index:
        seeds = bm25_node.search(K, topk=200, recency_boost=True)
    else:
        seeds = graph.filter_nodes(keywords=K, types=[Episode, Fact, Concept], topk=200)
    if need_raw_fallback(q, seeds):
        raw_hits = bm25_raw.search(K, topk=100)
        seeds = dedup_union(seeds, raw_hits)
    subgraph = graph.khop(seeds, k=1, edge_types=[ABOUT, SUPPORTS, TEMPORAL_NEXT])
    return rank_and_pack(subgraph, formula="0.7*BM25+0.3*Recency")
```

**CLI 示例**

```bash
memctl ingest --raw ./samples/chat1.txt
memctl r2g --from raw --to graph
memctl recall --q "Win11 蓝牙 打不开" --mvp --trace
# SearchIndex 未就绪时：
memctl recall --q "错误码 0x8007045D" --mvp --graph-only --allow-raw
```

**DoD（最小可用验收）**

- SearchIndex 可选：未就绪时纯图筛选可运行；
- `k=1` 邻接扩展与类型过滤正确；
- 原文回退在错误码/路径类查询下能返回可复制的原文片段；
- 全流程 P95 延迟 ≤ 1.5s（10 万节点/100 万边内测环境）。

---

## 5. 回忆与检索算法（Recall Pipeline）

### 5.1 查询构建

- 输入：当前用户问题 Q、当前会话上下文 C、系统状态 S。
- 生成：关键词集 K、查询向量 v、实体集合 E、时间约束 T、意图标签 I。

### 5.2 多阶段检索

1. **阶段 A：关键词 × 近因优先**
   - 倒排索引检索 Top-N（BM25）；
   - 近因加权：同一租户下最近 14～30 天的节点优先；
2. **阶段 B：向量 ANN**
   - 在（Episode/Fact/Preference/Concept）多索引上并行向量召回；
   - MMR 去冗余，控制多样性；
3. **阶段 C：图扩散**
   - 从候选种子集合出发，进行 k-hop（k=1～2）扩展；
   - 使用 **个性化 PageRank (PPR)** 对相关子图打分；

### 5.3 重排与预算打包

- **综合评分**： \(score = λ1*BM25 + λ2*cos(v, v_i) + λ3*GraphRank_i + λ4*Weight_i\)
- **Weight\_i**：见第 6 节权重模型。
- **Knapsack 近似**：在上下文 Token 预算下，优先选高分/高证据密度条目；
- **摘要器**：将支持性 Episode 证据压缩为“引用摘要 + 引用路径”。

### 5.4 伪代码（简化）

```python
candidates = bm25.search(K, topk=200, recency_boost=True)
vec_hits  = ann.search(v, index=[Episode, Fact, Concept], topk=300)
seeds = dedup_union(candidates, vec_hits, by="node_id", mmr=True)
subgraph = graph.khop(seeds, k=2, edge_types=[ABOUT, SUPPORTS, CONTRADICTS, TEMPORAL_NEXT])
rank = personalize_pagerank(subgraph, seeds)
for n in subgraph.nodes:
    n.final = λ1*n.bm25 + λ2*n.vec + λ3*rank[n] + λ4*weight(n)
pack = budget_pack(topk_by_final(subgraph.nodes), token_budget)
return summarize_with_citations(pack)
```

---

## 6. 权重、衰减与分层巩固

### 6.1 权重函数（节点级）

记忆“易取度”权重：

```
weight(node) = α*importance
             + β*log(1+frequency)
             + γ*recency_decay(t_now - t_node)
             + δ*emotion_strength
             + ε*user_feedback  # pin/upvote
             - ζ*staleness_penalty
```

- `recency_decay(Δt) = exp(-Δt / τ)`，τ 可按类型设定（Episode 短、Fact 中、Concept 长）。

### 6.2 记忆分层与转移策略（短→中→长期）

> 运行由 **PromoterJob**（每日/每 6h 可配）扫描并执行迁移；迁移产生 `DERIVED_FROM` 与版本映射。

**短期（0–7 天）**

- **存储**：原始对话片段（完整保留），含可引用锚点与元数据。
- **留存/淘汰触发**：在 0–7 天窗口内；访问频次（被检索/引用）≥ 1 次延长 3 天保护期。
- **迁移到中期触发**：`age ≥ 7d` **或** 主题重复（同一 ABOUT/Concept 或主题聚类）累计 ≥ 3 次。

**中期（7–30 天）**

- **存储**：摘要化 + 向量化（如可用）；保留关键 Episode 的最小证据集。
- **迁移到长期触发**：
  - 出现**稳定知识模式**：跨 ≥ 2 个会话/场景一致，
  - 且 `confidence ≥ θ_conf`（默认 0.8），
  - 且来自 ≥ 2 种不同来源/证据路径（多源一致）。

**长期（30 天 +）**

- **存储**：概念化/模式化（Concept/Fact），保留 `DERIVED_FROM` 指向中期/短期证据。
- **降级/归档**：若长期未被访问且主题过时，权重趋零后转冷存，仅保留概念摘要与最强引用路径。

### 6.3 巩固（Consolidation，智能触发）

- **触发机制**：
  1. **语义聚类**：基于向量相似（如可用，HDBSCAN/连通分量阈值）或基于关键词/co-occurrence 的无向图聚类（向量不可用时）。
  2. **时间模式识别**：在滑动窗口中识别周期性/趋势性（如每周重复的偏好/任务）。
  3. **睡眠巩固（离线批处理）**：低峰期批量重算聚类、概念中心（centroid）、PPR 中心度与引用路径简化。
- **产出**：生成/更新 Concept；合并重复 Facts，保持 `DERIVED_FROM` 与来源置信度汇总。

### 6.4 遗忘/归档（Decay/GC）与选择性遗忘

- **时间衰减**：按类型应用 `τ`；低权重节点进入候选集合；
- **归档**：将候选节点的原始片段移动到冷存，仅保留摘要与引用路径；
- **选择性遗忘（用户/策略触发）**：API 指定节点/子图删除：撤回倒排/向量/图索引→对象存储→写墓碑；支持租户级“被遗忘权”。

---

## 7. 冲突检测与一致性（版本链与自动验证）

### 7.1 版本链（Version Chain）

- **Fact** 节点不再直接覆盖，而是维护 **版本链**：`Fact(v1) -[VERSION_NEXT]-> Fact(v2) -[...]`，每个版本包含 `valid_since/valid_until`。
- 引入 `ACTIVE_OF` 边指向当前生效版本，历史可追溯；所有引用默认指向 `ACTIVE_OF`，审计可切换历史视图。

### 7.2 矛盾建模与优先级规则

- 当同一主体/属性出现冲突时，建立双向 `CONTRADICTS`；并记录冲突维度（时间/来源/数值）。
- **冲突优先级评分**：

```
prio(fact) = a*source_trust + b*recency_score + c*evidence_support + d*consistency
```

- `source_trust`：来源可信度（可配置白名单/信誉分）。
- `recency_score`：新近版本加权。
- `evidence_support`：支持该版本的 Episode/引用路径数量及权重。
- `consistency`：与其他高置信事实/概念的一致度。
- **决策**：将最高 `prio` 版本设为 `ACTIVE_OF`，低分版本保留在链上；当分差低于阈值触发**人工/Agent 审核队列**。

### 7.3 自动事实验证 Pipeline（可选/可关闭）

- **VerifierJob**：
  1. 汇聚内部证据（图内支持/反证路径）；
  2. （可选）调用外部校验器/规则库（可禁用以零外部依赖）；
  3. 更新 `confidence`、`source_trust`、`prio`；必要时重排 `ACTIVE_OF`。
- **可观测性**：产出 `ConflictDetected`、`FactPromoted/Demoted` 事件，供审计与回滚。

---

## 8. 安全与隐私

- **PII 检测与脱敏**：识别邮箱/电话/地址等；
- **按租户/用户隔离**：每个 `tenant_id` 数据与索引物理/逻辑隔离；
- **加密**：传输 TLS、静态加密（KMS）；
- **可删除性**：满足“被遗忘权”——软删→索引撤回→冷存删除；
- **最小化暴露**：回忆输出仅暴露必要片段及可追踪引用。

---

## 9. 接口设计（对外 API / 供其他 Agent 调用）

### 9.1 REST/gRPC（示例 JSON）

- **POST /memory/nodes**（创建/更新节点）

```json
{
  "tenant_id": "t1",
  "node": {"type": "Episode", "text": "...", "keywords": ["..."], "importance": 0.6}
}
```

- **POST /memory/edges**（创建边）

```json
{"tenant_id":"t1","edge": {"src":"idA","dst":"idB","type":"ABOUT","weight":0.7}}
```

- **POST /memory/ingest**（对话摄取）

```json
{"tenant_id":"t1","turn": {"input":"...","output":"...","meta": {"lang":"zh-CN"}}}
```

- **POST /memory/recall**（回忆）

```json
{
  "tenant_id": "t1",
  "query": {"text": "用户问：如何修蓝牙？", "timebox_days": 30},
  "budget": {"tokens": 2000},
  "options": {"k_hop": 2, "need_citations": true}
}
```

- **POST /memory/consolidate**（触发巩固）

```json
{"tenant_id":"t1","strategy":"community_detect","params":{"min_cluster":5}}
```

- **POST /memory/decay**（触发衰减/归档）

```json
{"tenant_id":"t1","policy":"default_v1"}
```

- **POST /feedback**（用户/Agent 反馈）

```json
{"tenant_id":"t1","node_id":"n123","action":"pin|upvote|downvote"}
```

### 9.2 C++ 内部接口（可插拔/可关闭）

> 以纯虚接口定义模块边界；插件通过 C ABI 导出，支持运行时装载/关闭。

```cpp
// === 公共模型 ===
struct RecallQuery { std::string text; std::vector<std::string> keywords; size_t token_budget{2000}; int k_hop{2}; };
struct ScoredId { uint64_t id; float score; };

// === 倒排索引（必备，可单独运行） ===
class ISearchIndex {
public:
  virtual ~ISearchIndex() = default;
  virtual void Upsert(uint64_t docId, std::string_view text, std::span<const uint32_t> fields = {}) = 0;
  virtual void Remove(uint64_t docId) = 0;
  virtual std::vector<ScoredId> Search(std::string_view query, size_t topk) const = 0;
  virtual void Flush() = 0;
};

// === 向量索引（可选，可关闭） ===
class IVectorIndex {
public:
  virtual ~IVectorIndex() = default;
  virtual void Upsert(uint64_t id, std::span<const float> vec) = 0;
  virtual void Remove(uint64_t id) = 0;
  virtual std::vector<ScoredId> Search(std::span<const float> q, size_t topk) const = 0;
  virtual void Flush() = 0;
};

// 插件工厂（C ABI）
extern "C" {
  IVectorIndex* CreateVectorIndex();
  void DestroyVectorIndex(IVectorIndex*);
}

// === 图存储与遍历（必备） ===
struct Edge { uint64_t src, dst; uint16_t type; float weight; };
class IGraphStore {
public:
  virtual ~IGraphStore() = default;
  virtual uint64_t AddNode(std::string_view label) = 0;
  virtual void     AddEdge(const Edge& e) = 0;
  virtual std::vector<uint64_t> KHopSeeds(const std::vector<uint64_t>& seeds, int k, std::span<const uint16_t> types) const = 0;
  virtual std::vector<ScoredId> PersonalizedPageRank(const std::vector<uint64_t>& seeds, int k_hop) const = 0;
};

// === 召回/重排管线（统一调度，感知降级） ===
class IRecallPipeline {
public:
  virtual ~IRecallPipeline() = default;
  virtual std::vector<ScoredId> Run(const RecallQuery& q) = 0; // 内部按配置选择向量/关键词/图扩散
};
```

**模块开关**（编译/运行时）：

- 编译：`-DENABLE_VECTOR`, `-DENABLE_SEARCH`, `-DENABLE_GRAPH`
- 运行：`vector.enabled=false` 即时关闭；召回管线自动降级。

### 9.3 事件总线（Event）

- `Memory.NodeCreated`, `Memory.EdgeCreated`, `Memory.WeightUpdated`, `Memory.Consolidated`, `Memory.Archived`, `Memory.ConflictDetected`

---

## 10. 多 Agent 协作与职责

- **Ingestion Agent**：对话切分、清洗、结构化。
- **Summarizer Agent**：生成 Episode/Fact/Preference 节点草稿。
- **Linker Agent**：主题归属、实体链接、相似性/因果/支持/矛盾判定。
- **Embedder Agent**：多路 embedding 生成与注册。
- **Indexer Service**：写入 vector 与 search 索引。
- **Retriever/Ranker Agent**：三阶段召回 + 图扩散 + 重排 + 打包。
- **Consolidator Agent**：聚类、抽象概念生成、去重与版本化。
- **Decay/GC Service**：权重计算、归档与清理。
- **Privacy Guardian**：PII 检测与策略执行。
- **Evaluator Agent**：离线/在线评估、指标上报。

> 主持协作的 Orchestrator 负责 DAG 调度与重试，确保幂等与 Exactly-once 语义（业务层幂等键）。

---

## 11. 评估指标（KPIs）

- **召回质量**：Recall\@k、NDCG、MMR 多样性、冲突率/矛盾率可控。
- **回答效果**：任务成功率、人工评审分、引用可追踪率、幻觉率。
- **性能**：P50/P95/P99 延迟、检索 QPS、写入吞吐、成本/GB。
- **演化**：巩固覆盖率（被概念化的比例）、冷存占比、权重分布稳定性。

### 11.1 运行时监控指标（Metrics）

```yaml
metrics:
  storage:
    - graph_density        # 图密度（|E| / |V| 或 avg_degree = 2|E|/|V|，分类型与层级统计）
    - index_fragmentation  # 索引碎片率（无效段占比、posting/邻接块空洞率）
    - compaction_ratio     # 压缩比（compact 前后段大小比）
  retrieval:
    - cache_hit_rate       # 候选/邻接/向量块缓存命中率
    - query_latency_p99    # 查询延迟 P99
    - recall_precision     # 线下标注集上的召回/精度
```

- **告警建议**：
  - `graph_density` 超阈（如 avg\_degree > 500 for SIMILAR\_TO）→ 触发稀疏化；
  - `index_fragmentation` > 0.25 → 触发合并/重排；
  - `query_latency_p99` 超 SLO（如 > 1.5s）→ 扩容或启用分层索引；
  - `cache_hit_rate` < 0.7 → 调整热点缓存大小或提升局部性。

---

## 12. 实施计划（Milestones）

### MVP

- 存储：Postgres+pgvector（向量）+ 简化图表（`nodes/edges` 两表 + GIN）
- 能力：摘要→向量→关键词→k=1 扩散；基本权重；引用打包
- 指标：P95 回忆 < 1.5s，Recall\@20 > 0.7（内部基准集）

### v1

- 引入真正图数据库（Neo4j/Nebula）、OpenSearch，PPR 扩散，巩固流水线
- 隐私/审计、冲突视图、事件总线

### v2

- 多模态记忆（图像/音频 embedding）、跨任务共享概念层、主动复习与推荐

---

## 13. 开发细节建议

- **语言/框架**：C++，gRPC 优先，REST 兼容。
- **嵌入维度**：文本 768\~1536，情绪 32\~128（可插拔）；统一以 `embeddings[]` 注册。
- **索引策略**：按 type 分库/分索引；Episode 大量、Concept 稀疏高权。
- **分片**：按 `tenant_id` + 时间范围；冷热分层存储。
- **幂等**：node 外部 key（如 source\_id+offset）→ upsert。
- **可观测性**：OpenTelemetry 埋点；结构化日志包含 `tenant_id`、`query_id`、pipeline\_stage。

---

## 14. 示例（端到端）

**输入**：多次关于“Win11 蓝牙无法开启”的对话。

**生成节点**：

- Episode：用户描述症状；
- Fact：BTHUSB EventID=17 与驱动异常相关；
- Preference：用户偏好中文回答；
- Concept："Win11 蓝牙故障排查"；
- Entity："Windows 11"，"BTHUSB"。

**链接**：Episode → ABOUT → Concept；Episode → MENTIONS → Entity；Fact ↔ SUPPORTS ↔ Concept。

**回忆**：用户再次询问蓝牙问题→关键词（"蓝牙", "Win11"）+ 向量召回→ 子图扩散（k=2）→ 返回概念摘要 + 关键步骤 + 引用路径（DERIVED\_FROM）打包至 1500 tokens。

---

## 15. 附录 A：默认超参（建议值）

- `λ1=0.2, λ2=0.4, λ3=0.3, λ4=0.1`（需 A/B 调参）
- `τ_Episode=14d, τ_Fact=90d, τ_Concept=180d`
- MMR 多样性阈值 `0.7`；ANN TopK：Episode=200，Fact=150，Concept=100
- 图扩散 k=2，边权：ABOUT=1.0、SUPPORTS=0.8、SIMILAR\_TO=0.5、TEMPORAL\_NEXT=0.3

---

## 16. 附录 B：Agent 提示词骨架（片段）

**Summarizer Agent**：

- 目标：生成简明 Episode/Fact/Preference；保留可引用原文锚点；抽取实体/时间/情绪。

**Linker Agent**：

- 目标：判定 ABOUT/MENTIONS/CAUSES/SUPPORTS/CONTRADICTS；产出置信度与边权；避免过密图。

**Retriever/Ranker Agent**：

- 目标：执行 A/B/C 三阶段检索与重排；在预算内打包最有力证据与摘要；保证引用可追踪。

**Consolidator Agent**：

- 目标：社区检测→生成/更新 Concept；事实对齐去重；维护 DERIVED\_FROM。

---

## 17. 风险与对策

- **图过密**：度数限制、稀疏化、按权重截断；
- **误链路**：边置信度门限、人工/Agent 审核队列；
- **冷启动**：偏向关键词+近因；早期提升用户反馈权重；
- **性能瓶颈**：分层召回、缓存热概念、分片与只读副本；
- **隐私风险**：PII 识别与最小披露策略、强制删除流程、租户隔离审计。

---

> 本文档为 v1，可在实现过程中以变更日志记录参数/接口演进；所有 Agent 与服务实现需遵循本文的 Schema 与协议。

---

## 18. 降级策略与能力矩阵

| 场景                | 可用能力            | 召回策略              | 打分公式变体                                    |
| ----------------- | --------------- | ----------------- | ----------------------------------------- |
| **全部可用**          | BM25 + 向量 + 图扩散 | A(关键词)→B(向量)→C(图) | `λ1*BM25 + λ2*Vec + λ3*Graph + λ4*Weight` |
| **Embedding 不可用** | BM25 + 图扩散      | A→C               | `λ1*BM25 + λ3*Graph + λ4*Weight`          |
| **向量索引未就绪/关闭**    | BM25 + 图扩散      | A→C               | 同上                                        |
| **图存储维护中**        | BM25 + 向量       | A→B               | `λ1*BM25 + λ2*Vec + λ4*Weight`            |
| **仅关键词可用（极端）**    | BM25            | A                 | `BM25 + 近因/权重`                            |

> 召回管线实时感知模块可用性与健康度（心跳/自检），自动选择路径并记录 Trace。

---

## 19. 构建与依赖策略

- **C++20**，尽量只使用标准库；可选三方：`protobuf`（API）、`fmt`（日志）、`absl`（可选）。
- **跨平台**：Linux x86\_64/arm64 优先；Windows 次之（使用 `mmap`/`CreateFileMapping` 适配层）。
- **无外部存储依赖**：所有索引/存储为本地段文件；支持后续扩展为共享存储。

---

## 20. 测试与验收

- **一致性/恢复**：随机故障注入（崩溃、断电）后 WAL 恢复正确性；
- **检索正确性**：BM25/NDCG、向量 Recall\@k、PPR 稳定性；
- **性能**：导入吞吐、P50/P95 延迟、内存占用（HNSW/邻接）；
- **降级验证**：逐项关闭模块，验证结果质量与可观测性；
- **基线数据集**：内部构造 1e6 级节点/边与 1e6 文档向量的合成基准。

---

## 21. 实施建议（针对实现 Agent）

1. **MVP 顺序**：先实现倒排索引 + 图存储（k=1 邻接）→ 再接 HNSW → 最后补齐 PPR 与巩固。
2. **持久化优先**：所有索引先以段式 append-only + WAL 实现 crash-safe；再做 compaction。
3. **接口优先**：先固化 9.2 接口，插件用 `.so/.dll` 动态装载；流水线以配置驱动。

---

## 22. 图稀疏化与分层索引（Hot/Cold 与自动优化）

### 22.1 稀疏化策略

1. **度上限（per-type cap）**：对高出度节点按边类型设上限 `k_type`（如 ABOUT=256，SIMILAR\_TO=64），保留 `weight×recency` Top-K，余者标记为冷边移入边冷存（可按需回页）。

2. **年龄-置信剪枝**：对 `age` 高且 `confidence` 低、`weight` 低的边，优先剪枝；

3. **相似性阈值**：`SIMILAR_TO` 仅保留相似度 ≥ θ（默认 0.75），并限制三角闭包（避免团簇过密）。

4. **证据折叠**：多条等价支持路径折叠为“证据包”，以减少冗余边。

5. **主题扇出控制**：ABOUT 边优先保留至中心概念（高中心度 Concept），对长尾 Concept 降采样。

### 22.2 分层图索引（Hierarchical Graph Index）

- **L0（原始层）**：完整邻接；仅用于离线巩固与审计。
- **L1（剪枝层）**：按 22.1 规则稀疏化的运行层，作为在线检索默认；
- **L2（概念层）**：以 Concept/Entity/Task 为“超节点”，边为聚合关系（支持/矛盾/因果），用于快速多跳/长径路检索；
- **跳转规则**：在线召回优先在 L2 做全局定位→回落到 L1 精化→必要时 L0 局部回填。

### 22.3 热点数据管理（Hot Set）

- **Hot-Node Cache**：维护高访问/高中心度节点的邻接块 LRU；
- **Pin 列表**：将关键 Concept/Preference 置顶常驻；
- **局部性优先**：优先同租户/同主题分区；邻接块按访问时间排序以便顺序 IO。

### 22.4 自动优化（GraphOptimizer）

- **输入**：第 11.1 节 metrics；
- **策略**：当 `graph_density`、`fragmentation`、`latency_p99` 超阈，触发：
  - 稀疏化/重写 L1；
  - 重算 L2 汇总边权；
  - 对热点分区重建邻接块、扩大缓存；
  - 调整各 `k_type`、相似阈值、PPR 迭代步数。
- **伪代码**：

```python
if avg_degree(SIMILAR_TO) > cap: prune_similar_to(cap)
if index_fragmentation > 0.25: run_compaction()
if p99_latency > slo: promote_L2_ratio(), enlarge_hot_cache()
```

---

## 23. 增强巩固算法与“睡眠巩固”

- **语义自动聚类**：
  - 向量可用：密度/社区检测（HDBSCAN/Label Prop），中心向量更新，低置信边下调；
  - 向量不可用：基于关键词与实体共现构造共现图做社区发现；
- **时间模式识别**：计算周期性指标（自相关/周期评分）识别习惯与规律，提升相关 Preference/Task 权重；
- **睡眠巩固批**：每日低峰运行：重算中心度、重链 `DERIVED_FROM` 最短证据路径、压缩冗余引用、刷新 L2 概念层。

---

## 24. 隐私增强与差分隐私

- **差分隐私（DP）**：对跨会话/跨用户聚合的统计与推荐引擎加入拉普拉斯/高斯噪声（可配置 ε, δ），对单租户内部可关闭；
- **选择性遗忘**：
  - **点删除**：按 `node_id/edge_id` 精确删除并回收所有索引；
  - **主题删除**：按 Concept 子图范围删除；
  - **时间窗删除**：按时间范围批量删除 Episode；
- **可追踪删除**：为删除任务生成审计记录与 Proof（索引撤回列表 + Blob 物理删除记录）。

---

---

## 25. 实施路径（单人业余开发版）

> 面向一名开发者的可落地路线；以“最小可用→可观测→可优化”的顺序推进。每一步都有**完成定义（DoD）****与****可验证产物**，便于在零碎时间推进。

### 25.1 总体推进策略

- **优先级序**：倒排索引（BM25）→ GraphStore（k=1 邻接）→ 召回管线（A→C）→ 度上限稀疏化 → L2 概念层 → 向量索引（可选）→ 巩固与版本链 → 自动优化与隐私增强。
- **工作粒度**：将任务切为 **独立可运行的小步**，保证每次提交能通过单元/集成测试与 CLI 演示。
- **分支策略**：`main`（可用）、`feat/*`（单步），以 PR 合并；每步更新 `MANIFEST` 与迁移脚本。

### 25.2 里程碑与 DoD

**M0 — 项目骨架与工具链**

- 产出：CMake 工程、模块目录结构、基础日志与配置、简单 CLI。
- DoD：`./memctl --help` 可运行；空实现通过单测框架。

**M1 — 自研倒排索引 MVP（SearchIndex）**

- 产出：分词（占位/简化）、字典与 postings 段、WAL+快照、BM25 检索、Flush/Load。
- DoD：`memctl index --add sample.txt && memctl search "蓝牙"` 返回 TopN，重启后结果一致；崩溃恢复测试通过。

**M2 — GraphStore MVP（节点/边/邻接）**

- 产出：节点/边段文件、出入度索引、k=1 邻接读取 API、WAL 与快照。
- DoD：`memctl graph --add-node --add-edge --neighbors id` 正确返回；重启一致；百万级（合成）导入通过。

**M3 — 召回管线 A→C（无向量）**

- 产出：RecallPipeline 可配置；阶段 A（BM25）召回，阶段 C（k=1 扩展 + 简易得分）。
- DoD：`memctl recall --q "Win11 蓝牙" --budget 1500` 返回打包摘要（占位）与引用路径；可观测 Trace。

**M4 — 图稀疏化（度上限/相似阈值占位）与 L1/L2 结构雏形**

- 产出：按边型 Top-K 剪枝、冷边归档标记；L2 概念层的数据结构与构建脚本（简化，先手工指定 Concept）。
- DoD：`memctl graph --prune --cap 256` 生效；`recall` 走 L2→L1 路径可观测；密度指标上报。

**M5 — 版本链与矛盾处理（最小集）**

- 产出：VERSION\_NEXT / ACTIVE\_OF 边；冲突优先级评分（来源/新近/证据）。
- DoD：构造冲突数据集，`ACTIVE_OF` 切换正确，审计视图展示版本链。

**M6 — 巩固与 PromoterJob（短→中→长期迁移）**

- 产出：PromoterJob 定时遍历；短中长期的迁移规则落地；睡眠巩固批（简化版）。
- DoD：导入样本后，达到触发条件自动生成/更新 Concept 与摘要，DERIVED\_FROM 链条完整。

**M7 — 自动优化与监控告警**

- 产出：GraphOptimizer 读取指标触发稀疏化/重建；metrics 导出（/metrics 文本或 JSON）。
- DoD：调高负载/密度可触发自动动作；日志与指标记录全部链路。

**M8 — 向量索引（HNSW，可选，可关闭）**

- 产出：IVectorIndex 插件 + 持久化；召回管线 A→B→C；降级可测。
- DoD：向量开启时提升召回覆盖；关闭后回退 A→C，结果稳定。

### 25.3 模块与目录建议

```
/memory
  /core        // 公共模型、配置、日志、errors
  /search      // 倒排索引（段、WAL、快照、BM25）
  /graph       // 节点/边段、邻接、稀疏化、L2
  /vector      // HNSW（可选，插件式）
  /pipeline    // RecallPipeline、重排、打包、降级
  /jobs        // PromoterJob、Consolidator、Optimizer
  /cli         // memctl：index/graph/recall/metrics
  /tests       // 单测 + 集成 + 基准
  /tools       // 数据生成器、故障注入脚本
```

### 25.4 每步可执行清单（适合碎片时间）

- **实现类** → **最小单测** → **集成 CLI 子命令** → **崩溃恢复/持久化测试** → **记录指标**。
- 每完成一小步，更新 `CHANGELOG.md` 与 `MANIFEST`，保证可回滚。

### 25.5 样例命令（演示/自检）

```bash
# 索引若干文本
memctl index --add ./samples/*.txt
# 构建/加载图
memctl graph --add-node "Concept:Win11蓝牙故障" --add-edge ...
# 回忆（仅 BM25 + 图扩展）
memctl recall --q "Win11 蓝牙 开不了" --budget 1500 --trace
# 稀疏化与层级检索
memctl graph --prune --cap 256
memctl recall --q "蓝牙 事件ID 17" --use-l2 --trace
# 版本链与审计
memctl fact --add v1 ...
memctl fact --add v2 ... --conflicts v1
memctl audit --fact <id> --history
# 迁移与巩固（离线批）
memctl job --run promoter --dry-run
memctl job --run consolidate
# 指标
memctl metrics --dump
```

### 25.6 风险与回避（单人版）

- **过度设计**：在 M3 之前不引入向量与 PPR，确保先有“能用”的 Recall。
- **一致性风险**：严格先 WAL 后段写，任何新段生效必须在快照中；提供 `fsync` 注入测试。
- **时间被打断**：任务颗粒保持最小，避免跨多次会话的巨块工作；每次提交保持可运行。
- **数据迁移**：`MANIFEST` 记录格式版本；提供 `memctl migrate` 与兼容读取。

### 25.7 验收样本与基准

- 合成数据：≥ 1e6 节点/边，主题 1e3，文档 1e5；
- 真实样本：选取你历史对话若干主题（10–50 条），用于检索效果观察与规则调参；
- 记录基线：P50/P95/P99 延迟、graph\_density、recall\_precision、cache\_hit率。

> 注：如日程紧张，可在 M3 之后直接投入使用（仅 BM25+图扩展），后续逐步补全 M4–M7；M8（向量）完全可延后或长期禁用。

