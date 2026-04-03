// Dual Contouring Demo - Complete Application
// 基于 Dear ImGui GLFW + OpenGL 3

#include "dc_app.h"
#include "dc_mesh.h"
#include "dc_quadtree.h"
#include "dc_contouring.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

// ============================================================================
// 错误回调
// ============================================================================
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// ============================================================================
// 主循环前的初始化
// ============================================================================
// OpenGL initialization is handled by GLFW/GLAD when glfwMakeContextCurrent is called

// ============================================================================
// 主代码
// ============================================================================
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    // Create window - size will be updated after ImGui context is created
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dual Contouring Demo", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    
    // Update window to use display size (must be done after first frame)
    // Note: io.DisplaySize is not valid here, so we use a reasonable default 1280x720
    // and rely on the render loop to handle resizing properly
    // glfwSetWindowSize(window, (int)io.DisplaySize.x, (int)io.DisplaySize.y);

    // Setup style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Chinese font: Try multiple paths
    ImFont* font = nullptr;
    
    // Try project fonts folder (relative to working directory at runtime)
    const char* font_paths[] = {
        "../../fonts/Pang正道标题体.ttf",
        "fonts/Pang正道标题体.ttf",
        "C:/Windows/Fonts/msyh.ttc",     // Microsoft YaHei
        "C:/Windows/Fonts/simhei.ttf",   // SimHei
    };
    
    for (int i = 0; i < IM_ARRAYSIZE(font_paths); i++) {
        font = io.Fonts->AddFontFromFileTTF(font_paths[i], 20.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
        if (font != nullptr) {
            fprintf(stderr, "[OK] Loaded Chinese font from: %s\n", font_paths[i]);
            break;
        } else {
            fprintf(stderr, "[WARN] Failed to load font from: %s\n", font_paths[i]);
        }
    }
    
    if (font == nullptr) {
        fprintf(stderr, "[ERROR] Could not load any Chinese font! Using default font.\n");
    }

    // ============================================================================
    // 应用状态初始化
    // ============================================================================
    AppState state;
    
    // 添加默认圆形场景
    AddCircle(&state, ImVec2(0.0f, 0.0f), 0.5f);
    
    // 初始化 OpenGL (已在 GLFW/GLAD 初始化时完成)
    // initOpenGL();

    // ============================================================================
    // DC 数据结构
    // ============================================================================
    dc::Quadtree quadtree;
    dc::QuadtreeNode* root = nullptr;
    dc::DualMesh mesh;
    dc::ContouringContext contourContext;
    
    // 脏标记 - 用于检测参数/形状变化
    bool needsRebuild = true;
    
    // 渲染区域边界
    const float canvasSize = 4.0f;  // 画布半尺寸
    ImVec4 canvasBounds = ImVec4(-canvasSize, -canvasSize, canvasSize, canvasSize);

    // Main loop
#ifdef __EMSCRIPTEN__
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ============================================================================
        // 检测参数变化
        // ============================================================================
        static int lastMaxDepth = -1;
        static float lastQefThreshold = -1.0f;
        static int lastShapeCount = -1;
        
        // 检查是否需要重建四叉树和网格
        if (state.maxDepth != lastMaxDepth || 
            state.qefThreshold != lastQefThreshold ||
            (int)state.shapes.size() != lastShapeCount) {
            needsRebuild = true;
            lastMaxDepth = state.maxDepth;
            lastQefThreshold = state.qefThreshold;
            lastShapeCount = (int)state.shapes.size();
        }
        
        // 重建四叉树和网格（如果需要）
        // ============================================================================
        if (needsRebuild) {
            // 清理旧数据 - createRoot 内部会调用 deleteTree(m_root) 清理旧树
            // 注意：不需要在这里调用 deleteTree，因为 createRoot 已经管理树的清理
            mesh.Clear();
            
            // 获取复合 SDF
            const SDFBase* sdf = GetCombinedSDF(&state);
            
            // 创建并构建四叉树
            root = quadtree.createRoot(ImVec2(0.0f, 0.0f), canvasSize, state.maxDepth);
            quadtree.build(root, sdf, state.maxDepth);
            
            // 统计节点数
            state.quadtreeNodeCount = (int)dc::Quadtree::countNodes(root);
            
            // 生成轮廓线顶点
            contourContext.sdf = sdf;
            contourContext.maxDepth = state.maxDepth;
            contourContext.qefThreshold = state.qefThreshold;
            contourContext.vertices.clear();
            dc::ContourNode(root, &contourContext);
            
            // 构建对偶网格
            mesh = dc::BuildDualMesh(root, &contourContext);
            
            // 更新统计信息
            UpdateStatsFromMesh(&state, &mesh);
            
            // 重置脏标记
            needsRebuild = false;
        }

        // ============================================================================
        // 自适应布局：左右分隔栏
        // ============================================================================
        // 新的布局结构：
        // +------------------+------------------+
        // |   参数面板      |                  |
        // |   (4个tab)      |    渲染画布      |
        // |                  |                  |
        // +------------------+------------------+
        // |  分布可视化路径   |                  |
        // |   (第5个tab)     |                  |
        // +------------------+------------------+
        
        // 左右分隔比例 (0.0-1.0)，控制左侧面板宽度
        static float horizontalSplitterRatio = 0.25f;  // 默认左侧占 25%
        static bool horizontalSplitterDragging = false;
        
        // 上下分隔比例 (0.0-1.0)，控制左侧上下区域高度
        static float verticalSplitterRatio = 0.65f;  // 默认上部分占 65%，底部留更多空间
        static bool verticalSplitterDragging = false;
        
        // 计算分隔后各区域尺寸
        const float totalWidth = io.DisplaySize.x;
        const float totalHeight = io.DisplaySize.y;
        const float minPanelWidth = 200.0f;
        const float minPanelHeight = 100.0f;
        
        // 左右分隔栏位置
        float leftPanelWidth = totalWidth * horizontalSplitterRatio;
        if (leftPanelWidth < minPanelWidth) leftPanelWidth = minPanelWidth;
        if (leftPanelWidth > totalWidth - minPanelWidth) leftPanelWidth = totalWidth - minPanelWidth;
        float rightCanvasWidth = totalWidth - leftPanelWidth - 8.0f;  // 8.0f 是分隔栏厚度
        
        // 上下分隔栏位置（在左侧面板内）
        float leftTopHeight = totalHeight * verticalSplitterRatio;
        if (leftTopHeight < minPanelHeight) leftTopHeight = minPanelHeight;
        if (leftTopHeight > totalHeight - minPanelHeight) leftTopHeight = totalHeight - minPanelHeight;
        float leftBottomHeight = totalHeight - leftTopHeight - 8.0f;  // 8.0f 是分隔栏厚度
        
        // ============================================================================
        // 左侧参数面板 - 使用 Child Window 实现
        // ============================================================================
        {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(leftPanelWidth, totalHeight), ImGuiCond_Always);
            
            ImGui::Begin("##left_panel", nullptr,
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoMove);
            
            // 上半部分：4个标签页
            {
                ImGui::BeginChild("##top_section", ImVec2(leftPanelWidth - 4.0f, leftTopHeight - 4.0f), true);
                
                if (ImGui::BeginTabBar("ParameterTabs")) {
                    // 算法参数标签
                    if (ImGui::BeginTabItem("算法参数")) {
                        ImGui::Text("算法参数");
                        ImGui::Separator();

                        // 最大深度滑块
                        if (ImGui::SliderInt("最大深度", &state.maxDepth, 1, 8, "%.0f")) {
                        }
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(%d)", state.maxDepth);

                        // QEF阈值滑块
                        if (ImGui::SliderFloat("QEF 阈值", &state.qefThreshold, 0.001f, 1.0f, "%.3f", ImGuiSliderFlags_Logarithmic)) {
                        }
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(%.3f)", state.qefThreshold);
                        
                        // 上下分隔比例滑块（调试用）
                        ImGui::Separator();
                        ImGui::Text("布局调试：");
                        ImGui::SliderFloat("上下比例", &verticalSplitterRatio, 0.2f, 0.8f, "%.2f");
                        ImGui::SliderFloat("左右比例", &horizontalSplitterRatio, 0.15f, 0.4f, "%.2f");
                        
                        ImGui::EndTabItem();
                    }
                    // 可视化选项标签
                    if (ImGui::BeginTabItem("可视化选项")) {
                        // SDF 相关可视化
                        if (ImGui::CollapsingHeader("SDF 可视化", ImGuiTreeNodeFlags_DefaultOpen)) {
                            ImGui::Checkbox("SDF 等值线", &state.showSDFContour);
                            ImGui::Checkbox("法线向量", &state.showNormals);
                        }
                        
                        // 四叉树可视化
                        if (ImGui::CollapsingHeader("四叉树可视化", ImGuiTreeNodeFlags_DefaultOpen)) {
                            ImGui::Checkbox("四叉树边界", &state.showQuadtreeBounds);
                            ImGui::Checkbox("节点中心点", &state.showQuadtreeNodes);
                        }
                        
                        // 对偶网格可视化
                        if (ImGui::CollapsingHeader("对偶网格可视化", ImGuiTreeNodeFlags_DefaultOpen)) {
                            ImGui::Checkbox("对偶网格", &state.showDualMesh);
                            ImGui::Checkbox("QEF 交点", &state.showQEFIntersections);
                        }
                        ImGui::EndTabItem();
                    }
                    // 形状编辑器标签
                    if (ImGui::BeginTabItem("形状编辑器")) {
                        RenderShapeEditor(&state);
                        ImGui::EndTabItem();
                    }
                    // 统计信息标签
                    if (ImGui::BeginTabItem("统计信息")) {
                        RenderInfoPanel(&state, &mesh);
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
                
                ImGui::EndChild();
            }
            
            // 上下分隔栏
            {
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                
                // 计算分隔栏区域
                ImVec2 splitterPos = ImGui::GetCursorScreenPos();
                ImGuiIO& splitterIO = ImGui::GetIO();
                
                // 分隔栏检测区域（手动计算）
                float splitterMinX = splitterPos.x;
                float splitterMaxX = splitterPos.x + leftPanelWidth;
                float splitterMinY = splitterPos.y;
                float splitterMaxY = splitterPos.y + 8.0f;
                
                ImVec2 mousePos = splitterIO.MousePos;
                bool isHovering = (mousePos.x >= splitterMinX && mousePos.x <= splitterMaxX &&
                                   mousePos.y >= splitterMinY && mousePos.y <= splitterMaxY);
                
                if (isHovering) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
                }
                
                // 处理拖动
                if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && (isHovering || verticalSplitterDragging)) {
                    verticalSplitterDragging = true;
                    // 根据鼠标位置更新上下比例
                    float newRatio = mousePos.y / totalHeight;
                    if (newRatio > 0.2f && newRatio < 0.8f) {
                        verticalSplitterRatio = newRatio;
                        leftTopHeight = totalHeight * verticalSplitterRatio;
                        leftBottomHeight = totalHeight - leftTopHeight - 8.0f;
                    }
                } else {
                    verticalSplitterDragging = false;
                }
                
                // 绘制分隔栏
                ImU32 splitterColor = (isHovering || verticalSplitterDragging)
                    ? IM_COL32(120, 120, 120, 255)
                    : IM_COL32(80, 80, 80, 255);
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                drawList->AddRectFilled(ImVec2(splitterMinX, splitterMinY), ImVec2(splitterMaxX, splitterMaxY), splitterColor);
                
                ImGui::PopStyleVar(2);
            }
            
            // 下半部分：第5个标签页 - 分布可视化路径
            {
                ImGui::BeginChild("##bottom_section", ImVec2(leftPanelWidth - 4.0f, leftBottomHeight - 4.0f), true);
                
                if (ImGui::BeginTabBar("DistributionTabs")) {
                    if (ImGui::BeginTabItem("分步可视化控制")) {
                    // 分步模式开关
                    ImGui::SeparatorText("分步可视化控制");
                    
                    if (ImGui::Checkbox("启用分步模式", &state.stepMode)) {
                        state.currentStep = 0;
                    }
                    
                    // 阶段选择
                    if (!state.stepMode) {
                        ImGui::Text("选择可视化阶段：");
                        
                        ImGui::RadioButton("仅树结构", (int*)&state.visStage, (int)VisStage::TREE_BUILD);
                        ImGui::SameLine();
                        ImGui::RadioButton("仅 QEF", (int*)&state.visStage, (int)VisStage::QEF_SOLVE);
                        
                        ImGui::RadioButton("仅网格", (int*)&state.visStage, (int)VisStage::MESH_GENERATE);
                        ImGui::SameLine();
                        ImGui::RadioButton("全部", (int*)&state.visStage, (int)VisStage::ALL);
                    }
                    
                    // 分步模式控制
                    if (state.stepMode) {
                        ImGui::Text("算法阶段进度：");
                        
                        const char* stageName = "未知";
                        switch (state.currentStep) {
                            case 0: stageName = "四叉树构建中..."; break;
                            case 1: stageName = "QEF 求解中..."; break;
                            default: stageName = "网格生成中..."; break;
                        }
                        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "%s", stageName);
                        
                        float progress = static_cast<float>(state.currentStep) / 2.0f;
                        ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f), "");
                        
                        ImGui::Text("步骤: %d / 2", state.currentStep);
                        
                        if (ImGui::ArrowButton("##prev", ImGuiDir_Left)) {
                            if (state.currentStep > 0) state.currentStep--;
                        }
                        ImGui::SameLine();
                        if (ImGui::ArrowButton("##next", ImGuiDir_Right)) {
                            if (state.currentStep < 2) state.currentStep++;
                        }
                    }
                    
                    ImGui::EndTabItem();
                }
                    ImGui::EndTabBar();
                }
                
                ImGui::EndChild();
            }
            
            ImGui::End();
        }
        
        // ============================================================================
        // 左右分隔栏
        // ============================================================================
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            
            ImVec2 splitterPos = ImGui::GetCursorScreenPos();
            ImGuiIO& splitterIO = ImGui::GetIO();
            
            // 分隔栏检测区域（垂直方向，手动计算）
            float splitterMinX = splitterPos.x;
            float splitterMaxX = splitterPos.x + 8.0f;
            float splitterMinY = splitterPos.y;
            float splitterMaxY = splitterPos.y + totalHeight;
            
            ImVec2 mousePos = splitterIO.MousePos;
            bool isHovering = (mousePos.x >= splitterMinX && mousePos.x <= splitterMaxX &&
                               mousePos.y >= splitterMinY && mousePos.y <= splitterMaxY);
            
            if (isHovering) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            }
            
            // 处理拖动
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && (isHovering || horizontalSplitterDragging)) {
                horizontalSplitterDragging = true;
                float newRatio = mousePos.x / totalWidth;
                if (newRatio > 0.15f && newRatio < 0.4f) {
                    horizontalSplitterRatio = newRatio;
                }
            } else {
                horizontalSplitterDragging = false;
            }
            
            // 绘制分隔栏
            ImU32 splitterColor = (isHovering || horizontalSplitterDragging)
                ? IM_COL32(120, 120, 120, 255)
                : IM_COL32(80, 80, 80, 255);
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(ImVec2(splitterMinX, splitterMinY), ImVec2(splitterMaxX, splitterMaxY), splitterColor);
            
            ImGui::PopStyleVar(2);
        }
        
        // 更新右侧画布尺寸
        rightCanvasWidth = totalWidth - leftPanelWidth - 8.0f;
        
        // ============================================================================
        // 右侧渲染画布
        // ============================================================================
        ImGui::SetNextWindowPos(ImVec2(leftPanelWidth + 8.0f, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(rightCanvasWidth, totalHeight), ImGuiCond_Always);
        
        {
            ImGui::Begin("渲染画布", nullptr,
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoMove);
            
            // 获取 ImDrawList 用于渲染
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 canvasTopLeft = ImGui::GetCursorScreenPos();
            
            // 计算画布中心（窗口中心）
            ImVec2 canvasCenter = ImVec2(
                canvasTopLeft.x + rightCanvasWidth * 0.5f,
                canvasTopLeft.y + totalHeight * 0.5f
            );
            
            // 缩放因子：画布单位转换为像素
            float scale = rightCanvasWidth / (canvasSize * 2.0f);
            
            // 创建渲染器
            Renderer renderer(drawList);
            
            // 坐标转换辅助函数：将世界坐标转换为屏幕坐标
            auto worldToScreen = [canvasCenter, scale](ImVec2 worldPos) -> ImVec2 {
                return ImVec2(
                    canvasCenter.x + worldPos.x * scale,
                    canvasCenter.y - worldPos.y * scale  // Y 轴翻转
                );
            };
            
            // 绘制背景网格
            ImU32 gridColor = IM_COL32(40, 40, 40, 255);
            float gridStep = 0.5f;
            for (float x = -canvasSize; x <= canvasSize; x += gridStep) {
                ImVec2 p1 = worldToScreen(ImVec2(x, -canvasSize));
                ImVec2 p2 = worldToScreen(ImVec2(x, canvasSize));
                renderer.DrawLine(p1, p2, gridColor, 0.5f);
            }
            for (float y = -canvasSize; y <= canvasSize; y += gridStep) {
                ImVec2 p1 = worldToScreen(ImVec2(-canvasSize, y));
                ImVec2 p2 = worldToScreen(ImVec2(canvasSize, y));
                renderer.DrawLine(p1, p2, gridColor, 0.5f);
            }
            
            // 绘制坐标轴
            ImU32 axisColor = IM_COL32(80, 80, 80, 255);
            ImVec2 origin = worldToScreen(ImVec2(0, 0));
            ImVec2 xAxisEnd = worldToScreen(ImVec2(canvasSize, 0));
            ImVec2 yAxisEnd = worldToScreen(ImVec2(0, canvasSize));
            renderer.DrawLine(origin, xAxisEnd, axisColor, 1.0f);
            renderer.DrawLine(origin, yAxisEnd, axisColor, 1.0f);
            
            // 获取 SDF 用于绘制等值线
            const SDFBase* sdf = GetCombinedSDF(&state);
            
            // 绘制 SDF 等值线
            if (state.showSDFContour) {
                ImU32 sdfColor = IM_COL32(255, 200, 100, 200);
                
                // 简化实现：绘制一些采样点表示 SDF
                const int samples = 50;
                float step = (canvasSize * 2.0f) / samples;
                
                for (int i = 0; i < samples; i++) {
                    for (int j = 0; j < samples; j++) {
                        float wx = -canvasSize + i * step;
                        float wy = -canvasSize + j * step;
                        float sdfVal = sdf->evaluate(ImVec2(wx, wy));
                        
                        // 绘制 SDF = 0 的点（表面附近）
                        if (sdfVal > -0.05f && sdfVal < 0.05f) {
                            ImVec2 screenPos = worldToScreen(ImVec2(wx, wy));
                            drawList->AddCircleFilled(screenPos, 1.5f, sdfColor);
                        }
                    }
                }
            }
            
            // 绘制四叉树结构
            if (state.showQuadtreeBounds && root) {
                RenderTreeStructure(root, &renderer);
            }
            
            // 绘制 QEF 求解过程
            if (state.showQEFIntersections && root) {
                RenderQFESolving(root, &renderer);
            }
            
            // 绘制对偶网格
            if (state.showDualMesh && mesh.IsValid()) {
                ImU32 meshColor = IM_COL32(100, 200, 255, 255);
                dc::RenderMesh(mesh, &renderer, meshColor, 1.5f);
            }
            
            // 绘制法线（如果在可视化选项中启用）
            // Note: 法线绘制功能待实现
            
            // 绘制形状编辑器中的形状
            for (int i = 0; i < (int)state.shapes.size(); i++) {
                const ShapeInfo& shape = state.shapes[i];
                ImVec2 screenPos = worldToScreen(shape.position);
                float screenRadius = shape.size * scale;
                
                if (shape.type == ShapeType::Circle) {
                    ImU32 shapeColor = (i == state.selectedShape) 
                        ? IM_COL32(255, 255, 100, 255) 
                        : IM_COL32(255, 255, 255, 180);
                    renderer.DrawCircle(screenPos, screenRadius, shapeColor, 32, 2.0f);
                    
                    // 绘制圆心点
                    renderer.DrawCircleFilled(screenPos, 4.0f, shapeColor);
                } else if (shape.type == ShapeType::Square) {
                    ImU32 shapeColor = (i == state.selectedShape) 
                        ? IM_COL32(255, 255, 100, 255) 
                        : IM_COL32(255, 255, 255, 180);
                    renderer.DrawQuad(screenPos, screenRadius * 2.0f, shapeColor, 0.0f, 2.0f);
                    
                    // 绘制中心点
                    renderer.DrawCircleFilled(screenPos, 4.0f, shapeColor);
                }
            }
            
            // 移动光标到画布区域外
            ImGui::SetCursorScreenPos(ImVec2(leftPanelWidth + 8.0f + rightCanvasWidth, totalHeight));
            
            ImGui::End();
        }

        // ============================================================================
        // 渲染
        // ============================================================================
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // ============================================================================
    // 清理
    // ============================================================================
    // delete root;  // 已注释：quadtree 是局部对象，析构时自动调用 deleteTree(m_root)，避免双重删除
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}