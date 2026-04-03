# Dual Contouring Demo UI布局优化计划

## TL;DR

> **快速摘要**：优化Dual Contouring演示程序的UI布局，实现标签页式参数面板、可拖动分割线和折叠功能。
>
> **交付成果**：
> - 标签页式参数面板（4个标签页：算法参数、可视化选项、形状编辑器、统计信息）
> - 可拖动分割线（调整参数与绘制区域比例）
> - 参数面板折叠/展开功能
> - 移除NoMove标志（允许窗口自由移动）
>
> **预计工作量**：中等
> **并行执行**：可以（最多3个并行任务）
> **关键路径**：Task 1 → Task 2 → Task 3 → Task 4 → Task 5

---

## 背景

### 原始请求
用户报告界面存在问题：
- 参数页面无法拖动
- 多个参数页面会重叠在一起
- 无法通过拖动参数页面与绘制页面中间的竖线来调整两者的比例

### 访谈总结
**关键讨论**：
- 参数面板布局：选择**标签页式布局**（算法参数、可视化选项、形状编辑器、统计信息分页显示）
- 分割器：需要**可拖动分割线**调整宽度比例
- 折叠功能：需要**折叠/展开**功能
- 窗口标志：移除`NoMove`标志

**研究结果**：
- ImGui分割器参考：imgui_demo.cpp中有官方分割器示例
- 标签页实现：使用`ImGui::BeginTabBar` + `ImGui::BeginTabItem`
- 当前代码问题：所有窗口使用绝对定位，多个窗口位置重叠

### Metis审查
**发现的问题**（已解决）：
- 标签页内容分配：方案A（分开4个标签）
- 分割器约束：最小250px，最大500px，画布最小500px，分割线8px，默认320px
- 折叠行为：完全隐藏，点击按钮恢复

---

## 工作目标

### 核心目标
优化UI布局，实现灵活可调的参数面板与绘制区域分割界面

### 具体交付物
- `demo/dual_contouring/main.cpp` - 修改主布局，实现分割器和标签页
- `demo/dual_contouring/dc_app.cpp` - 修改面板渲染函数
- `demo/dual_contouring/dc_app.h` - 添加新状态变量

### 完成定义
- [ ] 4个标签页正确切换显示不同内容
- [ ] 分割线可拖动，参数区域和画布区域宽度相应调整
- [ ] 折叠按钮可隐藏/显示参数区域
- [ ] 所有窗口可自由移动（移除NoMove）
- [ ] 分割线约束生效（最小250px，最大500px）

### 必须有
- 4个功能完整的标签页
- 可拖动分割线
- 折叠/展开功能
- 分割线宽度约束

### 禁止有
- 多个参数面板重叠问题
- 窗口无法拖动问题
- 无法调整比例问题
- 分割线拖动越界

---

## 验证策略

### 测试决策
- **基础设施存在**：否（项目无正式测试套件）
- **自动化测试**：无
- **框架**：无

### QA策略
每个任务必须包含**代理可执行QA场景**。验证方式：
- 编译构建验证
- 手动运行验证（按步骤执行）
- 截图证据保存

### QA场景示例（每个任务都需要）

```
场景：标签页切换功能
工具：手动编译运行
前置条件：程序正常启动
步骤：
  1. 观察左侧参数面板显示的标签
  2. 点击"可视化选项"标签
  3. 观察内容变化
预期结果：显示6个checkbox（SDF等值线、法线向量、四叉树边界等）
证据：保存截图 task-3-tab-visualization.png
```

---

## 执行策略

### 并行执行波次

