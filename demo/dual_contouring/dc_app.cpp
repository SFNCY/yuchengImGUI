#include "dc_app.h"
#include "dc_mesh.h"
#include "dc_renderer.h"

#include "imgui.h"

// ============================================================================
// 颜色定义 - 不同阶段使用不同颜色区分
// ============================================================================

// 前向声明
namespace ImGui {
    static void ColorSwatch(const char* label, ImU32 color);
}

/// @brief 四叉树阶段颜色
static constexpr ImU32 COLOR_TREE_NODE = IM_COL32(100, 200, 255, 255);   ///< 叶子节点 - 浅蓝
static constexpr ImU32 COLOR_TREE_INTERNAL = IM_COL32(60, 120, 180, 255);  ///< 内部节点 - 深蓝
static constexpr ImU32 COLOR_TREE_DEPTH_1 = IM_COL32(255, 100, 100, 255);  ///< 深度1 - 红
static constexpr ImU32 COLOR_TREE_DEPTH_2 = IM_COL32(255, 200, 100, 255);  ///< 深度2 - 橙
static constexpr ImU32 COLOR_TREE_DEPTH_3 = IM_COL32(100, 255, 100, 255);  ///< 深度3 - 绿
static constexpr ImU32 COLOR_TREE_DEPTH_4 = IM_COL32(100, 200, 255, 255);  ///< 深度4 - 青

/// @brief QEF 阶段颜色
static constexpr ImU32 COLOR_HERMITE_POINT = IM_COL32(100, 255, 100, 255);  ///< Hermite 交点 - 绿色
static constexpr ImU32 COLOR_QEF_SOLVED = IM_COL32(100, 100, 255, 255);    ///< QEF 解算点 - 蓝色
static constexpr ImU32 COLOR_QEF_LINE = IM_COL32(255, 255, 100, 255);      ///< QEF 向量 - 黄色

// ============================================================================
// RenderTreeStructure - 可视化四叉树结构
// ============================================================================
void RenderTreeStructure(const dc::QuadtreeNode* root, Renderer* renderer)
{
    if (!root || !renderer)
        return;

    // 递归绘制四叉树节点
    // 使用深度优先遍历，根据深度选择不同颜色
    struct StackItem {
        const dc::QuadtreeNode* node;
        int depth;
    };
    
    // 使用栈进行迭代遍历，避免递归栈溢出
    StackItem stack[64];  // 假设最大深度不超过64
    int stackTop = 0;
    
    stack[stackTop++] = {root, 0};
    
    while (stackTop > 0) {
        stackTop--;
        StackItem item = stack[stackTop];
        const dc::QuadtreeNode* node = item.node;
        int depth = item.depth;
        
        if (!node)
            continue;

        // 根据深度选择颜色
        // 深度越大，颜色越深
        ImU32 color;
        switch (depth % 4) {
            case 0: color = COLOR_TREE_DEPTH_1; break;
            case 1: color = COLOR_TREE_DEPTH_2; break;
            case 2: color = COLOR_TREE_DEPTH_3; break;
            default: color = COLOR_TREE_DEPTH_4; break;
        }
        
        // 判断是叶子节点还是内部节点
        // 叶子节点用实线，内部节点用虚线（通过不同粗细模拟）
        float thickness = node->subdivided ? 1.0f : 2.0f;
        
        // 计算边界框尺寸
        // bounds.halfSize 是半尺寸，需要乘以2得到完整尺寸
        float size = node->bounds.halfSize * 2.0f;
        
        // 绘制节点边界框（正方形）
        renderer->DrawQuad(
            node->bounds.center,
            size,
            color,
            0.0f,   // 无旋转
            thickness
        );
        
        // 如果是叶子节点，在中心绘制一个点
        if (!node->subdivided) {
            renderer->DrawCircleFilled(
                node->bounds.center,
                3.0f,
                color
            );
        }
        
        // 将子节点入栈（如果存在）
        // 按深度顺序入栈：先处理较深的节点
        if (node->nw) stack[stackTop++] = {node->nw, depth + 1};
        if (node->ne) stack[stackTop++] = {node->ne, depth + 1};
        if (node->sw) stack[stackTop++] = {node->sw, depth + 1};
        if (node->se) stack[stackTop++] = {node->se, depth + 1};
    }
}

