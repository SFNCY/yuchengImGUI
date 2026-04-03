# Dual Contouring 四叉树算法学习项目

## TL;DR

> **快速摘要**：实现一个 2D 四叉树版本的 Dual Contouring 算法，使用 ImGui 进行分步可视化，帮助理解算法原理。
>
> **交付物**：
> - `demo/dual_contouring/` 目录下的完整可运行演示程序
> - 四叉树空间分割结构可视化
> - QEF（二次误差函数）求解过程可视化
> - 最终对偶网格生成与渲染
> - 可交互的 SDF 形状编辑器和参数控制面板
>
> **预计工作量**：中等（6-8 个任务，预计 1-2 天开发）
> **并行执行**：是（3 个波次）
> **关键路径**：任务 1 → 任务 4 → 任务 6 → 任务 7 → 任务 8

---

## 背景

### 原始需求
用户希望学习 Dual Contouring 算法，请求实现一个四叉树版本并使用 ImGui 可视化，代码存放在 demo 新建文件夹下，侧重于原理理解。

### 访谈总结
**已确认的决策**：
- **2D 版本**：选择 2D 四叉树而非 3D 八叉树，降低复杂度便于学习
- **全部分步可视化**：同时显示四叉树结构、QEF 求解过程、最终网格
- **可编辑密度场**：用户可添加基本形状（圆形、方形等）创建 SDF 场景
- **参数可调**：四叉树深度、细分阈值等算法参数可实时调整
- **教学导向**：清晰的代码结构，详细的原理注释，适合学习

**研究结论**：
- Dual Contouring 源于 SIGGRAPH 2002，用于从体素数据重建网格
- 核心思想：在八叉树/四叉树节点中存储 Hermite 数据，用 QEF 找到最佳顶点位置
- 与 Marching Cubes 的区别：在单元格中心生成顶点而非边缘，实现对偶表示

### Metis 审查
**识别的缺口**（已处理）：
- QEF 求解器使用 SVD 分解而非简单矩阵求逆，确保数值稳定性
- 显式定义验收标准和排除项
- 可执行验证标准使用 ImDrawList 渲染

---

## 工作目标

### 核心目标
实现一个完整的 2D 四叉树 Dual Contouring 算法学习工具，分步可视化算法各阶段，帮助用户深入理解原理。

### 具体交付物
- `demo/dual_contouring/main.cpp` - 主程序入口
- `demo/dual_contouring/dc_sdf.h/cpp` - 符号距离函数（SDF）实现
- `demo/dual_contouring/dc_quadtree.h/cpp` - 四叉树数据结构
- `demo/dual_contouring/dc_hermite.h/cpp` - Hermite 数据存储
- `demo/dual_contouring/dc_qef.h/cpp` - QEF 求解器（SVD 分解）
- `demo/dual_contouring/dc_mesh.h/cpp` - 对偶网格生成
- `demo/dual_contouring/dc_renderer.h/cpp` - ImDrawList 渲染器
- `demo/dual_contouring/dc_app.h/cpp` - ImGui 交互界面

### 完成定义
- [ ] 圆形 SDF 渲染为闭合曲线（圆形）
- [ ] 方形 SDF 渲染为四边形
- [ ] 四叉树在表面穿越处正确细分
- [ ] QEF 解算点在单元格边界内
- [ ] 最终网格无自相交
- [ ] ImGui 面板显示：树深度、顶点数、边数
- [ ] 参数变更实时反映在可视化中

### 必须有
- SDF 基本形状（圆形、方形）
- 四叉树细分逻辑
- QEF 求解器（数值稳定）
- ImGui 参数控制面板
- 分步/连续可视化切换

### 严禁出现（防护栏）
- 3D 八叉树实现
- 网格文件导出功能
- 纹理/材质系统
- 碰撞检测

---

## 验证策略（强制要求）

### 零人工干预原则
所有验收标准必须可由智能体执行，无需人工确认。

### 测试决策
- **基础设施**：项目无正式测试框架，使用手动验证 + ImDrawList 可视化
- **自动化测试**：无
- **智能体执行 QA**：每个任务包含智能体执行 QA 场景

### QA 政策
每个任务必须包含智能体可执行的 QA 场景，证据保存在 `.sisyphus/evidence/` 目录。

- **前端/UI**：使用 Playwright 截图验证渲染结果
- **TUI/CLI**：不适用
- **API/后端**：不适用
- **库/模块**：通过编译测试和运行时断言验证

---

## 执行策略

### 并行执行波次

> 通过将独立任务分组到并行波次来最大化吞吐量。
> 每个波次完成后才开始下一个波次。
> 目标：每个波次 5-8 个任务，少于 3 个任务（除最终波次外）= 拆分不足。

```
波次 1（立即启动 — 基础 + 脚手架）：
├── 任务 1: 项目脚手架 + CMake 配置
├── 任务 2: SDF 基础实现（圆形、方形）
├── 任务 3: 四叉树数据结构
└── 任务 4: 基础渲染框架（ImDrawList）

波次 2（第 1 波完成后 — 核心算法）：
├── 任务 5: Hermite 数据存储
├── 任务 6: QEF 求解器（SVD 分解）
├── 任务 7: Dual Contouring 核心算法
└── 任务 8: 网格生成与渲染

波次 3（第 2 波完成后 — UI + 集成）：
├── 任务 9: ImGui 控制面板
├── 任务 10: 交互式形状编辑器
├── 任务 11: 分步可视化系统
└── 任务 12: 参数实时调节

波次 FINAL（所有任务完成后 — 4 并行审查）：
├── 任务 F1: 计划合规审计 (oracle)
├── 任务 F2: 代码质量审查 (unspecified-high)
├── 任务 F3: 真实手动 QA (unspecified-high)
└── 任务 F4: 范围保真度检查 (deep)
```

