#pragma once

#include "imgui.h"
#include <vector>
#include <cmath>

// 包含 SDF 定义
#include "dc_sdf.h"

// ============================================================================
// Renderer 类 - ImDrawList 2D 渲染封装
// ============================================================================

/// @brief 基于 ImDrawList 的 2D 渲染器封装类
///
/// 该类管理 ImDrawList 指针，提供基本的 2D 图形绘制功能。
/// 渲染必须在 ImGui 的 NewFrame() 和 EndFrame() 之间调用。
///
/// 使用方法:
/// @code
/// // 在 ImGui 窗口内获取 ImDrawList 并传递给渲染器
/// ImDrawList* draw_list = ImGui::GetWindowDrawList();
/// Renderer renderer(draw_list);
///
/// // 绘制基本图形
/// renderer.DrawCircle(ImVec2(400, 300), 50, IM_COL32(255, 0, 0, 255));
/// renderer.DrawLine(ImVec2(0, 0), ImVec2(100, 100), IM_COL32(0, 255, 0, 255));
/// @endcode
class Renderer
{
public:
    // ========================================================================
    // 构造与析构
    // ========================================================================

    /// @brief 构造函数
    /// @param draw_list ImDrawList 指针，通常通过 ImGui::GetWindowDrawList() 获取
    explicit Renderer(ImDrawList* draw_list) : m_draw_list(draw_list) {}

    /// @brief 默认构造函数（创建空渲染器）
    Renderer() : m_draw_list(nullptr) {}

    /// @brief 析构函数
    ~Renderer() = default;

    // ========================================================================
    // 帧管理
    // ========================================================================

    /// @brief 开始帧 - 在绘制前调用
    ///
    /// ImDrawList 不需要显式的 BeginFrame，但此方法可用于：
    /// - 验证 draw_list 是否有效
    /// - 执行渲染前的准备工作
    void BeginFrame()
    {
        // ImDrawList 使用即时模式，无需显式开始帧
        // 但可以在这里添加调试检查或状态重置
    }

    /// @brief 结束帧 - 在绘制完成后调用
    ///
    /// ImDrawList 不需要显式的 EndFrame，但此方法可用于：
    /// - 执行渲染后的清理工作
    /// - 验证渲染完整性
    void EndFrame()
    {
        // ImDrawList 使用即时模式，无需显式结束帧
    }

    // ========================================================================
    // 基础绘制方法
    // ========================================================================

    /// @brief 设置 ImDrawList 指针
    /// @param draw_list ImDrawList 指针
    void SetDrawList(ImDrawList* draw_list)
    {
        m_draw_list = draw_list;
    }

    /// @brief 获取当前 ImDrawList 指针
    /// @return ImDrawList 指针
    ImDrawList* GetDrawList() const
    {
        return m_draw_list;
    }

    // ========================================================================
    // 基本图元绘制
    // ========================================================================

    /// @brief 绘制点
    /// @param pos 点的位置（屏幕坐标）
    /// @param color 颜色（使用 IM_COL32(r,g,b,a) 宏）
    ///
    /// 注意：ImDrawList 没有直接的 AddPoint 方法，
    /// 这里使用一个像素的线段来模拟点
    void DrawPoint(ImVec2 pos, ImU32 color)
    {
        if (!m_draw_list)
            return;

        // 使用零长度的线段来绘制点
        // thickness 参数控制点的大小
        m_draw_list->AddLine(pos, ImVec2(pos.x + 1.0f, pos.y + 1.0f), color, 2.0f);
    }

    /// @brief 绘制线段
    /// @param p1 起点位置
    /// @param p2 终点位置
    /// @param color 颜色
    /// @param thickness 线条粗细（默认 1.0 像素）
    ///
    /// ImDrawList::AddLine 会绘制一条从 p1 到 p2 的线段。
    /// 线条使用抗锯齿渲染。
    void DrawLine(ImVec2 p1, ImVec2 p2, ImU32 color, float thickness = 1.0f)
    {
        if (!m_draw_list)
            return;

        // AddLine 签名：
        // void AddLine(const ImVec2& p1, const ImVec2& p2, ImU32 col, float thickness = 1.0f);
        m_draw_list->AddLine(p1, p2, color, thickness);
    }