// ============================================================================
// RenderQFESolving - 可视化 QEF 求解过程
// ============================================================================
//
// QEF (Quadratic Error Function) 求解过程：
// 1. 对于每个包含表面的叶子节点，找到法线与边界的交点（Hermite 数据）
// 2. 使用这些交点构建 QEF 方程
// 3. 求解 QEF 得到顶点位置
//
// 当前简化实现：
// - 在每个叶子节点中心显示一个点（表示 QEF 解算点）
// - 在边界中点显示小圆点（表示 Hermite 交点）
// - 用线条连接相关点
// ============================================================================
void RenderQFESolving(const dc::QuadtreeNode* root, Renderer* renderer)
{
    if (!root || !renderer)
        return;

    // 遍历四叉树，找到所有叶子节点
    struct StackItem {
        const dc::QuadtreeNode* node;
    };
    
    StackItem stack[64];
    int stackTop = 0;
    stack[stackTop++] = {root};
    
    while (stackTop > 0) {
        stackTop--;
        const dc::QuadtreeNode* node = stack[stackTop].node;
        
        if (!node)
            continue;
        
        // 如果是叶子节点，绘制 QEF 相关内容
        if (!node->subdivided) {
            const float hs = node->bounds.halfSize;
            const ImVec2& center = node->bounds.center;
            
            // 绘制 Hermite 交点（边界中点）
            // 上边界中点
            renderer->DrawCircleFilled(
                ImVec2(center.x, center.y + hs),
                4.0f,
                COLOR_HERMITE_POINT
            );
            // 下边界中点
            renderer->DrawCircleFilled(
                ImVec2(center.x, center.y - hs),
                4.0f,
                COLOR_HERMITE_POINT
            );
            // 左边界中点
            renderer->DrawCircleFilled(
                ImVec2(center.x - hs, center.y),
                4.0f,
                COLOR_HERMITE_POINT
            );
            // 右边界中点
            renderer->DrawCircleFilled(
                ImVec2(center.x + hs, center.y),
                4.0f,
                COLOR_HERMITE_POINT
            );
            
            // 绘制 QEF 解算点（节点中心）- 使用较大圆点
            renderer->DrawCircleFilled(
                center,
                6.0f,
                COLOR_QEF_SOLVED
            );
            
            // 绘制从中心到边界的连线（表示 QEF 求解方向）
            // 上
            renderer->DrawLine(
                center,
                ImVec2(center.x, center.y + hs),
                COLOR_QEF_LINE,
                1.0f
            );
            // 下
            renderer->DrawLine(
                center,
                ImVec2(center.x, center.y - hs),
                COLOR_QEF_LINE,
                1.0f
            );
            // 左
            renderer->DrawLine(
                center,
                ImVec2(center.x - hs, center.y),
                COLOR_QEF_LINE,
                1.0f
            );
            // 右
            renderer->DrawLine(
                center,
                ImVec2(center.x + hs, center.y),
                COLOR_QEF_LINE,
                1.0f
            );
        }
        
        // 继续遍历子节点
        if (node->nw) stack[stackTop++] = {node->nw};
        if (node->ne) stack[stackTop++] = {node->ne};
        if (node->sw) stack[stackTop++] = {node->sw};
        if (node->se) stack[stackTop++] = {node->se};
    }
}