### 依赖矩阵（完整）

| 任务 | 依赖 | 被阻塞 | 阻塞 |
|------|------|--------|------|
| 1 | - | 2, 3, 4 | 2, 3, 4 |
| 2 | - | 5, 7 | 5, 7 |
| 3 | - | 5, 6, 7 | 5, 6, 7 |
| 4 | - | 8, 9, 10 | 8, 9, 10 |
| 5 | 2, 3 | 6, 7 | 6, 7 |
| 6 | 3, 5 | 7 | 7 |
| 7 | 2, 3, 5, 6 | 8, 11 | 8, 11 |
| 8 | 4, 7 | 9, 10, 11 | 9, 10, 11 |
| 9 | 4, 8 | 12 | 12 |
| 10 | 4, 8 | 12 | 12 |
| 11 | 7, 8 | 12 | 12 |
| 12 | 9, 10, 11 | F1, F2, F3, F4 | - |

**关键路径**：任务 1 → 任务 3 → 任务 5 → 任务 6 → 任务 7 → 任务 8 → 任务 12 → F1-F4
**并行加速**：约 60% 更快（相比顺序执行）
**最大并发**：4 个任务（波次 1）

---

## TODOs

> 实现 + 测试 = 同一个任务。永不分离。
> 每个任务必须包含：推荐的智能体配置文件 + 并行化信息 + QA 场景。
> 没有 QA 场景的任务是不完整的。绝不例外。

- [x] 1. 项目脚手架 + CMake 配置

  **实现步骤**：
  - 创建 `demo/dual_contouring/` 目录结构
  - 复制并修改 `demo/opengl/CMakeLists.txt`
  - 创建 `main.cpp` 入口文件，基础 GLFW + ImGui 初始化
  - 在 `demo/CMakeLists.txt` 中添加 `add_subdirectory(dual_contouring)`
  - 创建 `README.md` 标记"开发中"

  **禁止事项**：
  - 不要修改 `3rdlibs/` 目录
  - 不要修改根目录的 CMake 配置

  **推荐智能体配置文件**：
  > 选择类别 + 技能的依据是任务领域。仅论证每个选择。
  - **类别**：`quick`（简单文件创建，无复杂逻辑）
    - 理由：主要是复制和轻微修改文件，结构简单
  - **技能**：`无`
    - 无需特殊技能
  - **评估但省略的技能**：
    - `git-master`：无需 git 操作

  **并行化**：
  - **可并行运行**：是
  - **并行组**：波次 1（与任务 2, 3, 4）
  - **阻塞**：任务 5, 6, 7
  - **被阻塞**：无（可立即开始）

  **引用**（关键 — 执行者没有我的访谈上下文。引用是他们的唯一指南）：

  **模式引用**（现有代码模式）：
  - `demo/opengl/main.cpp:1-50` - GLFW + ImGui 初始化模式（复制此结构）
  - `demo/opengl/CMakeLists.txt:1-23` - CMake 子项目配置模式
  - `demo/CMakeLists.txt:1-7` - 添加子目录的方式

  **类型/接口引用**（实现的契约）：
  - 无

  **测试引用**（测试模式）：
  - 无（此项目无正式测试框架）

  **外部引用**（库和框架）：
  - GLFW 文档：`https://www.glfw.org/docs/latest/intro_guide.html` - 窗口创建
  - ImGui 文档：`https://dearimgui.com/docs` - 初始化和主循环

  **每个引用的重要性**（解释相关性）：
  - 不要只列出文件 — 解释执行者应从中提取什么模式/信息
  - 差：`demo/opengl/main.cpp`（模糊，是什么 utils？为什么？）
  - 好：`demo/opengl/main.cpp:40-50` - 使用此初始化序列进行 GLFW 配置

  **验收标准**：

  > **智能体可执行验证** — 不允许人工操作。
  > 每个标准必须可通过运行命令或使用工具验证。

  - [ ] 目录 `demo/dual_contouring/` 存在
  - [ ] 文件 `demo/dual_contouring/main.cpp` 存在且编译通过
  - [ ] 文件 `demo/dual_contouring/CMakeLists.txt` 存在
  - [ ] `demo/CMakeLists.txt` 包含 `add_subdirectory(dual_contouring)`
  - [ ] 运行 `compile.bat` 后生成 `demo/dual_contouring/demoDualContouring.exe`

  **QA 场景**（强制 — 无 QA 场景的任务将被拒绝）：

  > **这不是可选的。没有 QA 场景的任务是不完整的。**
  >
  > 编写验证实际行为的场景测试。
  > 最少：每个任务 1 个 happy path + 1 个失败/边缘情况。
  > 每个场景 = 精确工具 + 精确步骤 + 精确断言 + 证据路径。
  >
  > **执行者必须在实现后运行这些场景。**
  > **编排者在标记任务完成前会验证证据文件存在。**

  ```
  场景：项目骨架编译成功
    工具：Bash
    前提条件：全新克隆，无先前构建
    步骤：
      1. 运行 `compile.bat` 在 Windows 上
      2. 等待编译完成
    预期结果：编译成功，生成 `demo/dual_contouring/demoDualContouring.exe`
    失败指示：编译错误、链接失败
    证据：.sisyphus/evidence/task-1-compile-success.txt
  ```

  **证据捕获**：
  - [ ] 每个证据文件命名为：task-{N}-{scenario-slug}.{ext}
  - [ ] 截图用于 UI，终端输出用于 CLI，响应体用于 API

  **提交**：是
  - 消息：`feat(demo): 添加 dual contouring 学习演示骨架`
  - 文件：`demo/dual_contouring/main.cpp`, `demo/dual_contouring/CMakeLists.txt`, `demo/CMakeLists.txt`