```
Wave 1 (立即启动 — 基础设施)：
├── Task 1: 修改dc_app.h添加新状态变量
├── Task 2: 修改dc_app.cpp移除NoMove标志
└── Task 3: 创建分割器辅助函数

Wave 2 (Wave 1完成后 — 标签页实现)：
├── Task 4: 在main.cpp实现标签页式参数面板框架
├── Task 5: 将控制面板内容移入"算法参数"标签
├── Task 6: 将可视化选项移入"可视化选项"标签
├── Task 7: 将形状编辑器移入"形状编辑器"标签
└── Task 8: 将统计信息移入"统计信息"标签

Wave 3 (Wave 2完成后 — 分割器和折叠)：
├── Task 9: 实现水平分割线
├── Task 10: 实现折叠/展开功能
└── Task 11: 添加展开按钮

Wave FINAL (所有任务完成后 — 4并行审查)：
├── Task F1: 计划合规审查 ✅
├── Task F2: 代码质量审查 ✅
├── Task F3: 手动QA ✅
└── Task F4: 范围忠诚度检查 ✅
```

### 依赖矩阵

- **Task 1-3**: 无依赖，可并行
- **Task 4-8**: 依赖Task 1（状态变量）
- **Task 9-11**: 依赖Task 4（标签页框架）
- **Task F1-F4**: 依赖所有Task完成

### 代理调度摘要

- **Wave 1**: 3个任务 → `quick` (修改状态和函数)
- **Wave 2**: 5个任务 → `unspecified-high` (标签页内容迁移)
- **Wave 3**: 3个任务 → `unspecified-high` (分割器和折叠)
- **FINAL**: 4个任务 → `unspecified-high` + `oracle`

---

## 待办事项

> 每个任务必须包含：推荐代理配置文件 + 并行化信息 + QA场景。

- [x] 1. **修改dc_app.h添加新状态变量**

  **要做什么**：
  - 在 `AppState` 结构体中添加以下新字段：
    - `int currentTabIndex` - 当前选中的标签页索引（0=算法参数，1=可视化选项，2=形状编辑器，3=统计信息）
    - `float splitterPosition` - 分割线位置（默认320.0f）
    - `float lastSplitterPosition` - 折叠前记录的分割线位置
    - `bool parameterPanelCollapsed` - 参数面板是否折叠

  **禁止做**：
  - 不修改现有的算法参数和可视化选项字段
  - 不改变现有的 `RenderXxx` 函数签名

  **推荐代理配置**：
  - **类别**：`quick`
    - 原因：简单的结构体字段添加，不涉及复杂逻辑
  - **技能**：`无`
  - **省略技能**：无需特殊技能

  **并行化**：
  - **可并行运行**：是
  - **并行组**：Wave 1（与Task 2, 3并行）
  - **阻塞**：Task 4-8（标签页需要这些状态变量）
  - **被阻塞**：无

  **引用**：
  - `demo/dual_contouring/dc_app.h:67-175` - AppState结构体现有字段，添加新字段时保持格式一致

  **验收标准**：
  - [ ] dc_app.h中AppState结构体包含4个新字段
  - [ ] 新字段有合理的默认值
  - [ ] 编译通过

  **QA场景**：

  ```
  场景：状态变量添加验证
    工具：Bash (grep)
    前置条件：dc_app.h已修改
    步骤：
      1. 执行 grep "currentTabIndex" demo/dual_contouring/dc_app.h
      2. 执行 grep "splitterPosition" demo/dual_contouring/dc_app.h
      3. 执行 grep "parameterPanelCollapsed" demo/dual_contouring/dc_app.h
    预期结果：每个grep都找到对应的字段定义
    证据：保存grep输出 task-1-grep-output.txt
  ```

  **提交**：是（第1次提交）
  - 消息：`refactor(ui): 添加UI状态变量到AppState`
  - 文件：`demo/dual_contouring/dc_app.h`