// ============================================================================
// RenderAllStages - 根据当前阶段渲染
// ============================================================================
//
// 分步渲染逻辑：
// - stepMode = false：直接渲染 visStage 指定的阶段
//   - TREE_BUILD: 只渲染四叉树结构
//   - QEF_SOLVE: 只渲染 QEF 求解过程
//   - MESH_GENERATE: 只渲染网格（通过 mesh 指针）
//   - ALL: 渲染所有阶段
//
// - stepMode = true：根据 currentStep 决定渲染内容
//   - Step 0: 只渲染 TREE_BUILD
//   - Step 1: 渲染 TREE_BUILD + QEF_SOLVE
//   - Step 2+: 渲染全部阶段
// ============================================================================
void RenderAllStages(const AppState* state, Renderer* renderer, const dc::QuadtreeNode* root)
{
    if (!state || !renderer)
        return;
    
    // 分步模式判断
    if (state->stepMode) {
        // 分步模式下根据 currentStep 决定渲染内容
        switch (state->currentStep) {
            case 0:
                // 阶段1：只渲染四叉树构建
                RenderTreeStructure(root, renderer);
                break;
            case 1:
                // 阶段2：渲染四叉树 + QEF 求解
                RenderTreeStructure(root, renderer);
                RenderQFESolving(root, renderer);
                break;
            default:
                // 阶段3+：渲染全部阶段
                RenderTreeStructure(root, renderer);
                RenderQFESolving(root, renderer);
                // 网格渲染由调用方负责（需要 mesh 数据）
                break;
        }
    } else {
        // 连续模式下直接根据 visStage 渲染
        switch (state->visStage) {
            case VisStage::TREE_BUILD:
                RenderTreeStructure(root, renderer);
                break;
            case VisStage::QEF_SOLVE:
                RenderQFESolving(root, renderer);
                break;
            case VisStage::MESH_GENERATE:
                // 网格渲染由调用方负责
                break;
            case VisStage::ALL:
                // 全部渲染
                RenderTreeStructure(root, renderer);
                RenderQFESolving(root, renderer);
                break;
        }
    }
}