    /// @brief 绘制矩形/四边形
    /// @param center 中心位置
    /// @param size 正方形的边长
    /// @param color 颜色
    /// @param rotation 旋转角度（弧度，默认 0）
    /// @param thickness 边框粗细（默认 1.0）
    ///
    /// 该方法绘制一个以 center 为中心、边长为 size 的正方形。
    /// 可选旋转角度绕中心点旋转。
    void DrawQuad(ImVec2 center, float size, ImU32 color, float rotation = 0.0f, float thickness = 1.0f)
    {
        if (!m_draw_list)
            return;

        // 计算正方形四个角的位置
        // 假设中心为 (cx, cy)，边长为 size
        float half_size = size * 0.5f;

        // 四个角相对于中心的偏移（未旋转）
        ImVec2 corners[4] = {
            ImVec2(-half_size, -half_size),  // 左上
            ImVec2( half_size, -half_size),  // 右上
            ImVec2( half_size,  half_size),  // 右下
            ImVec2(-half_size,  half_size)   // 左下
        };

        // 如果有旋转，应用旋转矩阵
        if (rotation != 0.0f)
        {
            float cos_r = std::cos(rotation);
            float sin_r = std::sin(rotation);

            for (int i = 0; i < 4; i++)
            {
                float x = corners[i].x * cos_r - corners[i].y * sin_r;
                float y = corners[i].x * sin_r + corners[i].y * cos_r;
                corners[i] = ImVec2(x, y);
            }
        }

        // 添加中心偏移
        for (int i = 0; i < 4; i++)
        {
            corners[i].x += center.x;
            corners[i].y += center.y;
        }

        // AddPolyline 绘制多个连接的线段
        // 或者使用 AddQuad（如果可用）
        // 这里使用 AddPolyline 来绘制四个边的多段线
        m_draw_list->AddPolyline(corners, 4, color, ImDrawFlags_Closed, thickness);
    }

    /// @brief 绘制圆（轮廓）
    /// @param center 圆心位置
    /// @param radius 半径
    /// @param color 颜色
    /// @param segments 圆滑段数（默认 32，0 表示自动细分）
    /// @param thickness 边框粗细（默认 1.0）
    ///
    /// 使用 ImDrawList::AddCircle 绘制圆。
    /// segments 参数控制圆的多边形近似程度：
    /// - 12: 粗糙的十二边形
    /// - 32: 平滑的圆（推荐）
    /// - 0:  自动细分（根据圆的大小和质量设置）
    void DrawCircle(ImVec2 center, float radius, ImU32 color, int segments = 32, float thickness = 1.0f)
    {
        if (!m_draw_list)
            return;

        // AddCircle 签名：
        // void AddCircle(const ImVec2& center, float radius, ImU32 col,
        //                int num_segments = 0, float thickness = 1.0f);
        // 当 num_segments = 0 时，ImGui 会根据 CircleSegmentMaxError 自动细分
        m_draw_list->AddCircle(center, radius, color, segments, thickness);
    }

    /// @brief 绘制填充圆
    /// @param center 圆心位置
    /// @param radius 半径
    /// @param color 颜色
    /// @param segments 圆滑段数（默认 32，0 表示自动细分）
    void DrawCircleFilled(ImVec2 center, float radius, ImU32 color, int segments = 32)
    {
        if (!m_draw_list)
            return;

        // AddCircleFilled 签名：
        // void AddCircleFilled(const ImVec2& center, float radius, ImU32 col,
        //                      int num_segments = 0);
        m_draw_list->AddCircleFilled(center, radius, color, segments);
    }

    // ========================================================================
    // SDF 等值线绘制
    // ========================================================================

    /// @brief 绘制 SDF 等值线（轮廓）
    /// @param sdf 指向 SDF 基类的指针
    /// @param isoValue 等值线的值（通常为 0.0，表示表面）
    /// @param color 颜色
    /// @param bounds 搜索区域的边界
    /// @param resolution 采样分辨率
    ///
    /// 使用 Marching Squares 算法绘制 SDF 的等值线。
    /// 该方法在指定边界内采样 SDF，并在等值点之间连线。
    void DrawSDFContour(const SDFBase* sdf, float isoValue, ImU32 color,
                        const ImVec4& bounds = ImVec4(0.0f, 0.0f, 800.0f, 600.0f),
                        int resolution = 100);

    /// @brief 绘制 SDF 填充等值区
    /// @param sdf 指向 SDF 基类的指针
    /// @param isoValue 等值线的值
    /// @param insideColor 内部颜色（SDF < isoValue 的区域）
    /// @param outsideColor 外部颜色（SDF > isoValue 的区域）
    /// @param bounds 搜索区域边界
    /// @param resolution 采样分辨率
    void DrawSDFContourFilled(const SDFBase* sdf, float isoValue,
                               ImU32 insideColor, ImU32 outsideColor,
                               const ImVec4& bounds = ImVec4(0.0f, 0.0f, 800.0f, 600.0f),
                               int resolution = 50);

private:
    /// @brief 计算 SDF 在给定点的值
    /// @param sdf SDF 指针
    /// @param p 查询点
    /// @return SDF 值（负值表示内部，正值表示外部）
    float SampleSDF(const SDFBase* sdf, ImVec2 p) const;

    /// @brief Marching Squares 查表索引计算
    /// @param values 四个角点的 SDF 值数组
    /// @param isoValue 等值线值
    /// @return 查表索引 (0-15)
    int GetMarchingSquaresIndex(const float values[4], float isoValue) const;

    /// @brief 在两点之间插值找到等值线交点
    /// @param p1 点1
    /// @param p2 点2
    /// @param v1 点1的 SDF 值
    /// @param v2 点2的 SDF 值
    /// @param isoValue 等值线值
    /// @return 交点位置
    ImVec2 Interpolate(ImVec2 p1, ImVec2 p2, float v1, float v2, float isoValue) const;

    /// ImDrawList 指针（由 ImGui 管理）
    ImDrawList* m_draw_list = nullptr;
};