- [x] 2. **修改dc_app.cpp移除NoMove标志**

  **要做什么**：
  - 找到并修改 `RenderControlPanel` 函数（dc_app.cpp:479-554）：
    - 移除 `ImGuiWindowFlags_NoMove` 标志
  - 找到并修改 `RenderInfoPanel` 函数（dc_app.cpp:567-643）：
    - 移除 `ImGuiWindowFlags_NoMove` 标志
  - 找到并修改 `RenderShapeEditor` 函数（dc_app.cpp:719-844）：
    - 移除 `ImGuiWindowFlags_NoMove` 标志

  **禁止做**：
  - 不修改窗口内容，只修改标志
  - 不改变窗口位置和大小设置

  **推荐代理配置**：
  - **类别**：`quick`
    - 原因：简单的标志位修改，模式清晰
  - **技能**：`无`

  **并行化**：
  - **可并行运行**：是
  - **并行组**：Wave 1（与Task 1, 3并行）
  - **阻塞**：无
  - **被阻塞**：无

  **引用**：
  - `demo/dual_contouring/dc_app.cpp:494` - RenderControlPanel中的NoMove标志
  - `demo/dual_contouring/dc_app.cpp:578` - RenderInfoPanel中的NoMove标志
  - `demo/dual_contouring/dc_app.cpp:731` - RenderShapeEditor中的NoMove标志

  **验收标准**：
  - [ ] RenderControlPanel不再包含NoMove
  - [ ] RenderInfoPanel不再包含NoMove
  - [ ] RenderShapeEditor不再包含NoMove
  - [ ] 编译通过

  **QA场景**：

  ```
  场景：NoMove标志移除验证
    工具：Bash (grep)
    前置条件：dc_app.cpp已修改
    步骤：
      1. grep "NoMove" demo/dual_contouring/dc_app.cpp
    预期结果：grep结果为空（NoMove已全部移除）
    证据：保存grep输出 task-2-nomove-check.txt
  ```

  **提交**：是（第1次提交）
  - 消息：`refactor(ui): 移除控制面板的NoMove标志`
  - 文件：`demo/dual_contouring/dc_app.cpp`

- [x] 3. **创建分割器辅助函数**

  **要做什么**：
  - 在 `dc_app.cpp` 或 `main.cpp` 中添加一个 `RenderSplitter` 辅助函数：
    ```cpp
    // 渲染可拖动分割线
    // splitterX: 分割线当前位置的指针
    // minWidth: 参数区域最小宽度（250.0f）
    // maxWidth: 参数区域最大宽度（500.0f）
    // totalWidth: 总宽度（窗口宽度）
    // splitterThickness: 分割线宽度（8.0f）
    void RenderSplitter(float* splitterX, float minWidth, float maxWidth, float totalWidth, float splitterThickness)
    ```
  - 函数需要处理：
    - 鼠标悬停时改变光标样式
    - 鼠标拖动时更新splitterX位置
    - 约束检查（最小/最大宽度）
    - 绘制分割线（使用ImDrawList）

  **禁止做**：
  - 不实现折叠逻辑（Task 10负责）
  - 不创建标签页框架（Task 4负责）

  **推荐代理配置**：
  - **类别**：`unspecified-high`
    - 原因：需要理解ImGui内部机制和鼠标事件处理
  - **技能**：`无`

  **并行化**：
  - **可并行运行**：是
  - **并行组**：Wave 1（与Task 1, 2并行）
  - **阻塞**：Task 9（实现分割线需要此函数）
  - **被阻塞**：无

  **引用**：
  - `imgui_demo.cpp` - ImGui官方分割器示例（如果有）
  - `demo/dual_contouring/main.cpp:293-297` - 当前画布位置计算逻辑

  **验收标准**：
  - [ ] RenderSplitter函数存在并可调用
  - [ ] 函数签名包含所有必需参数
  - [ ] 函数处理鼠标拖动和约束检查
  - [ ] 编译通过

  **QA场景**：

  ```
  场景：分割器函数编译验证
    工具：Bash (grep)
    前置条件：dc_app.cpp或main.cpp已添加函数
    步骤：
      1. grep "RenderSplitter" demo/dual_contouring/*.cpp demo/dual_contouring/*.h
    预期结果：找到函数定义
    证据：保存grep输出 task-3-splitter-function.txt
  ```

  **提交**：是（第1次提交）
  - 消息：`feat(ui): 添加分割线辅助函数`
  - 文件：`demo/dual_contouring/dc_app.cpp` 或 `main.cpp`

