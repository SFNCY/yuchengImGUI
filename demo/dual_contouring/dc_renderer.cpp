#include "dc_renderer.h"
#include "dc_sdf.h"

// ============================================================================
// Marching Squares 边缘表
// ============================================================================

/// @brief Marching Squares 查找表
///
/// 每个单元格有四个角点，每个角点有两种状态（inside/outside 等值面）
/// 总共有 2^4 = 16 种状态
///
/// 边缘连接方式：
///   2 ---- 3
///   |      |
///   |      |
///   0 ---- 1
///
/// 索引含义：
///   bit 0: 角点 0 (左下) > isoValue
///   bit 1: 角点 1 (右下) > isoValue
///   bit 2: 角点 2 (左上) > isoValue
///   bit 3: 角点 3 (右上) > isoValue
///
/// 每种索引指定哪些边缘应该有线段穿过
static const int MS_EDGE_TABLE[16] = {
    // 0000: 全部在外，无边缘
    0,
    // 0001: 仅左下在内
    0x5,   // 0101: 边缘 0-2 和 0-1
    // 0010: 仅右下在内
    0x9,   // 1001: 边缘 0-1 和 1-3
    // 0011: 下边在内
    0x1,   // 0001: 边缘 0-1
    // 0100: 仅左上在内
    0x6,   // 0110: 边缘 0-2 和 2-3
    // 0101: 左右对称（垂直边缘）
    0x5,   // 0101: 边缘 0-2 和 0-1
    // 0110: 上边在内
    0x2,   // 0010: 边缘 2-3
    // 0111: 底部三个角在内
    0x9,   // 1001: 边缘 0-1 和 1-3
    // 1000: 仅右上在内
    0xA,   // 1010: 边缘 1-3 和 2-3
    // 1001: 右上和左下（对角线）
    0x5,   // 0101: 边缘 0-2 和 0-1（简化为鞍形）
    // 1010: 左右对称（水平边缘）
    0xA,   // 1010: 边缘 1-3 和 2-3
    // 1011: 左上、右上、右下
    0x6,   // 0110: 边缘 0-2 和 2-3
    // 1100: 左边在内
    0x2,   // 0010: 边缘 2-3
    // 1101: 左上、左下、右下
    0x9,   // 1001: 边缘 0-1 和 1-3
    // 1110: 上边在内
    0x1,   // 0001: 边缘 0-1
    // 1111: 全部在内，无边缘
    0
};

// ============================================================================
// Renderer 实现
// ============================================================================

float Renderer::SampleSDF(const SDFBase* sdf, ImVec2 p) const
{
    if (!sdf)
        return 0.0f;
    // 使用 dc_sdf.h 的 evaluate 方法
    return sdf->evaluate(ImVec2(p.x, p.y));
}

int Renderer::GetMarchingSquaresIndex(const float values[4], float isoValue) const
{
    // 计算每个角点的状态（在外=1，在内=0）
    // 注意：这里假设 SDF 值 < 0 表示在形状内部
    int index = 0;
    if (values[0] > isoValue) index |= 1;  // 左下
    if (values[1] > isoValue) index |= 2;  // 右下
    if (values[2] > isoValue) index |= 4;  // 左上
    if (values[3] > isoValue) index |= 8;  // 右上
    return index;
}

ImVec2 Renderer::Interpolate(ImVec2 p1, ImVec2 p2, float v1, float v2, float isoValue) const
{
    // 线性插值找到等值线穿过的边缘位置
    // 如果 v1 和 v2 非常接近，返回中点以避免除零
    float diff = v2 - v1;
    if (std::abs(diff) < 1e-6f)
        return ImVec2((p1.x + p2.x) * 0.5f, (p1.y + p2.y) * 0.5f);

    float t = (isoValue - v1) / diff;
    // 限制 t 在 [0, 1] 范围内
    t = std::max(0.0f, std::min(1.0f, t));

    return ImVec2(
        p1.x + t * (p2.x - p1.x),
        p1.y + t * (p2.y - p1.y)
    );
}

