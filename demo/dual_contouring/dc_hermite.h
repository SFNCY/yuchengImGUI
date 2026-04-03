#pragma once

#include "dc_sdf.h"
#include "dc_quadtree.h"

#include <vector>

namespace dc {

//=============================================================================
// Hermite 数据结构
//=============================================================================
// Hermite 数据存储表面交点的几何信息：
// - position: 交点在二维空间中的位置
// - normal: 表面在交点处的法向量（指向 SDF 增长的方向，即指向形状外部）
//
// Hermite 数据的作用：
// 在双轮廓线（Dual Contouring）中，我们需要知道表面的精确位置和方向，
// 以便在四叉树节点内部定位轮廓线顶点。
//=============================================================================
struct HermiteData {
    ImVec2 position; // 交点位置
    ImVec2 normal;   // 表面法线（指向外部）

    HermiteData() : position(0.0f, 0.0f), normal(0.0f, 0.0f) {}
    HermiteData(const ImVec2& pos, const ImVec2& norm) : position(pos), normal(norm) {}
};

//=============================================================================
// 边交点数组
// 存储四叉树节点四条边的交点数据：
// - bottom: 底边交点 (minX, minY) -> (maxX, minY)
// - top: 顶边交点 (minX, maxY) -> (maxX, maxY)
// - left: 左边交点 (minX, minY) -> (minX, maxY)
// - right: 右边交点 (maxX, minY) -> (maxX, maxY)
//
// 如果某条边没有交点，则对应数组为空
//=============================================================================
struct EdgeIntersections {
    std::vector<HermiteData> bottom; // 底边交点
    std::vector<HermiteData> top;     // 顶边交点
    std::vector<HermiteData> left;    // 左边交点
    std::vector<HermiteData> right;   // 右边交点

    // 检查是否没有交点
    bool isEmpty() const {
        return bottom.empty() && top.empty() && left.empty() && right.empty();
    }

    // 获取总交点数量
    int totalCount() const {
        return static_cast<int>(bottom.size() + top.size() + left.size() + right.size());
    }
};

//=============================================================================
// 边交点查找函数
// 使用二分法在边上寻找 SDF 等值面（零等值面）的交点
//=============================================================================

// 在一条边上使用二分法寻找 SDF 零点（表面交点）
// 参数：
//   edgeStart: 边起点坐标
//   edgeEnd: 边终点坐标
//   sdf: SDF 函数
//   iterations: 二分法迭代次数（默认 8 次，精度足够）
// 返回值：
//   交点位置（vec2），如果边上没有交点则返回零向量
//
// 算法原理：
// 1. 在边的两个端点采样 SDF 值
// 2. 如果两端点符号相同（都没有穿越表面），返回无效
// 3. 如果符号不同，使用二分法逐步逼近零点
// 4. 每次迭代：将线段二分，检查中点符号，决定搜索哪一半
//============================================================================
HermiteData BinarySearchEdgeIntersection(const ImVec2& edgeStart, const ImVec2& edgeEnd, const SDFBase* sdf, int iterations = 8);

// 计算 Hermite 数据的法线（通过 SDF 梯度）
// 使用有限差分法近似计算梯度：
//   ∂SDF/∂x ≈ (SDF(x+ε,y) - SDF(x-ε,y)) / (2ε)
//   ∂SDF/∂y ≈ (SDF(x,y+ε) - SDF(x,y-ε)) / (2ε)
// 法线 = normalize(gradient)
ImVec2 ComputeNormal(const ImVec2& point, const SDFBase* sdf, float epsilon = 0.0001f);

// 查找节点四条边与 SDF 表面的所有交点
// 参数：
//   node: 四叉树节点
//   sdf: SDF 函数
// 返回值：
//   EdgeIntersections 结构，包含四条边的交点数组
//
// 边定义：
//   节点边界框 [minX, maxX] x [minY, maxY]
//   底边: (minX, minY) -> (maxX, minY)
//   顶边: (minX, maxY) -> (maxX, maxY)
//   左边: (minX, minY) -> (minX, maxY)
//   右边: (maxX, minY) -> (maxX, maxY)
EdgeIntersections FindEdgeIntersections(QuadtreeNode* node, const SDFBase* sdf);

//=============================================================================
// 递归收集 Hermite 数据
// 遍历四叉树所有节点，收集边交点数据
//=============================================================================

// 递归收集所有节点的 Hermite 数据
// 参数：
//   node: 四叉树节点
//   sdf: SDF 函数
//   output: 输出向量，存储所有 Hermite 数据
//
// 遍历顺序：深度优先（先处理当前节点，再递归处理子节点）
void CollectAllHermiteData(QuadtreeNode* node, const SDFBase* sdf, std::vector<HermiteData>& output);

// 检查边是否有表面穿越（用于优化：跳过无交点的边）
// 通过检查边两端点的 SDF 符号判断
bool EdgeHasSignChange(const ImVec2& start, const ImVec2& end, const SDFBase* sdf);

} // namespace dc
