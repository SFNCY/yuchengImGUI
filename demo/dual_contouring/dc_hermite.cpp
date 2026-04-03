#include "dc_hermite.h"

#include <cmath>

namespace dc {

//=============================================================================
// 二分法边交点查找
//=============================================================================
HermiteData BinarySearchEdgeIntersection(const ImVec2& edgeStart, const ImVec2& edgeEnd, const SDFBase* sdf, int iterations)
{
    // 采样边两端点的 SDF 值
    float sdfStart = sdf->evaluate(edgeStart);
    float sdfEnd = sdf->evaluate(edgeEnd);

    // 如果两端点符号相同（同正或同负），边上没有表面穿越
    // 注意：允许非常接近零的情况也算作无交点（数值稳定性）
    if (sdfStart * sdfEnd > 0.0f && std::abs(sdfStart) > 1e-6f && std::abs(sdfEnd) > 1e-6f) {
        return HermiteData(); // 返回空的 HermiteData
    }

    // 如果一端恰好在表面上，直接使用该点
    if (std::abs(sdfStart) < 1e-6f) {
        ImVec2 normal = ComputeNormal(edgeStart, sdf);
        return HermiteData(edgeStart, normal);
    }
    if (std::abs(sdfEnd) < 1e-6f) {
        ImVec2 normal = ComputeNormal(edgeEnd, sdf);
        return HermiteData(edgeEnd, normal);
    }

    // 二分法迭代 - 使用可修改的本地变量
    ImVec2 searchStart = edgeStart;
    ImVec2 searchEnd = edgeEnd;
    float sdfSearchStart = sdfStart;
    float sdfSearchEnd = sdfEnd;
    ImVec2 mid = edgeStart;

    for (int i = 0; i < iterations; i++) {
        // 计算中点
        mid = ImVec2((searchStart.x + searchEnd.x) * 0.5f, (searchStart.y + searchEnd.y) * 0.5f);
        float sdfMid = sdf->evaluate(mid);

        // 如果中点恰好在表面上，停止搜索
        if (std::abs(sdfMid) < 1e-6f) {
            break;
        }

        // 判断哪一半有符号变化
        // 如果 sdfSearchStart 和 sdfMid 符号不同，说明零点在左边
        if (sdfSearchStart * sdfMid < 0.0f) {
            // 零点在 [searchStart, mid] 之间
            searchEnd = mid;
            sdfSearchEnd = sdfMid;
        } else {
            // 零点在 [mid, searchEnd] 之间
            searchStart = mid;
            sdfSearchStart = sdfMid;
        }
    }

    // 计算交点法线
    ImVec2 normal = ComputeNormal(mid, sdf);
    return HermiteData(mid, normal);
}

//=============================================================================
// 法线计算（有限差分法）
//=============================================================================
ImVec2 ComputeNormal(const ImVec2& point, const SDFBase* sdf, float epsilon)
{
    // 使用中心差分法计算梯度
    // ∂SDF/∂x ≈ (SDF(x+ε,y) - SDF(x-ε,y)) / (2ε)
    // ∂SDF/∂y ≈ (SDF(x,y+ε) - SDF(x,y-ε)) / (2ε)

    float sdfP = sdf->evaluate(point);

    float dfdx = (sdf->evaluate(ImVec2(point.x + epsilon, point.y)) -
                  sdf->evaluate(ImVec2(point.x - epsilon, point.y))) / (2.0f * epsilon);

    float dfdy = (sdf->evaluate(ImVec2(point.x, point.y + epsilon)) -
                  sdf->evaluate(ImVec2(point.x, point.y - epsilon))) / (2.0f * epsilon);

    // 梯度向量
    ImVec2 gradient(dfdx, dfdy);

    // 归一化得到法线
    float len = std::hypot(gradient.x, gradient.y);
    if (len > 1e-6f) {
        return ImVec2(gradient.x / len, gradient.y / len);
    }

    // 如果梯度为零（不应该发生），返回默认法线
    return ImVec2(0.0f, 1.0f);
}

//=============================================================================
// 检查边是否有符号变化
//=============================================================================
bool EdgeHasSignChange(const ImVec2& start, const ImVec2& end, const SDFBase* sdf)
{
    float sdfStart = sdf->evaluate(start);
    float sdfEnd = sdf->evaluate(end);

    // 如果两端点 SDF 值符号不同，说明有表面穿越
    // 使用 > 0 判断而不是 >=，以处理恰好为零的情况
    return sdfStart * sdfEnd < 0.0f;
}

//=============================================================================
// 查找节点所有边的交点
//=============================================================================
EdgeIntersections FindEdgeIntersections(QuadtreeNode* node, const SDFBase* sdf)
{
    EdgeIntersections result;

    // 获取节点边界
    ImVec2 minPt = node->bounds.min();
    ImVec2 maxPt = node->bounds.max();

    // 定义四条边的起点和终点
    // 底边: (minX, minY) -> (maxX, minY)
    ImVec2 bottomStart = minPt;
    ImVec2 bottomEnd = ImVec2(maxPt.x, minPt.y);

    // 顶边: (minX, maxY) -> (maxX, maxY)
    ImVec2 topStart = ImVec2(minPt.x, maxPt.y);
    ImVec2 topEnd = maxPt;

    // 左边: (minX, minY) -> (minX, maxY)
    ImVec2 leftStart = minPt;
    ImVec2 leftEnd = ImVec2(minPt.x, maxPt.y);

    // 右边: (maxX, minY) -> (maxX, maxY)
    ImVec2 rightStart = ImVec2(maxPt.x, minPt.y);
    ImVec2 rightEnd = ImVec2(maxPt.x, maxPt.y);

    // 底边交点
    if (EdgeHasSignChange(bottomStart, bottomEnd, sdf)) {
        HermiteData hd = BinarySearchEdgeIntersection(bottomStart, bottomEnd, sdf);
        if (std::hypot(hd.normal.x, hd.normal.y) > 1e-6f) { // 确保是有效的交点
            result.bottom.push_back(hd);
        }
    }

    // 顶边交点
    if (EdgeHasSignChange(topStart, topEnd, sdf)) {
        HermiteData hd = BinarySearchEdgeIntersection(topStart, topEnd, sdf);
        if (std::hypot(hd.normal.x, hd.normal.y) > 1e-6f) {
            result.top.push_back(hd);
        }
    }

    // 左边交点
    if (EdgeHasSignChange(leftStart, leftEnd, sdf)) {
        HermiteData hd = BinarySearchEdgeIntersection(leftStart, leftEnd, sdf);
        if (std::hypot(hd.normal.x, hd.normal.y) > 1e-6f) {
            result.left.push_back(hd);
        }
    }

    // 右边交点
    if (EdgeHasSignChange(rightStart, rightEnd, sdf)) {
        HermiteData hd = BinarySearchEdgeIntersection(rightStart, rightEnd, sdf);
        if (std::hypot(hd.normal.x, hd.normal.y) > 1e-6f) {
            result.right.push_back(hd);
        }
    }

    return result;
}

//=============================================================================
// 递归收集所有 Hermite 数据
//=============================================================================
void CollectAllHermiteData(QuadtreeNode* node, const SDFBase* sdf, std::vector<HermiteData>& output)
{
    if (node == nullptr) {
        return;
    }

    // 查找当前节点的边交点
    EdgeIntersections intersections = FindEdgeIntersections(node, sdf);

    // 添加当前节点的所有交点到输出
    for (const HermiteData& hd : intersections.bottom) {
        output.push_back(hd);
    }
    for (const HermiteData& hd : intersections.top) {
        output.push_back(hd);
    }
    for (const HermiteData& hd : intersections.left) {
        output.push_back(hd);
    }
    for (const HermiteData& hd : intersections.right) {
        output.push_back(hd);
    }

    // 如果节点已细分，递归处理子节点
    if (node->subdivided) {
        CollectAllHermiteData(node->nw, sdf, output);
        CollectAllHermiteData(node->ne, sdf, output);
        CollectAllHermiteData(node->sw, sdf, output);
        CollectAllHermiteData(node->se, sdf, output);
    }
}

} // namespace dc