- [x] 4. **实现标签页式参数面板框架**

  **要做什么**：
  - 修改 `main.cpp` 中的参数面板渲染逻辑（约第226-288行）
  - 创建统一的参数面板窗口，替换原来的4个独立窗口
  - 在面板内部使用 `ImGui::BeginTabBar` 和 `ImGui::BeginTabItem` 实现标签页
  - 面板结构：
    ```cpp
    ImGui::Begin("参数面板", nullptr, /* 无NoMove标志 */);
    
    if (ImGui::BeginTabBar("ParameterTabs")) {
        if (ImGui::BeginTabItem("算法参数")) {
            // Task 5: 放置算法参数内容
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("可视化选项")) {
            // Task 6: 放置可视化选项内容
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("形状编辑器")) {
            // Task 7: 放置形状编辑器内容
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("统计信息")) {
            // Task 8: 放置统计信息内容
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
    ```
  - 使用 `state->currentTabIndex` 保存/恢复当前标签页

  **禁止做**：
  - 不在这里放置具体内容（由Task 5-8负责）
  - 不实现分割器（由Task 9负责）
  - 不实现折叠功能（由Task 10负责）

  **推荐代理配置**：
  - **类别**：`unspecified-high`
    - 原因：需要理解ImGui标签页API和布局协调
  - **技能**：`无`

  **并行化**：
  - **可并行运行**：否
  - **并行组**：Wave 2
  - **阻塞**：Task 5, 6, 7, 8
  - **被阻塞**：Task 1（需要currentTabIndex等状态变量）

  **引用**：
  - `demo/dual_contouring/main.cpp:226-288` - 当前参数面板渲染代码
  - `demo/dual_contouring/dc_app.h` - AppState结构体（Task 1添加的字段）

  **验收标准**：
  - [ ] 参数面板使用ImGui::BeginTabBar
  - [ ] 有4个ImGui::BeginTabItem对应4个标签
  - [ ] 标签页框架可编译运行
  - [ ] 点击标签可切换（内容待Task 5-8填充）

  **QA场景**：

  ```
  场景：标签页框架编译运行
    工具：手动编译运行
    前置条件：main.cpp已修改
    步骤：
      1. 编译项目：cmake --build . --config Release
      2. 运行程序
      3. 观察左侧参数面板是否显示4个标签
      4. 点击每个标签观察是否可切换
    预期结果：4个标签显示，可切换，但内容为占位符
    证据：保存截图 task-4-tabs-framework.png
  ```

  **提交**：是（第2次提交）
  - 消息：`feat(ui): 实现标签页式参数面板框架`
  - 文件：`demo/dual_contouring/main.cpp`

- [x] 5. **将控制面板内容移入"算法参数"标签**

  **要做什么**：
  - 将 `RenderControlPanel` 函数中的"算法参数"部分（约第501-526行）移入Task 4创建的"算法参数"标签中
  - 内容包括：
    - "最大深度"滑块（范围1-8）
    - "QEF阈值"滑块（范围0.001f-1.0f）
  - 保持原有的 `ImGui::SliderInt` 和 `ImGui::SliderFloat` 调用

  **禁止做**：
  - 不修改参数绑定的变量名（保持 `state->maxDepth` 等）
  - 不改变滑块的范围和默认值

  **推荐代理配置**：
  - **类别**：`quick`
    - 原因：简单的代码移动，模式固定
  - **技能**：`无`

  **并行化**：
  - **可并行运行**：是
  - **并行组**：Wave 2（与Task 4, 6, 7, 8并行）
  - **阻塞**：无
  - **被阻塞**：Task 4

  **引用**：
  - `demo/dual_contouring/dc_app.cpp:501-526` - 当前算法参数渲染代码
  - `demo/dual_contouring/main.cpp:Task 4框架` - 标签页框架位置

  **验收标准**：
  - [ ] "算法参数"标签显示maxDepth滑块
  - [ ] "算法参数"标签显示QEF阈值滑块
  - [ ] 滑块可交互且值会更新

  **QA场景**：

  ```
  场景：算法参数标签内容验证
    工具：手动运行
    前置条件：Task 4和5完成
    步骤：
      1. 运行程序
      2. 点击"算法参数"标签
      3. 找到"最大深度"滑块，拖动改变值
      4. 找到"QEF阈值"滑块，拖动改变值
    预期结果：两个滑块都可见且可交互，值改变时四叉树/网格会重新生成
    证据：保存截图 task-5-algo-params.png
  ```

  **提交**：否（包含在第2次提交中）
  - 文件：`demo/dual_contouring/main.cpp`