---

- [x] 2. SDF 基础实现（圆形、方形）

  **实现步骤**：
  - 创建 `dc_sdf.h` 头文件
  - 实现 `SDFCircle` 结构体：圆心 + 半径
  - 实现 `SDFSquare` 结构体：中心 + 边长
  - 实现 `SDFUnion` 用于组合多个 SDF
  - 实现 `SampleSDF(vec2 point)` 返回点到表面距离
  - 实现 `EvaluateSDF(vec2 point, SDF* sdf)` 计算给定点的 SDF 值

  **禁止事项**：
  - 不要实现复杂形状（多边形、自由曲线）
  - 不要实现 SDF 布尔运算（除 union 外）

  **推荐智能体配置文件**：
  - **类别**：`quick`（数学库实现，模式简单）
    - 理由：SDF 是标准数学公式，实现直接
  - **技能**：`无`

  **并行化**：
  - **可并行运行**：是
  - **并行组**：波次 1（与任务 1, 3, 4）
  - **阻塞**：任务 5, 7
  - **被阻塞**：无

  **引用**：

  **模式引用**：
  - `demo/opengl/main.cpp:116-118` - ImVec4 颜色定义模式

  **外部引用**：
  - Inigo Quilez SDF 文档：`https://iquilezles.org/articles/distfunctions2d/` - 2D SDF 公式

  **验收标准**：

  - [ ] `dc_sdf.h` 包含 `SampleSDF(vec2, SDF*)` 函数
  - [ ] `SampleSDF((0,0), Circle(0,0,1))` 返回 `±1.0`（取决于内外定义）
  - [ ] `SampleSDF((0,0), Square(0,0,2))` 返回 `±1.0`
  - [ ] 代码编译无错误

  **QA 场景**：

  ```
  场景：SDF 圆形距离计算正确
    工具：Bash (g++ 编译测试)
    前提条件：SDF 实现完成
    步骤：
      1. 创建测试程序调用 `SampleSDF((1,0), Circle(0,0,1))`
      2. 编译并运行
      3. 验证返回值约为 0.0（在圆上）
    预期结果：返回值在 [-0.001, 0.001] 范围内
    失败指示：返回值远离 0
    证据：.sisyphus/evidence/task-2-sdf-circle-test.txt
  ```

  **提交**：是
  - 消息：`feat(dc): 实现 SDF 基础形状（圆形、方形）`
  - 文件：`demo/dual_contouring/dc_sdf.h`, `demo/dual_contouring/dc_sdf.cpp`

---

- [x] 3. 四叉树数据结构

  **实现步骤**：
  - 创建 `dc_quadtree.h` 头文件
  - 实现 `QuadtreeNode` 结构体：
    - 边界框（x, y, size）
    - 四个子节点指针（NW, NE, SW, SE）
    - 是否已细分标志
    - 层级深度
  - 实现 `CreateQuadtree(vec2 center, float size, int maxDepth)` 根节点创建
  - 实现 `Subdivide(QuadtreeNode* node)` 细分函数
  - 实现 `ShouldSubdivide(QuadtreeNode* node, SDF*)` 判断准则：节点边界内有表面穿越
  - 实现 `BuildQuadtree(SDF*, int maxDepth)` 递归构建完整四叉树

  **禁止事项**：
  - 不要实现节点删除/合并
  - 不要实现四叉树遍历以外的节点操作

  **推荐智能体配置文件**：
  - **类别**：`quick`（标准数据结构，模式清晰）
    - 理由：四叉树是经典数据结构，实现模式明确
  - **技能**：`无`

  **并行化**：
  - **可并行运行**：是
  - **并行组**：波次 1（与任务 1, 2, 4）
  - **阻塞**：任务 5, 6, 7
  - **被阻塞**：无

  **引用**：

  **模式引用**：
  - 无（项目中无现有四叉树实现）

  **外部引用**：
  - 四叉树教程：`https://en.wikipedia.org/wiki/Quadtree` - 基本概念

  **验收标准**：

  - [ ] `QuadtreeNode` 包含边界、深度、四个子节点指针
  - [ ] `BuildQuadtree` 在 maxDepth=3 时创建正确数量的节点
  - [ ] 表面穿越的节点被细分，不穿越的节点不细分
  - [ ] 根节点大小正确

  **QA 场景**：

  ```
  场景：四叉树在圆形表面处正确细分
    工具：Bash (编译测试)
    前提条件：四叉树实现完成
    步骤：
      1. 创建半径 1 的圆形 SDF
      2. 用 maxDepth=2 构建四叉树
      3. 验证与圆形相交的节点被细分
      4. 验证与圆形不相交的节点未细分
    预期结果：靠近表面的节点深度 > 远离表面的节点
    失败指示：所有节点同深度，或无节点被细分
    证据：.sisyphus/evidence/task-3-quadtree-subdivide.txt
  ```

  **提交**：是
  - 消息：`feat(dc): 实现四叉树数据结构`
  - 文件：`demo/dual_contouring/dc_quadtree.h`, `demo/dual_contouring/dc_quadtree.cpp`