// ============================================================================
// RenderStepControls - 渲染分步控制按钮
// ============================================================================
//
// 渲染分步可视化的控制面板，包括：
// - 模式切换：连续模式 / 分步模式
// - 阶段选择：树构建 / QEF 求解 / 网格生成 / 全部
// - 步进控制：上一步 / 下一步（仅分步模式显示）
// - 自动播放：播放 / 暂停（仅分步模式显示）
// - 进度指示：当前步 / 总步数
// ============================================================================
void RenderStepControls(AppState* state)
{
    if (!state)
        return;
    
    // =========================================================================
    // 分步模式开关
    // =========================================================================
    ImGui::SeparatorText("分步可视化控制");
    
    // 模式切换：连续模式 / 分步模式
    if (ImGui::Checkbox("启用分步模式", &state->stepMode)) {
        // 切换模式时重置步数
        state->currentStep = 0;
    }
    
    // =========================================================================
    // 阶段选择（仅连续模式显示）
    // =========================================================================
    if (!state->stepMode) {
        ImGui::Text("选择可视化阶段：");
        
        // 使用单选按钮选择阶段
        int stageIndex = static_cast<int>(state->visStage);
        
        if (ImGui::RadioButton("仅树结构", &stageIndex, static_cast<int>(VisStage::TREE_BUILD))) {
            state->visStage = VisStage::TREE_BUILD;
        }
        ImGui::SameLine();
        
        if (ImGui::RadioButton("仅 QEF", &stageIndex, static_cast<int>(VisStage::QEF_SOLVE))) {
            state->visStage = VisStage::QEF_SOLVE;
        }
        
        if (ImGui::RadioButton("仅网格", &stageIndex, static_cast<int>(VisStage::MESH_GENERATE))) {
            state->visStage = VisStage::MESH_GENERATE;
        }
        ImGui::SameLine();
        
        if (ImGui::RadioButton("全部", &stageIndex, static_cast<int>(VisStage::ALL))) {
            state->visStage = VisStage::ALL;
        }
    }
    
    // =========================================================================
    // 分步模式控制（仅分步模式显示）
    // =========================================================================
    if (state->stepMode) {
        ImGui::Text("算法阶段进度：");
        
        // 显示当前阶段名称
        const char* stageName = "未知";
        switch (state->currentStep) {
            case 0: stageName = "四叉树构建中..."; break;
            case 1: stageName = "QEF 求解中..."; break;
            default: stageName = "网格生成中..."; break;
        }
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "%s", stageName);
        
        // 进度条显示
        float progress = static_cast<float>(state->currentStep) / 2.0f;
        ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f), "");
        
        // 步数指示
        ImGui::Text("步骤: %d / 2", state->currentStep);
        
        ImGui::Spacing();
        
        // 步进控制按钮
        ImGui::Text("步进控制：");
        
        // 上一步按钮
        ImGui::BeginDisabled(state->currentStep <= 0);
        if (ImGui::ArrowButton("##prev", ImGuiDir_Left)) {
            if (state->currentStep > 0) {
                state->currentStep--;
            }
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        
        // 下一步按钮
        ImGui::BeginDisabled(state->currentStep >= 2);
        if (ImGui::ArrowButton("##next", ImGuiDir_Right)) {
            if (state->currentStep < 2) {
                state->currentStep++;
            }
        }
        ImGui::EndDisabled();
        
        ImGui::SameLine();
        
        // 重置按钮
        if (ImGui::Button("重置")) {
            state->currentStep = 0;
        }
        
        ImGui::Spacing();
        
        // 自动播放控制
        ImGui::Text("自动播放：");
        ImGui::Checkbox("启用自动播放", &state->autoPlay);
        
        if (state->autoPlay) {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(100.0f);
            
            // 间隔时间滑块
            int interval = state->autoPlayInterval;
            if (ImGui::SliderInt("ms", &interval, 100, 2000)) {
                state->autoPlayInterval = interval;
            }
            
            // 根据时间自动步进
            static float lastTime = 0.0f;
            float currentTime = ImGui::GetTime();
            
            if (currentTime - lastTime > state->autoPlayInterval / 1000.0f) {
                if (state->currentStep < 2) {
                    state->currentStep++;
                } else {
                    // 到达最后一步时停止
                    state->autoPlay = false;
                }
                lastTime = currentTime;
            }
        }
    }
    
    ImGui::Separator();
    
    // =========================================================================
    // 颜色图例
    // =========================================================================
    ImGui::Text("颜色图例：");
    
    // 四叉树颜色
    ImGui::ColorSwatch("##tree1", COLOR_TREE_DEPTH_1); ImGui::SameLine();
    ImGui::Text("深度1"); ImGui::SameLine(100.0f);
    ImGui::ColorSwatch("##tree2", COLOR_TREE_DEPTH_2); ImGui::SameLine();
    ImGui::Text("深度2");
    
    ImGui::ColorSwatch("##tree3", COLOR_TREE_DEPTH_3); ImGui::SameLine();
    ImGui::Text("深度3"); ImGui::SameLine(100.0f);
    ImGui::ColorSwatch("##tree4", COLOR_TREE_DEPTH_4); ImGui::SameLine();
    ImGui::Text("深度4");
    
    ImGui::Spacing();
    
    // QEF 颜色
    ImGui::ColorSwatch("##hermite", COLOR_HERMITE_POINT); ImGui::SameLine();
    ImGui::Text("Hermite 交点"); ImGui::SameLine(100.0f);
    ImGui::ColorSwatch("##qef", COLOR_QEF_SOLVED); ImGui::SameLine();
    ImGui::Text("QEF 解算点");
}

// ============================================================================
// 辅助函数：ImGui::ColorSwatch - 绘制颜色样本
// ============================================================================
// 在 ImGui 中显示一个小颜色方块
// ============================================================================
namespace ImGui {
    // ColorSwatch - 显示颜色样本的辅助函数
    // 使用 GetWindowDrawList 直接绘制矩形
    static void ColorSwatch(const char* label, ImU32 color) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        
        // 绘制小方块
        draw_list->AddRectFilled(
            pos,
            ImVec2(pos.x + 16.0f, pos.y + 16.0f),
            color
        );
        
        // 移动光标位置
        ImGui::SetCursorScreenPos(ImVec2(pos.x + 20.0f, pos.y));
    }
} // namespace ImGui