- [x] 6. **将可视化选项移入"可视化选项"标签**

  **要做什么**：
  - 将 `RenderControlPanel` 函数中的"可视化选项"部分（约第532-551行）移入Task 4创建的"可视化选项"标签中
  - 内容包括：
    - "SDF可视化"折叠头
      - SDF等值线checkbox
      - 法线向量checkbox
    - "四叉树可视化"折叠头
      - 四叉树边界checkbox
      - 节点中心点checkbox
    - "对偶网格可视化"折叠头
      - 对偶网格checkbox
      - QEF交点checkbox

  **禁止做**：
  - 不修改checkbox绑定的变量名（保持 `state->showSDFContour` 等）

  **推荐代理配置**：
  - **类别**：`quick`
    - 原因：简单的代码移动，模式固定
  - **技能**：`无`

  **并行化**：
  - **可并行运行**：是
  - **并行组**：Wave 2
  - **阻塞**：无
  - **被阻塞**：Task 4

  **引用**：
  - `demo/dual_contouring/dc_app.cpp:532-551` - 当前可视化选项渲染代码

  **验收标准**：
  - [ ] "可视化选项"标签显示3个折叠头
  - [ ] 每个折叠头下有对应的checkbox
  - [ ] checkbox可交互

  **QA场景**：

  ```
  场景：可视化选项标签内容验证
    工具：手动运行
    前置条件：Task 4和6完成
    步骤：
      1. 运行程序
      2. 点击"可视化选项"标签
      3. 展开"SDF可视化"折叠头
      4. 勾选/取消勾选"SDF等值线"
    预期结果：画布上的SDF等值线显示/隐藏
    证据：保存截图 task-6-vis-options.png
  ```

  **提交**：否（包含在第2次提交中）

- [x] 7. **将形状编辑器移入"形状编辑器"标签**

  **要做什么**：
  - 将 `RenderShapeEditor` 函数（约第719-844行）的内容移入Task 4创建的"形状编辑器"标签中
  - 内容包括：
    - 形状列表（可选择）
    - 选中形状编辑（位置X/Y，大小）
    - 添加圆形/添加方形按钮
    - 删除选中形状按钮

  **禁止做**：
  - 不修改形状添加/删除的逻辑
  - 不改变 `state->shapes` 的操作

  **推荐代理配置**：
  - **类别**：`unspecified-high`
    - 原因：形状编辑器逻辑较复杂
  - **技能**：`无`

  **并行化**：
  - **可并行运行**：是
  - **并行组**：Wave 2
  - **阻塞**：无
  - **被阻塞**：Task 4

  **引用**：
  - `demo/dual_contouring/dc_app.cpp:719-844` - 当前形状编辑器完整实现

  **验收标准**：
  - [ ] "形状编辑器"标签显示形状列表
  - [ ] 可添加圆形和方形
  - [ ] 可选择和编辑形状
  - [ ] 可删除选中形状

  **QA场景**：

  ```
  场景：形状编辑器标签功能验证
    工具：手动运行
    前置条件：Task 4和7完成
    步骤：
      1. 运行程序
      2. 点击"形状编辑器"标签
      3. 点击"添加圆形"按钮
      4. 在形状列表中选择刚添加的圆形
      5. 修改位置X为0.5
      6. 点击"添加方形"按钮
    预期结果：两个形状都出现在画布上
    证据：保存截图 task-7-shape-editor.png
  ```

  **提交**：否（包含在第2次提交中）