---

- [x] 4. 基础渲染框架（ImDrawList）

  **实现步骤**：
  - 创建 `dc_renderer.h` 头文件
  - 实现 `Renderer` 类：
    - ImDrawList 指针管理
    - `BeginFrame()` / `EndFrame()` 方法
  - 实现 `DrawPoint(vec2, ImU32 color)` 绘制点
  - 实现 `DrawLine(vec2 p1, vec2 p2, ImU32 color)` 绘制线
  - 实现 `DrawQuad(vec2 center, float size, ImU32 color)` 绘制四边形
  - 实现 `DrawCircle(vec2 center, float radius, ImU32 color, int segments=32)` 绘制圆
  - 实现 `DrawSDFContour(SDF*, float isoValue, ImU32 color)` 绘制 SDF 等值线

  **禁止事项**：
  - 不要实现 3D 渲染
  - 不要实现光照/着色

  **推荐智能体配置文件**：
  - **类别**：`visual-engineering`（UI/渲染相关）
    - 理由：使用 ImDrawList 进行 2D 渲染
  - **技能**：`无`（项目无 ImDrawList 技能）
  - **评估但省略的技能**：
    - `artistry`：不需要创意设计

  **并行化**：
  - **可并行运行**：是
  - **并行组**：波次 1（与任务 1, 2, 3）
  - **阻塞**：任务 8, 9, 10
  - **被阻塞**：无

  **引用**：

  **模式引用**：
  - `demo/opengl/main.cpp:184-191` - ImGui 渲染模式：`ImGui_ImplOpenGL3_RenderDrawData`

  **外部引用**：
  - ImDrawList API：`https://dearimgui.com/docs/draw_list` - AddLine, AddCircle, AddQuad

  **验收标准**：

  - [ ] `DrawCircle` 绘制圆形，无明显多边形痕迹
  - [ ] `DrawLine` 连接两个点
  - [ ] `DrawSDFContour` 正确显示 SDF 等值线
  - [ ] 渲染在 ImGui NewFrame/EndFrame 之间调用

  **QA 场景**：

  ```
  场景：渲染器正确绘制圆形
    工具：Playwright（截图验证）
    前提条件：渲染器实现完成，集成到 main.cpp
    步骤：
      1. 启动程序
      2. 调用 DrawCircle((400,300), 50, IM_COL32(255,0,0,255))
      3. 截图保存
    预期结果：红色圆形可见于 (400,300)，半径约 50 像素
    失败指示：圆形未显示，或位置/大小错误
    证据：.sisyphus/evidence/task-4-render-circle.png
  ```

  **提交**：是
  - 消息：`feat(dc): 实现 ImDrawList 基础渲染框架`
  - 文件：`demo/dual_contouring/dc_renderer.h`, `demo/dual_contouring/dc_renderer.cpp`

---

- [x] 5. Hermite 数据存储

  **实现步骤**：
  - 创建 `dc_hermite.h` 头文件
  - 实现 `HermiteData` 结构体：
    - `position` - 交点位置 (vec2)
    - `normal` - 法线方向 (vec2)
  - 实现 `EdgeIntersections` 数组存储每条边的交点数据
  - 实现 `FindEdgeIntersections(QuadtreeNode*, SDF*)` 函数：
    - 检查四叉树节点的四条边
    - 使用二分法或线性搜索找到边与 SDF 等值面的交点
    - 存储交点位置和法线（通过 SDF 梯度计算）
  - 实现 `CollectAllHermiteData(QuadtreeNode*, SDF*)` 递归收集所有节点的 Hermite 数据

  **禁止事项**：
  - 不要实现三维 Hermite 数据
  - 不要实现复杂边相交算法（二分法足够用于教学）

  **推荐智能体配置文件**：
  - **类别**：`quick`（数学计算，模式直接）
    - 理由：Hermite 数据存储是简单结构体和数组操作
  - **技能**：`无`

  **并行化**：
  - **可并行运行**：是（但被任务 2, 3 阻塞）
  - **并行组**：无（顺序）
  - **阻塞**：任务 6, 7
  - **被阻塞**：任务 2, 3

  **引用**：

  **外部引用**：
  - Inigo Quilez 梯度计算：`https://iquilezles.org/articles/distfunctions2d/` - dFdx, dFdy 用于法线计算

  **验收标准**：

  - [ ] `HermiteData` 包含 position 和 normal 成员
  - [ ] 圆形 SDF 的 Hermite 交点法线指向圆心外
  - [ ] 方形 SDF 的 Hermite 交点法线垂直于边
  - [ ] 所有表面穿越边都被正确标记

  **QA 场景**：

  ```
  场景：圆形 SDF 正确生成 Hermite 交点
    工具：Bash (编译测试)
    前提条件：Hermite 数据实现完成
    步骤：
      1. 创建半径 1 的圆形 SDF
      2. 在包围盒 [-2,2] x [-2,2] 内构建四叉树
      3. 调用 FindEdgeIntersections
      4. 验证交点数量正确（应约为 8-16 个，取决于细分深度）
    预期结果：所有交点的法线大致指向圆心外（径向）
    失败指示：交点数量为 0 或法线方向错误
    证据：.sisyphus/evidence/task-5-hermite-circle.txt
  ```

  **提交**：是
  - 消息：`feat(dc): 实现 Hermite 数据存储`
  - 文件：`demo/dual_contouring/dc_hermite.h`, `demo/dual_contouring/dc_hermite.cpp`