void Renderer::DrawSDFContour(const SDFBase* sdf, float isoValue, ImU32 color,
                              const ImVec4& bounds, int resolution)
{
    if (!m_draw_list || !sdf || resolution <= 0)
        return;

    float minX = bounds.x;
    float minY = bounds.y;
    float maxX = bounds.z;
    float maxY = bounds.w;

    float stepX = (maxX - minX) / static_cast<float>(resolution);
    float stepY = (maxY - minY) / static_cast<float>(resolution);

    // 存储每个单元格的四个角点的 SDF 值
    float values[4];

    // 遍历网格的每个单元格
    for (int j = 0; j < resolution; j++)
    {
        for (int i = 0; i < resolution; i++)
        {
            // 计算当前单元格四个角点的坐标
            ImVec2 p0 = ImVec2(minX + i * stepX,     minY + j * stepY);     // 左下
            ImVec2 p1 = ImVec2(minX + (i + 1) * stepX, minY + j * stepY);   // 右下
            ImVec2 p2 = ImVec2(minX + i * stepX,     minY + (j + 1) * stepY); // 左上
            ImVec2 p3 = ImVec2(minX + (i + 1) * stepX, minY + (j + 1) * stepY); // 右上

            // 计算四个角点的 SDF 值
            values[0] = SampleSDF(sdf, p0);
            values[1] = SampleSDF(sdf, p1);
            values[2] = SampleSDF(sdf, p2);
            values[3] = SampleSDF(sdf, p3);

            // 获取 Marching Squares 索引
            int index = GetMarchingSquaresIndex(values, isoValue);

            // 查找表值为 0 表示没有边缘穿过这个单元格
            if (MS_EDGE_TABLE[index] == 0)
                continue;

            // 计算等值线交点
            // 边缘顺序：0=下边(0-1), 1=右边(1-3), 2=上边(2-3), 3=左边(0-2)
            ImVec2 edges[4];
            edges[0] = Interpolate(p0, p1, values[0], values[1], isoValue); // 下边
            edges[1] = Interpolate(p1, p3, values[1], values[3], isoValue); // 右边
            edges[2] = Interpolate(p2, p3, values[2], values[3], isoValue); // 上边
            edges[3] = Interpolate(p0, p2, values[0], values[2], isoValue); // 左边

            // 根据索引绘制边缘
            int edge_mask = MS_EDGE_TABLE[index];

            // 下边 (0-1)
            if (edge_mask & 0x1)
            {
                m_draw_list->AddLine(edges[0], edges[1], color);
            }
            // 右边 (1-3)
            if (edge_mask & 0x2)
            {
                m_draw_list->AddLine(edges[1], edges[3], color);
            }
            // 上边 (2-3)
            if (edge_mask & 0x4)
            {
                m_draw_list->AddLine(edges[2], edges[3], color);
            }
            // 左边 (0-2)
            if (edge_mask & 0x8)
            {
                m_draw_list->AddLine(edges[3], edges[0], color);
            }
        }
    }
}