- [x] 8. **将统计信息移入"统计信息"标签**

  **要做什么**：
  - 将 `RenderInfoPanel` 函数（约第567-643行）的内容移入Task 4创建的"统计信息"标签中
  - 内容包括：
    - 性能指标（FPS，帧时间）
    - 对偶网格统计（顶点数，边数）
    - 四叉树统计（节点数，当前深度）
    - 调试信息（SDF评估次数）

  **禁止做**：
  - 不修改统计计算逻辑
  - 不改变数据来源

  **推荐代理配置**：
  - **类别**：`quick`
    - 原因：简单的代码移动
  - **技能**：`无`

  **并行化**：
  - **可并行运行**：是
  - **并行组**：Wave 2
  - **阻塞**：无
  - **被阻塞**：Task 4

  **引用**：
  - `demo/dual_contouring/dc_app.cpp:567-643` - 当前统计信息面板完整实现

  **验收标准**：
  - [ ] "统计信息"标签显示FPS
  - [ ] 显示顶点数和边数
  - [ ] 显示节点数
  - [ ] 数据随程序运行更新

  **QA场景**：

  ```
  场景：统计信息标签内容验证
    工具：手动运行
    前置条件：Task 4和8完成
    步骤：
      1. 运行程序
      2. 点击"统计信息"标签
      3. 观察FPS数值
      4. 添加一个新形状
      5. 观察顶点数和边数变化
    预期结果：统计数据正确显示并更新
    证据：保存截图 task-8-stats.png
  ```

  **提交**：否（包含在第2次提交中）

- [x] 9. **实现水平分割线**

  **要做什么**：
  - 在参数面板和绘制画布之间添加可拖动的分割线
  - 使用Task 3创建的 `RenderSplitter` 函数
  - 分割线参数：
    - `splitterX`: `&state.splitterPosition`
    - `minWidth`: 250.0f
    - `maxWidth`: 500.0f
    - `totalWidth`: 窗口总宽度
    - `splitterThickness`: 8.0f
  - 当分割线位置改变时：
    - 调整参数面板宽度
    - 调整绘制画布宽度和位置

  **禁止做**：
  - 不实现折叠逻辑（Task 10负责）
  - 不改变标签页内容

  **推荐代理配置**：
  - **类别**：`unspecified-high`
    - 原因：需要协调多个窗口的布局计算
  - **技能**：`无`

  **并行化**：
  - **可并行运行**：否
  - **并行组**：Wave 3
  - **阻塞**：无
  - **被阻塞**：Task 3（需要RenderSplitter函数）

  **引用**：
  - `demo/dual_contouring/main.cpp:293-297` - 当前画布位置计算
  - `Task 3: RenderSplitter` - 分割线函数

  **验收标准**：
  - [ ] 参数面板和画布之间有可见的分割线
  - [ ] 鼠标悬停在分割线上时光标改变
  - [ ] 拖动分割线可调整两侧宽度
  - [ ] 分割线位置被约束在250-500px范围内

  **QA场景**：

  ```
  场景：分割线拖动验证
    工具：手动运行
    前置条件：Task 3和9完成
    步骤：
      1. 运行程序
      2. 观察参数面板和画布之间的分割线
      3. 将鼠标移到分割线上，观察光标变化
      4. 按住左键向左拖动
      5. 释放鼠标，观察参数面板变窄、画布变宽
      6. 向右拖动到最大位置（500px）
    预期结果：分割线可拖动，宽度约束生效
    证据：保存截图 task-9-splitter.png
  ```

  **提交**：是（第3次提交）
  - 消息：`feat(ui): 实现分割线功能`
  - 文件：`demo/dual_contouring/main.cpp`