---

- [x] 6. QEF 求解器（SVD 分解）

  **实现步骤**：
  - 创建 `dc_qef.h` 头文件
  - 实现 `QEFSolver` 类：
    - 存储法线数组
    - 存储位置数组
  - 实现 `AddConstraint(vec2 position, vec2 normal)` 添加约束
  - 实现 `Solve()` 使用 SVD 分解求解：
    - 构建 ATA 和 ATb 矩阵
    - 使用 Jacobi SVD 或简化的 SVD
    - 处理奇异/秩不足情况（添加小量扰动）
    - 返回最小二乘解
  - 实现 `GetSolution()` 返回求解结果
  - 实现 `GetError()` 返回误差

  **禁止事项**：
  - 不要使用外部矩阵库（手写 SVD 或简化版本）
  - 不要实现病态矩阵处理（记录但简化）

  **推荐智能体配置文件**：
  - **类别**：`ultrabrain`（数学库实现，需要稳定算法）
    - 理由：SVD 是数值不稳定的，需要仔细实现
  - **技能**：`无`
  - **评估但省略的技能**：
    - `deep`：不适合数学核心实现

  **并行化**：
  - **可并行运行**：否
  - **并行组**：无
  - **阻塞**：任务 7
  - **被阻塞**：任务 3, 5

  **引用**：

  **外部引用**：
  - SVD 教程：`https://en.wikipedia.org/wiki/Singular_value_decomposition` - 数学基础
  - 数值方法：`https://www.library.cornell.edu/numerical/nla` - Jacobi SVD 算法

  **验收标准**：

  - [ ] `Solve()` 返回 vec2 类型点
  - [ ] 解算点位于单元格内部或附近
  - [ ] 对于对称分布的法线，解算点接近质心
  - [ ] 奇异矩阵情况不会崩溃（使用扰动）

  **QA 场景**：

  ```
  场景：QEF 求解器对简单情况返回正确解
    工具：Bash (编译测试)
    前提条件：QEF 求解器实现完成
    步骤：
      1. 添加四个法线：(1,0), (-1,0), (0,1), (0,-1)
      2. 对应位置：(1,0), (-1,0), (0,1), (0,-1)
      3. 调用 Solve()
      4. 验证解算点接近 (0,0)
    预期结果：解算点位于 [-0.1, 0.1] x [-0.1, 0.1] 范围内
    失败指示：解算点远离原点
    证据：.sisyphus/evidence/task-6-qef-simple.txt
  ```

  **提交**：是
  - 消息：`feat(dc): 实现 QEF 求解器（SVD 分解）`
  - 文件：`demo/dual_contouring/dc_qef.h`, `demo/dual_contouring/dc_qef.cpp`

---

- [x] 7. Dual Contouring 核心算法

  **实现步骤**：
  - 创建 `dc_contouring.h` 头文件
  - 实现 `ContouringContext` 结构体：
    - 指向当前 SDF 的指针
    - 最大细分深度
    - QEF 阈值
  - 实现 `ContourNode(QuadtreeNode*, ContouringContext*)` 递归函数：
    - 检查节点是否包含表面
    - 如果是叶子节点：收集 Hermite 数据，求解 QEF，存储顶点
    - 如果不是叶子节点：递归处理四个子节点
  - 实现 `GenerateContour(SDF*, QuadtreeNode*, int maxDepth)` 主函数：
    - 构建四叉树
    - 调用 ContourNode
    - 返回顶点位置数组

  **禁止事项**：
  - 不要实现多分辨率 LOD（保持单一分辨率用于教学）
  - 不要实现并行化（顺序处理用于清晰理解）

  **推荐智能体配置文件**：
  - **类别**：`deep`（核心算法逻辑）
    - 理由：DC 算法是核心，需要仔细实现和理解
  - **技能**：`无`

  **并行化**：
  - **可并行运行**：否
  - **并行组**：无
  - **阻塞**：任务 8, 11
  - **被阻塞**：任务 2, 3, 5, 6

  **引用**：

  **外部引用**：
  - 原始论文：Fedkat/CAD 每侧一个交点原则

  **验收标准**：

  - [ ] 圆形 SDF 生成单个封闭顶点的环
  - [ ] 方形 SDF 生成四个顶点的四边形
  - [ ] 顶点位于其父节点内部
  - [ ] 顶点数与表面复杂度成正比

  **QA 场景**：

  ```
  场景：DC 算法对圆形 SDF 生成正确网格
    工具：Bash (编译测试)
    前提条件：DC 算法实现完成
    步骤：
      1. 创建半径 1 的圆形 SDF
      2. 用 maxDepth=4 调用 GenerateContour
      3. 收集返回的顶点
      4. 验证顶点数 > 0
      5. 验证所有顶点到圆心距离约等于 1
    预期结果：顶点数约 16-32 个，所有顶点位于圆上
    失败指示：顶点在圆内或圆外散乱分布
    证据：.sisyphus/evidence/task-7-dc-circle.txt
  ```

  **提交**：是
  - 消息：`feat(dc): 实现 Dual Contouring 核心算法`
  - 文件：`demo/dual_contouring/dc_contouring.h`, `demo/dual_contouring/dc_contouring.cpp`