// ============================================================================
// 渲染控制面板
// ============================================================================
//
// 创建 ImGui 控制面板，允许用户调整算法参数和可视化选项。
// 面板使用单列布局，位于窗口顶部。
//
// 参数：
//   state 应用程序状态指针（必须非空）
// ============================================================================
void RenderControlPanel(AppState* state)
{
    // 安全检查
    if (state == nullptr) {
        return;
    }
    
    // 设置面板宽度
    const float panel_width = 280.0f;
    
    // 开始控制面板窗口
    // 使用 ImGuiWindowFlags_AlwaysAutoResize 让窗口根据内容自动调整大小
    ImGui::SetNextWindowSize(ImVec2(panel_width, 0));
    ImGui::Begin("控制面板", nullptr, 
        ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse);
    
    // ========================================================================
    // 算法参数区域
    // ========================================================================
    
    ImGui::Text("算法参数");
    ImGui::Separator();
    
    // 最大深度滑块
    // 范围: [1, 8]，默认值: 4
    // 深度越大，四叉树越精细，计算量越大
    if (ImGui::SliderInt("最大深度", &state->maxDepth, 1, 8, "%.0f")) {
        // 滑块值改变时自动保存到 state
        // 用户可以在滑块上右击输入精确值
    }
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), 
        "(%d)", state->maxDepth);
    
    // QEF 阈值滑块
    // 范围: [0.001f, 1.0f]，默认值: 0.1
    // 阈值越小，对顶点位置要求越严格
    if (ImGui::SliderFloat("QEF 阈值", &state->qefThreshold, 0.001f, 1.0f, "%.3f", 
        ImGuiSliderFlags_Logarithmic)) {
        // 使用对数刻度，因为阈值跨越多个数量级
    }
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), 
        "(%.3f)", state->qefThreshold);
    
    ImGui::Spacing();
    
    // ========================================================================
    // 可视化选项区域
    // ========================================================================
    
    ImGui::Text("可视化选项");
    ImGui::Separator();
    
    // SDF 相关可视化
    if (ImGui::CollapsingHeader("SDF 可视化", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("SDF 等值线", &state->showSDFContour);
        ImGui::Checkbox("法线向量", &state->showNormals);
    }
    
    // 四叉树可视化
    if (ImGui::CollapsingHeader("四叉树可视化", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("四叉树边界", &state->showQuadtreeBounds);
        ImGui::Checkbox("节点中心点", &state->showQuadtreeNodes);
    }
    
    // 对偶网格可视化
    if (ImGui::CollapsingHeader("对偶网格可视化", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("对偶网格", &state->showDualMesh);
        ImGui::Checkbox("QEF 交点", &state->showQEFIntersections);
    }
    
    ImGui::End();
}

// ============================================================================
// 渲染信息面板
// ============================================================================
//
// 显示网格统计信息和性能指标。
// 面板显示在控制面板下方。
//
// 参数：
//   state 应用程序状态指针
//   mesh 对偶网格指针（可以为 nullptr）
// ============================================================================
void RenderInfoPanel(AppState* state, const dc::DualMesh* mesh)
{
    // 安全检查
    if (state == nullptr) {
        return;
    }
    
    // 开始信息面板窗口
    ImGui::SetNextWindowSize(ImVec2(280.0f, 0));
    ImGui::Begin("统计信息", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse);
    
    // ========================================================================
    // 帧率信息
    // ========================================================================
    
    ImGui::Text("性能指标");
    ImGui::Separator();
    
    ImGuiIO& io = ImGui::GetIO();
    float fps = io.Framerate;
    float frame_time = 1000.0f / fps;
    
    ImGui::Text("帧率: %.1f FPS", fps);
    ImGui::Text("帧时间: %.2f ms", frame_time);
    
    ImGui::Spacing();
    
    // ========================================================================
    // 对偶网格统计
    // ========================================================================
    
    ImGui::Text("对偶网格");
    ImGui::Separator();
    
    int vertex_count = 0;
    int edge_count = 0;
    
    if (mesh != nullptr && mesh->IsValid()) {
        vertex_count = static_cast<int>(mesh->vertices.size());
        edge_count = static_cast<int>(mesh->edges.size());
    } else {
        // 使用 state 中缓存的值
        vertex_count = state->meshVertexCount;
        edge_count = state->meshEdgeCount;
    }
    
    ImGui::Text("顶点数: %d", vertex_count);
    ImGui::Text("边数: %d", edge_count);
    
    ImGui::Spacing();
    
    // ========================================================================
    // 四叉树统计
    // ========================================================================
    
    ImGui::Text("四叉树");
    ImGui::Separator();
    
    ImGui::Text("节点数: %d", state->quadtreeNodeCount);
    ImGui::Text("当前深度: %d", state->maxDepth);
    
    ImGui::Spacing();
    
    // ========================================================================
    // 调试信息
    // ========================================================================
    
    ImGui::Text("调试");
    ImGui::Separator();
    
    ImGui::Text("SDF 评估次数: %d", state->sdfEvalCount);
    
    ImGui::End();
}

// ============================================================================
// 更新统计信息
// ============================================================================
//
// 从对偶网格提取统计信息并更新到 AppState。
// 在每次重建网格后调用此函数更新统计显示。
//
// 参数：
//   state 应用程序状态指针（更新此结构体）
//   mesh 对偶网格指针（读取此结构体）
// ============================================================================
void UpdateStatsFromMesh(AppState* state, const dc::DualMesh* mesh)
{
    if (state == nullptr) {
        return;
    }
    
    if (mesh != nullptr && mesh->IsValid()) {
        state->meshVertexCount = static_cast<int>(mesh->vertices.size());
        state->meshEdgeCount = static_cast<int>(mesh->edges.size());
    }
}

// ============================================================================
// 形状编辑器函数实现
// ============================================================================

// ============================================================================
// AddCircle - 添加圆形形状
// ============================================================================
void AddCircle(AppState* state, ImVec2 pos, float radius)
{
    if (state == nullptr) {
        return;
    }
    state->shapes.push_back(ShapeInfo(ShapeType::Circle, pos, radius));
}

// ============================================================================
// AddSquare - 添加方形形状
// ============================================================================
void AddSquare(AppState* state, ImVec2 pos, float size)
{
    if (state == nullptr) {
        return;
    }
    state->shapes.push_back(ShapeInfo(ShapeType::Square, pos, size));
}

// ============================================================================
// RemoveShape - 删除指定形状
// ============================================================================
void RemoveShape(AppState* state, int index)
{
    if (state == nullptr) {
        return;
    }
    
    int count = (int)state->shapes.size();
    if (index < 0 || index >= count) {
        return;
    }
    
    state->shapes.erase(state->shapes.begin() + index);
    
    // 如果删除的是选中的形状，或者选中索引超出范围，清除选中状态
    if (state->selectedShape == index || state->selectedShape >= count) {
        state->selectedShape = -1;
    }
}

// ============================================================================
// RenderShapeEditor - 渲染形状编辑器面板
// ============================================================================
void RenderShapeEditor(AppState* state)
{
    if (state == nullptr || !state->showShapeEditor) {
        return;
    }
    
    // 设置窗口宽度
    const float panel_width = 300.0f;
    ImGui::SetNextWindowSize(ImVec2(panel_width, 400.0f));
    
    // 开始形状编辑器窗口
    if (!ImGui::Begin("形状编辑器", &state->showShapeEditor,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }
    
    // ========================================================================
    // 形状列表区域
    // ========================================================================
    
    ImGui::Text("形状列表");
    ImGui::Separator();
    
    int count = (int)state->shapes.size();
    
    if (count == 0) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), 
            "暂无形状，请添加");
    } else {
        // 显示每个形状
        for (int i = 0; i < count; i++) {
            const ShapeInfo& shape = state->shapes[i];
            
            // 确定形状名称
            const char* shape_name = (shape.type == ShapeType::Circle) ? "圆形" : "方形";
            
            // 选中高亮
            if (state->selectedShape == i) {
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.5f, 0.8f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.6f, 0.9f, 1.0f));
            }
            
            // 创建可选择的行
            char label[128];
            snprintf(label, sizeof(label), "%s #%d | 位置:(%.2f, %.2f) 大小:%.2f", 
                shape_name, i, shape.position.x, shape.position.y, shape.size);
            
            bool is_selected = (state->selectedShape == i);
            if (ImGui::Selectable(label, &is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                state->selectedShape = i;
            }
            
            if (state->selectedShape == i) {
                ImGui::PopStyleColor(2);
            }
        }
    }
    
    ImGui::Spacing();
    
    // ========================================================================
    // 选中形状编辑区域
    // ========================================================================
    
    if (state->selectedShape >= 0 && state->selectedShape < count) {
        ImGui::Separator();
        ImGui::Text("编辑选中形状");
        ImGui::Separator();
        
        ShapeInfo& shape = state->shapes[state->selectedShape];
        
        // 位置编辑
        float pos_x = shape.position.x;
        float pos_y = shape.position.y;
        
        ImGui::InputFloat("位置 X", &pos_x, 0.1f, 0.5f, "%.2f");
        ImGui::InputFloat("位置 Y", &pos_y, 0.1f, 0.5f, "%.2f");
        
        shape.position.x = pos_x;
        shape.position.y = pos_y;
        
        // 大小编辑
        float size = shape.size;
        ImGui::InputFloat("大小", &size, 0.1f, 0.5f, "%.2f");
        if (size > 0.001f) {
            shape.size = size;
        }
        
        ImGui::Spacing();
        
        // 删除按钮
        if (ImGui::Button("删除选中形状", ImVec2(-1, 0))) {
            RemoveShape(state, state->selectedShape);
        }
    }
    
    ImGui::Spacing();
    
    // ========================================================================
    // 添加形状按钮
    // ========================================================================
    
    ImGui::Separator();
    ImGui::Text("添加形状");
    ImGui::Separator();
    
    // 使用表格布局排列按钮
    float button_width = (panel_width - 20.0f) / 2.0f;
    
    if (ImGui::Button("添加圆形", ImVec2(button_width, 0))) {
        // 在中心位置添加一个默认大小的圆形
        AddCircle(state, ImVec2(0.0f, 0.0f), 0.5f);
        state->selectedShape = (int)state->shapes.size() - 1;
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("添加方形", ImVec2(button_width, 0))) {
        // 在中心位置添加一个默认大小的方形
        AddSquare(state, ImVec2(0.0f, 0.0f), 1.0f);
        state->selectedShape = (int)state->shapes.size() - 1;
    }
    
    ImGui::End();
}