- [x] 10. **实现折叠/展开功能**

  **要做什么**：
  - 在参数面板添加折叠按钮（可在标签栏右侧）
  - 点击折叠按钮时：
    - 设置 `state.parameterPanelCollapsed = true`
    - 保存当前分割线位置到 `state.lastSplitterPosition`
    - 将参数面板宽度设为0
    - 画布扩展到全宽
  - 点击展开按钮时：
    - 设置 `state.parameterPanelCollapsed = false`
    - 从 `state.lastSplitterPosition` 恢复分割线位置
    - 恢复参数面板宽度

  **禁止做**：
  - 不修改分割线约束逻辑
  - 不改变标签页内容

  **推荐代理配置**：
  - **类别**：`unspecified-high`
    - 原因：需要管理多个状态的协调
  - **技能**：`无`

  **并行化**：
  - **可并行运行**：否
  - **并行组**：Wave 3
  - **阻塞**：无
  - **被阻塞**：Task 9

  **引用**：
  - `demo/dual_contouring/main.cpp:Task 4` - 标签页框架
  - `Task 1: AppState` - parameterPanelCollapsed字段

  **验收标准**：
  - [ ] 参数面板右上角有折叠按钮
  - [ ] 点击折叠按钮参数面板消失
  - [ ] 画布扩展到全宽
  - [ ] 点击展开按钮参数面板恢复

  **QA场景**：

  ```
  场景：折叠展开功能验证
    工具：手动运行
    前置条件：Task 9和10完成
    步骤：
      1. 运行程序
      2. 确认参数面板可见，画布在右侧
      3. 点击参数面板右上角的折叠按钮
      4. 观察参数面板消失，画布扩展到全宽
      5. 点击画布左侧边缘的展开按钮
      6. 观察参数面板恢复，画布回到原位置
    预期结果：折叠/展开功能正常工作
    证据：保存截图 task-10-collapse.png 和 task-10-expand.png
  ```

  **提交**：是（第3次提交）
  - 消息：`feat(ui): 添加参数面板折叠/展开功能`
  - 文件：`demo/dual_contouring/main.cpp`

- [x] 11. **添加展开按钮**

  **要做什么**：
  - 当参数面板折叠时，在画布左侧边缘显示一个展开按钮
  - 按钮样式：垂直窄条 + 箭头图标
  - 位置：画布最左侧
  - 点击效果：触发Task 10的展开逻辑

  **禁止做**：
  - 不改变折叠/展开的核心逻辑

  **推荐代理配置**：
  - **类别**：`quick`
    - 原因：简单的UI元素添加
  - **技能**：`无`

  **并行化**：
  - **可并行运行**：是（与Task 10有依赖，但可并行设计）
  - **并行组**：Wave 3
  - **阻塞**：无
  - **被阻塞**：Task 10

  **引用**：
  - `demo/dual_contouring/main.cpp:Task 10` - 展开逻辑位置

  **验收标准**：
  - [ ] 折叠状态下画布左侧有可见的展开按钮
  - [ ] 按钮可点击并触发展开

  **QA场景**：

  ```
  场景：展开按钮验证
    工具：手动运行
    前置条件：Task 10和11完成
    步骤：
      1. 折叠参数面板
      2. 观察画布左侧边缘的展开按钮
      3. 点击展开按钮
    预期结果：参数面板展开，按钮消失
    证据：保存截图 task-11-expand-button.png
  ```

  **提交**：否（包含在第3次提交中）

---

## 提交策略

- **提交1**: `refactor(ui): 添加分割器和标签页基础设施` - dc_app.h, dc_app.cpp
- **提交2**: `feat(ui): 实现标签页式参数面板` - main.cpp
- **提交3**: `feat(ui): 添加分割线和折叠功能` - main.cpp

---

## 成功标准

### 验证命令
```bash
# 编译项目
cd demo/dual_contouring && mkdir -p build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release

# 运行程序
./build/Release/demoGlfwOpenGL.exe
```

### 最终检查清单
- [ ] 所有"必须有"项存在
- [ ] 所有"禁止有"项不存在
- [ ] 4个标签页切换正常
- [ ] 分割线拖动正常
- [ ] 折叠/展开正常
- [ ] 窗口可移动
- [ ] 无编译错误