---

- [x] 8. 网格生成与渲染

  **实现步骤**：
  - 创建 `dc_mesh.h` 头文件
  - 实现 `MeshVertex` 结构体：位置、法线（可选）
  - 实现 `MeshEdge` 结构体：起点、终点索引
  - 实现 `DualMesh` 结构体：顶点数组、边数组
  - 实现 `BuildDualMesh(QuadtreeNode*, ContouringContext*)` 构建对偶网格：
    - 从 ContourNode 获取顶点
    - 通过相邻节点共享边建立连接
    - 形成闭合环或开放边
  - 实现 `RenderMesh(DualMesh*, Renderer*)` 使用 ImDrawList 渲染网格

  **禁止事项**：
  - 不要实现复杂网格数据结构和算法（仅对偶边）
  - 不要实现网格优化（简化、细分）

  **推荐智能体配置文件**：
  - **类别**：`visual-engineering`（2D 渲染）
    - 理由：使用 ImDrawList 渲染网格
  - **技能**：`无`

  **并行化**：
  - **可并行运行**：否
  - **并行组**：无
  - **阻塞**：任务 9, 10, 11
  - **被阻塞**：任务 4, 7

  **引用**：

  **模式引用**：
  - `dc_renderer.h/cpp` - 使用已实现的 DrawLine 方法

  **验收标准**：

  - [ ] `DualMesh` 包含顶点数组和边数组
  - [ ] 圆形 SDF 生成封闭的环形网格
  - [ ] 网格渲染无明显瑕疵
  - [ ] 网格边数与顶点成正确比例

  **QA 场景**：

  ```
  场景：网格渲染显示正确的圆形
    工具：Playwright（截图验证）
    前提条件：网格生成和渲染实现完成
    步骤：
      1. 启动程序
      2. 生成圆形 SDF 的对偶网格
      3. 渲染网格
      4. 截图保存
    预期结果：显示闭合圆形网格，边缘平滑
    失败指示：网格不闭合、有交叉线、形状扭曲
    证据：.sisyphus/evidence/task-8-mesh-circle.png
  ```

  **提交**：是
  - 消息：`feat(dc): 实现网格生成与渲染`
  - 文件：`demo/dual_contouring/dc_mesh.h`, `demo/dual_contouring/dc_mesh.cpp`

---

- [x] 9. ImGui 控制面板

  **实现步骤**：
  - 创建 `dc_app.h` 头文件
  - 实现 `AppState` 结构体存储应用状态：
    - 当前 SDF 类型
    - 四叉树最大深度
    - QEF 阈值
    - 可视化选项
  - 实现 `RenderControlPanel(AppState*)` 函数：
    - 使用 ImGui::SliderInt 调整最大深度
    - 使用 ImGui::SliderFloat 调整 QEF 阈值
    - 使用 ImGui::Checkbox 控制各可视化层
  - 实现 `RenderInfoPanel(AppState*, DualMesh*)` 显示统计信息：
    - 顶点数
    - 边数
    - 四叉树节点数
    - 当前 FPS

  **禁止事项**：
  - 不要实现复杂 UI 布局（保持简单单栏面板）
  - 不要实现持久化设置（每次启动默认）

  **推荐智能体配置文件**：
  - **类别**：`visual-engineering`（UI/UX）
    - 理由：ImGui 面板是 UI 组件
  - **技能**：`无`

  **并行化**：
  - **可并行运行**：否
  - **并行组**：无
  - **阻塞**：任务 12
  - **被阻塞**：任务 4, 8

  **引用**：

  **模式引用**：
  - `demo/opengl/main.cpp:156-171` - ImGui 窗口、Slider、Checkbox 使用模式

  **外部引用**：
  - ImGui 文档：`https://dearimgui.com/docs/widgets` - 控件文档

  **验收标准**：

  - [ ] ImGui 面板显示在窗口左侧或右侧
  - [ ] 深度滑块范围 1-8，默认 4
  - [ ] 阈值滑块范围 0.001-1.0，默认 0.1
  - [ ] 统计信息实时更新

  **QA 场景**：

  ```
  场景：ImGui 面板参数调节生效
    工具：Playwright（UI 交互）
    前提条件：ImGui 控制面板实现完成
    步骤：
      1. 启动程序
      2. 找到深度滑块
      3. 将深度从 4 调整为 6
      4. 观察四叉树可视化变化
    预期结果：四叉树显示更深的层级
    失败指示：调整滑块无效果或程序崩溃
    证据：.sisyphus/evidence/task-9-panel-adjust.png
  ```

  **提交**：是
  - 消息：`feat(dc): 实现 ImGui 控制面板`
  - 文件：`demo/dual_contouring/dc_app.h`, `demo/dual_contouring/dc_app.cpp`

---