// ============================================================================
// GetCombinedSDF - 获取复合 SDF
// ============================================================================
// 使用静态 buffer 存储所有 SDF 对象，避免每次调用时重新分配
// 最大支持 32 个形状
static const int MAX_SHAPES = 32;
static SDFCircle g_circles[MAX_SHAPES];
static SDFSquare g_squares[MAX_SHAPES];
static SDFUnion g_unions[MAX_SHAPES - 1];
static SDFCircle g_defaultCircle(0.0f, 0.0f, 0.001f);  // 极小的默认圆形

const SDFBase* GetCombinedSDF(AppState* state)
{
    if (state == nullptr || state->shapes.empty()) {
        return &g_defaultCircle;
    }
    
    int count = (int)state->shapes.size();
    if (count > MAX_SHAPES) {
        count = MAX_SHAPES;
    }
    
    // 构建所有形状的 SDF
    for (int i = 0; i < count; i++) {
        const ShapeInfo& shape = state->shapes[i];
        if (shape.type == ShapeType::Circle) {
            g_circles[i] = SDFCircle(shape.position.x, shape.position.y, shape.size);
        } else {
            g_squares[i] = SDFSquare(shape.position.x, shape.position.y, shape.size);
        }
    }
    
    // 如果只有一个形状
    if (count == 1) {
        const ShapeInfo& shape = state->shapes[0];
        if (shape.type == ShapeType::Circle) {
            return &g_circles[0];
        } else {
            return &g_squares[0];
        }
    }
    
    // 多个形状：构建并集链
    // 从后向前两两合并
    for (int i = 0; i < count - 1; i++) {
        g_unions[i].sdf1 = (SDFBase*)&g_circles[i];  // 安全的类型转换
        g_unions[i].sdf2 = (SDFBase*)&g_unions[i + 1];
    }
    
    // 最后一个 union 的 sdf2 指向最后一个形状
    g_unions[count - 2].sdf2 = (state->shapes[count - 1].type == ShapeType::Circle) 
        ? (SDFBase*)&g_circles[count - 1] 
        : (SDFBase*)&g_squares[count - 1];
    
    return &g_unions[0];
}
