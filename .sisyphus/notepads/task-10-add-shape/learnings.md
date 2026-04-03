# Task 10 - 交互式形状编辑器 学习记录

## 任务概述
扩展 `dc_app.h` 和 `dc_app.cpp`，实现交互式形状编辑器功能。

## 完成内容

### 1. dc_app.h 扩展
- 添加 `ShapeType` 枚举（Circle, Square）
- 添加 `ShapeInfo` 结构体（type, position, size）
- 扩展 `AppState`：
  - `std::vector<ShapeInfo> shapes` - 形状列表
  - `int selectedShape` - 当前选中形状索引
  - `bool showShapeEditor` - 编辑器可见性
  - `GetShapeCount()` 和 `HasShapes()` 辅助方法

### 2. dc_app.cpp 实现
- `AddCircle()` - 添加圆形到形状列表
- `AddSquare()` - 添加方形到形状列表
- `RemoveShape()` - 从形状列表删除形状
- `RenderShapeEditor()` - ImGui 形状编辑面板
  - 显示形状列表（可选择）
  - 编辑选中形状的位置和大小
  - 添加圆形/方形按钮
  - 删除选中形状按钮
- `GetCombinedSDF()` - 获取复合 SDF
  - 使用静态 buffer 存储最多 32 个形状
  - 遍历所有形状构建 SDFUnion 链

## 关键设计决策

### SDF 并集链构建
- 使用静态数组 `g_circles[MAX_SHAPES]`, `g_squares[MAX_SHAPES]`, `g_unions[MAX_SHAPES-1]`
- 从后向前两两合并，最后一个 union 指向实际形状
- 避免了每次调用时重新分配内存的问题

### 形状选择 UI
- 使用 `ImGui::Selectable` 实现列表选择
- 选中高亮显示
- 实时编辑位置和大小参数

## 已知限制
- 最多支持 32 个形状（静态 buffer 限制）
- 线程不安全（静态存储）
- LSP 报告 imgui.h 找不到（实际编译不受影响）

## 文件变更
- `demo/dual_contouring/dc_app.h` - 扩展 AppState，添加函数声明
- `demo/dual_contouring/dc_app.cpp` - 实现形状编辑函数