void Renderer::DrawSDFContourFilled(const SDFBase* sdf, float isoValue,
                                    ImU32 insideColor, ImU32 outsideColor,
                                    const ImVec4& bounds, int resolution)
{
    if (!m_draw_list || !sdf || resolution <= 0)
        return;

    float minX = bounds.x;
    float minY = bounds.y;
    float maxX = bounds.z;
    float maxY = bounds.w;

    float stepX = (maxX - minX) / static_cast<float>(resolution);
    float stepY = (maxY - minY) / static_cast<float>(resolution);

    // 临时存储多边形顶点（每个单元格最多 4 个顶点）
    ImVec2 polygon[4];
    int polygonCount = 0;

    // 遍历网格的每个单元格
    for (int j = 0; j < resolution; j++)
    {
        for (int i = 0; i < resolution; i++)
        {
            // 计算当前单元格四个角点的坐标
            ImVec2 p0 = ImVec2(minX + i * stepX,     minY + j * stepY);     // 左下
            ImVec2 p1 = ImVec2(minX + (i + 1) * stepX, minY + j * stepY);   // 右下
            ImVec2 p2 = ImVec2(minX + i * stepX,     minY + (j + 1) * stepY); // 左上
            ImVec2 p3 = ImVec2(minX + (i + 1) * stepX, minY + (j + 1) * stepY); // 右上

            // 计算四个角点的 SDF 值
            float values[4];
            values[0] = SampleSDF(sdf, p0);
            values[1] = SampleSDF(sdf, p1);
            values[2] = SampleSDF(sdf, p2);
            values[3] = SampleSDF(sdf, p3);

            // 获取 Marching Squares 索引
            int index = GetMarchingSquaresIndex(values, isoValue);

            // 根据索引确定单元格的类型并构建多边形
            polygonCount = 0;

            // 使用简化的方法：对于常见的模式构建填充多边形
            switch (index)
            {
            case 0: // 0000: 全部在外
            case 15: // 1111: 全部在内
                // 这些情况不绘制或绘制整个单元格
                if (index == 15)
                {
                    // 全部在内 - 绘制整个单元格
                    polygon[0] = p0;
                    polygon[1] = p1;
                    polygon[2] = p3;
                    polygon[3] = p2;
                    polygonCount = 4;
                }
                break;

            case 1: // 0001: 仅左下在外
            case 14: // 1110: 仅左下在内
                polygon[0] = Interpolate(p0, p1, values[0], values[1], isoValue);
                polygon[1] = Interpolate(p0, p2, values[0], values[2], isoValue);
                polygon[2] = p0;
                polygonCount = 3;
                break;

            case 2: // 0010: 仅右下在外
            case 13: // 1101: 仅右下在内
                polygon[0] = p1;
                polygon[1] = Interpolate(p1, p0, values[1], values[0], isoValue);
                polygon[2] = Interpolate(p1, p3, values[1], values[3], isoValue);
                polygonCount = 3;
                break;

            case 4: // 0100: 仅左上在外
            case 11: // 1011: 仅左上在内
                polygon[0] = p2;
                polygon[1] = Interpolate(p2, p3, values[2], values[3], isoValue);
                polygon[2] = Interpolate(p2, p0, values[2], values[0], isoValue);
                polygonCount = 3;
                break;

            case 8: // 1000: 仅右上在外
            case 7: // 0111: 仅右上在内
                polygon[0] = Interpolate(p3, p2, values[3], values[2], isoValue);
                polygon[1] = p3;
                polygon[2] = Interpolate(p3, p1, values[3], values[1], isoValue);
                polygonCount = 3;
                break;

            case 5: // 0101: 左下和右上在外（对角线情况）
            case 10: // 1010: 左上和右下在外（对角线情况）
                // 鞍形情况 - 绘制两个三角形
                // 简化为使用对角线分割
                polygon[0] = Interpolate(p0, p1, values[0], values[1], isoValue);
                polygon[1] = p1;
                polygon[2] = Interpolate(p1, p3, values[1], values[3], isoValue);
                polygon[3] = Interpolate(p3, p2, values[3], values[2], isoValue);
                polygon[4] = p2;
                polygon[5] = Interpolate(p2, p0, values[2], values[0], isoValue);
                // 使用 AddPolyline 绘制填充
                if (index == 5)
                {
                    // 绘制凸包
                    m_draw_list->AddPolyline(polygon, 6, insideColor, ImDrawFlags_Closed, 1.0f);
                }
                else
                {
                    m_draw_list->AddPolyline(polygon, 6, outsideColor, ImDrawFlags_Closed, 1.0f);
                }
                continue; // 已经绘制，继续下一个单元格

            default:
                // 其他情况（3, 6, 9, 12）：使用简化方法
                // 计算平均颜色作为近似
                float avgValue = (values[0] + values[1] + values[2] + values[3]) * 0.25f;
                ImU32 fillColor = (avgValue < isoValue) ? insideColor : outsideColor;

                // 绘制整个单元格作为近似
                polygon[0] = p0;
                polygon[1] = p1;
                polygon[2] = p3;
                polygon[3] = p2;
                polygonCount = 4;
                break;
            }

            // 绘制多边形
            if (polygonCount >= 3)
            {
                ImU32 fillColor = insideColor;
                m_draw_list->AddPolyline(polygon, polygonCount, fillColor, ImDrawFlags_Closed, 1.0f);
            }
        }
    }
}