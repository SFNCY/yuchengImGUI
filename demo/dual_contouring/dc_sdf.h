#pragma once

#include "imgui.h"

#include <cmath>
#include <algorithm>

//================================================================================
// 2D 符号距离函数 (SDF) 基础形状实现
// 参考: Inigo Quilez - https://iquilezles.org/articles/distfunctions2d/
//
// SDF 核心概念：
// - 返回值为点到形状表面的最短距离
// - 返回值为负：点在形状内部
// - 返回值为正：点在形状外部
// - 返回值为零：点恰好在形状表面
//================================================================================

//--------------------------------------------------------------------------------
// SDF 基类 - 所有 SDF 形状的抽象基类
//--------------------------------------------------------------------------------
struct SDFBase
{
    virtual ~SDFBase() = default;

    // 计算给定点的 SDF 值
    // point: 要计算的2D点（使用 ImVec2 格式）
    // 返回值: 点到表面的距离（负=内部，正=外部，零=表面）
    virtual float evaluate(const ImVec2& point) const = 0;
};

//--------------------------------------------------------------------------------
// SDFCircle - 圆形 SDF
//
// 公式: distance = |point - center| - radius
//
// 工作原理：
// 1. 计算点到圆心的欧几里得距离
// 2. 减去半径得到有符号距离
// 3. 结果 < 0 表示点在圆内，结果 > 0 表示点在圆外
//--------------------------------------------------------------------------------
struct SDFCircle : public SDFBase
{
    ImVec2  center;   // 圆心坐标
    float radius;   // 圆的半径

    SDFCircle() : center(0.0f, 0.0f), radius(0.0f) {}
    SDFCircle(float cx, float cy, float r) : center(cx, cy), radius(r) {}

    // 评估函数：计算点到圆周的距离
    // 内部优化：使用 squared length 避免开方运算
    virtual float evaluate(const ImVec2& point) const override
    {
        // 步骤1：计算点到圆心的向量
        ImVec2 diff(point.x - center.x, point.y - center.y);

        // 步骤2：计算向量长度并减去半径
        // 使用 hypot 计算欧几里得距离 |diff|
        // 减法操作将0距离点移动到圆周上
        return std::hypot(diff.x, diff.y) - radius;
    }
};

//--------------------------------------------------------------------------------
// SDFSquare - 方形 SDF (轴对齐正方形)
//
// 公式: distance = max(|dx|, |dy|) - halfSize
//
// 工作原理：
// 1. 计算点到方形中心在x和y方向上的偏移
// 2. 取绝对值得到距离（消除正负方向差异）
// 3. 取max找到最远方向的距离（这决定了到边界的最短距离）
// 4. 减去halfSize将0距离点移动到方形边缘
//--------------------------------------------------------------------------------
struct SDFSquare : public SDFBase
{
    ImVec2  center;    // 方形中心坐标
    float halfSize;  // 半边长（总边长 = 2 * halfSize）

    SDFSquare() : center(0.0f, 0.0f), halfSize(0.0f) {}
    SDFSquare(float cx, float cy, float s) : center(cx, cy), halfSize(s * 0.5f) {}

    // 评估函数：计算点到方形边缘的距离
    virtual float evaluate(const ImVec2& point) const override
    {
        // 步骤1：计算点到中心的偏移
        ImVec2 diff(point.x - center.x, point.y - center.y);

        // 步骤2：计算偏移分量的绝对值
        // fabsf() 处理四个象限的对称性
        float abs_dx = std::fabsf(diff.x);
        float abs_dy = std::fabsf(diff.y);

        // 步骤3：取最大值得到切比雪夫距离（L∞范数）
        // 这是到方形边界最近距离的关键
        // 如果点在角上，距离 = max(|dx|, |dy|)
        float dist = std::fmax(abs_dx, abs_dy);

        // 步骤4：减去半边长，偏移到边界
        return dist - halfSize;
    }
};

//--------------------------------------------------------------------------------
// SDFUnion - SDF 联合组合
//
// 公式: distance = min(sdf1, sdf2)
//
// 工作原理：
// 1. 联合表示两个形状的集合
// 2. 联合的SDF值 = 两个SDF中的较小者（更近的那个）
// 3. 这确保了结果的符号正确性和距离连续性
//--------------------------------------------------------------------------------
struct SDFUnion : public SDFBase
{
    const SDFBase* sdf1;  // 第一个 SDF
    const SDFBase* sdf2;  // 第二个 SDF

    SDFUnion() : sdf1(nullptr), sdf2(nullptr) {}
    SDFUnion(const SDFBase* a, const SDFBase* b) : sdf1(a), sdf2(b) {}

    // 评估函数：返回两个SDF中的最小值
    // 这实现了形状的"并集"操作
    virtual float evaluate(const ImVec2& point) const override
    {
        // 取两个SDF值的较小者
        // 较小者表示点更接近的形状边界
        float d1 = sdf1->evaluate(point);
        float d2 = sdf2->evaluate(point);
        return std::fmin(d1, d2);
    }
};

//--------------------------------------------------------------------------------
// SampleSDF - 计算点到 SDF 表面的距离（采样函数）
//
// 这是 SDF 的主要入口函数，用于计算任意点到形状表面的距离
//
// 参数:
//   point: 要计算的2D点坐标
//   sdf: 指向 SDF 基类的指针，支持多态调用
//
// 返回值:
//   float: 有符号距离值
//     < 0: 点在形状内部
//     > 0: 点在形状外部
//     = 0: 点在形状表面
//
// 使用示例:
//   SDFCircle circle(0.0f, 0.0f, 1.0f);
//   float d = SampleSDF(vec2(1.0f, 0.0f), &circle);  // 返回 ~0.0（点在圆上）
//--------------------------------------------------------------------------------
inline float SampleSDF(const ImVec2& point, const SDFBase* sdf)
{
    return sdf->evaluate(point);
}

//--------------------------------------------------------------------------------
// EvaluateSDF - 计算给定点的 SDF 值（别名函数）
//
// 这是 SampleSDF 的别名，提供更语义化的命名
// 两者功能完全相同，选择使用取决于个人偏好
//
// 参数:
//   point: 要计算的2D点坐标
//   sdf: 指向 SDF 基类的指针
//
// 返回值: 与 SampleSDF 相同的有符号距离值
//--------------------------------------------------------------------------------
inline float EvaluateSDF(const ImVec2& point, const SDFBase* sdf)
{
    return sdf->evaluate(point);
}