- [x] 10. 交互式形状编辑器

  **实现步骤**：
  - 扩展 `AppState` 添加：
    - 形状列表（圆形、方形数组）
    - 当前编辑模式
  - 实现 `RenderShapeEditor(AppState*)` 函数：
    - 列出当前场景中的形状
    - 每种形状的参数编辑（位置、大小）
    - 添加/删除形状按钮
  - 实现 `AddCircle(AppState*, vec2 pos, float radius)` 添加圆形
  - 实现 `AddSquare(AppState*, vec2 pos, float size)` 添加方形
  - 实现 `RemoveShape(AppState*, int index)` 删除形状
  - 实现复合 SDF：遍历所有形状求并集

  **禁止事项**：
  - 不要实现复杂形状（多边形、自由曲线）
  - 不要实现形状变换（仅支持位置和大小）

  **推荐智能体配置文件**：
  - **类别**：`visual-engineering`（UI 交互）
    - 理由：形状编辑器是交互式 UI
  - **技能**：`无`

  **并行化**：
  - **可并行运行**：否
  - **并行组**：无
  - **阻塞**：任务 12
  - **被阻塞**：任务 4, 8

  **引用**：

  **模式引用**：
  - `dc_app.h/cpp` - 使用现有面板模式

  **验收标准**：

  - [ ] 可以添加至少两种形状（圆形、方形）
  - [ ] 可以删除现有形状
  - [ ] 形状列表实时显示
  - [ ] 添加/删除形状后网格立即更新

  **QA 场景**：

  ```
  场景：添加圆形后网格更新
    工具：Playwright（UI 交互）
    前提条件：形状编辑器实现完成
    步骤：
      1. 启动程序（默认有一个形状）
      2. 点击"添加圆形"按钮
      3. 观察网格变化
      4. 截图保存
    预期结果：网格显示添加的圆形
    失败指示：点击无效果或网格未更新
    证据：.sisyphus/evidence/task-10-add-shape.png
  ```

  **提交**：是
  - 消息：`feat(dc): 实现交互式形状编辑器`
  - 文件：`demo/dual_contouring/dc_app.h`, `demo/dual_contouring/dc_app.cpp`

---

- [x] 11. 分步可视化系统

  **实现步骤**：
  - 扩展 `AppState` 添加：
    - 当前可视化阶段（TreeBuild, QEFSolve, MeshGenerate）
    - 是否分步模式
    - 当前步数
  - 实现 `RenderTreeStructure(QuadtreeNode*, Renderer*)` 可视化四叉树：
    - 用不同颜色区分已细分/未细分节点
    - 用线条绘制节点边界
  - 实现 `RenderQFESolving(QuadtreeNode*, Renderer*)` 可视化 QEF 求解：
    - 显示每个节点的 Hermite 交点
    - 显示 QEF 解算点
    - 可选：动画显示求解过程
  - 实现 `RenderAllStages(AppState*, Renderer*)` 根据当前阶段渲染对应内容
  - 实现分步控制：下一步/上一步/自动播放按钮

  **禁止事项**：
  - 不要实现复杂的动画系统（简单状态切换足够）
  - 不要实现历史记录（仅当前状态）

  **推荐智能体配置文件**：
  - **类别**：`visual-engineering`（分步 UI）
    - 理由：分步可视化是 UI 功能
  - **技能**：`无`

  **并行化**：
  - **可并行运行**：否
  - **并行组**：无
  - **阻塞**：任务 12
  - **被阻塞**：任务 7, 8

  **引用**：

  **模式引用**：
  - `dc_renderer.h/cpp` - 使用现有渲染方法

  **验收标准**：

  - [ ] 可以切换：仅树、仅 QEF、仅网格、全部
  - [ ] 分步模式下显示进度指示
  - [ ] "下一步"按钮逐步推进可视化
  - [ ] 不同阶段用不同颜色区分

  **QA 场景**：

  ```
  场景：分步可视化正确切换阶段
    工具：Playwright（UI 交互）
    前提条件：分步可视化实现完成
    步骤：
      1. 启动程序
      2. 选择"仅树"视图
      3. 截图保存
      4. 选择"仅 QEF"视图
      5. 截图保存
      6. 选择"全部"视图
      7. 截图保存
    预期结果：三个截图分别显示：树结构、QEF 点、完整网格
    失败指示：视图未切换或显示错误内容
    证据：.sisyphus/evidence/task-11-stages-*.png
  ```

  **提交**：是
  - 消息：`feat(dc): 实现分步可视化系统`
  - 文件：`demo/dual_contouring/dc_app.h`, `demo/dual_contouring/dc_app.cpp`

---

- [ ] 12. 参数实时调节

  **实现步骤**：
  - 集成所有组件到 `main.cpp`：
    - 初始化 GLFW + ImGui
    - 创建 AppState 实例
    - 主循环中调用所有更新和渲染函数
  - 实现实时调节：
    - 参数变更时立即重新计算
    - 使用 dirty flag 避免不必要重算
  - 实现性能优化：
    - 最大深度限制为 8
    - QEF 阈值合理范围
  - 实现窗口布局：
    - 左侧：ImGui 控制面板
    - 右侧/中央：渲染画布

  **禁止事项**：
  - 不要实现多窗口（单一窗口足够）
  - 不要实现复杂布局系统

  **推荐智能体配置文件**：
  - **类别**：`visual-engineering`（集成+UI）
    - 理由：集成所有组件并创建单一窗口 UI
  - **技能**：`无`

  **并行化**：
  - **可并行运行**：否
  - **并行组**：无
  - **阻塞**：任务 F1, F2, F3, F4
  - **被阻塞**：任务 9, 10, 11

  **引用**：

  **模式引用**：
  - `demo/opengl/main.cpp:127-193` - 完整的主循环模式

  **验收标准**：

  - [ ] 程序启动显示完整 UI
  - [ ] 调整参数时网格实时更新
  - [ ] 添加/删除形状后网格实时更新
  - [ ] FPS 显示在 ImGui 面板中
  - [ ] 程序可正常退出

  **QA 场景**：

  ```
  场景：完整应用正常工作
    工具：Playwright（综合测试）
    前提条件：参数实时调节实现完成
    步骤：
      1. 启动程序
      2. 调整深度滑块到 6
      3. 添加一个圆形
      4. 切换到"全部"视图
      5. 截图保存
    预期结果：显示包含两个圆形形状的完整网格，深度为 6
    失败指示：程序崩溃、UI 无响应、网格不正确
    证据：.sisyphus/evidence/task-12-full-app.png
  ```

  **提交**：是
  - 消息：`feat(dc): 实现参数实时调节和主循环集成`
  - 文件：`demo/dual_contouring/main.cpp`

---

## 最终验证波次（强制 — 所有实现任务后）

> 4 个审查智能体并行运行。全部必须批准。呈现综合结果给用户并获得明确"okay"后再完成。
>
> **不要在获得用户 okay 后自动进行验证。**
> **不要在获得用户 okay 前标记 F1-F4 为已完成。** 拒绝或用户反馈 -> 修复 -> 重新运行 -> 呈现 -> 等待 okay。

- [ ] F1. **计划合规审计** — `oracle`
  从头到尾阅读计划。对于每个"必须有"：验证实现存在（读取文件、运行命令）。对于每个"严禁出现"：搜索禁止模式 — 如果发现则拒绝并指出文件:行号。检查证据文件存在于 .sisyphus/evidence/。比较交付物与计划。
  输出：`必须有 [N/N] | 严禁出现 [N/N] | 任务 [N/N] | 裁决：批准/拒绝`

- [ ] F2. **代码质量审查** — `unspecified-high`
  运行编译器 + linter。审查所有更改文件：`as any`/`@ts-ignore`、空 catch、console.log in prod、注释掉代码、未使用导入。检查 AI 垃圾：过多注释、过度抽象、泛型名称（data/result/item/temp）。
  输出：`构建 [通过/失败] | 林特 [通过/失败] | 测试 [N 通过/N 失败] | 文件 [N 干净/N 问题] | 裁决`

- [ ] F3. **真实手动 QA** — `unspecified-high`（+ `playwright` 技能如果有 UI）
  从干净状态开始。执行每个任务中每个 QA 场景的每一步 — 遵循精确步骤，捕获证据。测试跨任务集成（协同工作的功能，而非孤立）。测试边缘情况：空状态、无效输入、快速操作。保存到 `.sisyphus/evidence/final-qa/`。
  输出：`场景 [N/N 通过] | 集成 [N/N] | 边缘情况 [N 已测试] | 裁决`

- [ ] F4. **范围保真度检查** — `deep`
  对于每个任务：阅读"做什么"，阅读实际 diff（git log/diff）。验证 1:1 — 计划中的所有内容都已构建（无遗漏），计划外的任何内容都已构建（无蔓延）。检查"禁止做什么"合规性。检测跨任务污染：任务 N 处理任务 M 的文件。标记未解释的更改。
  输出：`任务 [N/N 合规] | 污染 [干净/N 问题] | 未解释 [干净/N 文件] | 裁决`

---

## 提交策略

- **1**：`feat(dc): 项目骨架` — main.cpp, CMakeLists.txt
- **2**：`feat(dc): SDF 基础形状` — dc_sdf.h/cpp
- **3**：`feat(dc): 四叉树结构` — dc_quadtree.h/cpp
- **4**：`feat(dc): 渲染框架` — dc_renderer.h/cpp
- **5**：`feat(dc): Hermite 数据` — dc_hermite.h/cpp
- **6**：`feat(dc): QEF 求解器` — dc_qef.h/cpp
- **7**：`feat(dc): Dual Contouring 算法` — dc_contouring.h/cpp
- **8**：`feat(dc): 网格生成` — dc_mesh.h/cpp
- **9**：`feat(dc): ImGui 控制面板` — dc_app.h/cpp
- **10**：`feat(dc): 形状编辑器` — dc_app.h/cpp
- **11**：`feat(dc): 分步可视化` — dc_app.h/cpp
- **12**：`feat(dc): 参数调节` — dc_app.h/cpp

---

## 成功标准

### 验证命令
```bash
# 编译验证
cmake --build . --target demoDualContouring

# 运行验证
./demo/dual_contouring/demoDualContouring.exe
```

### 最终检查清单
- [ ] 所有"必须有"项存在
- [ ] 所有"严禁出现"项不存在
- [ ] 所有测试通过
- [ ] 编译成功无警告
- [ ] ImGui 窗口正确显示
- [ ] 四叉树可视化正常工作
- [ ] QEF 求解过程可见
- [ ] 最终网格正确渲染
- [ ] 参数调节实时响应
- [ ] 形状编辑功能正常
