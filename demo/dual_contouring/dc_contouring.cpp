#include "dc_contouring.h"

#include <cmath>
#include <algorithm>

namespace dc {

//=============================================================================
// 辅助函数：检测节点是否包含表面穿越
//=============================================================================
// 使用边的符号变化检测节点是否与 SDF 表面相交
//
// 原理：
// 如果节点边界框的四个角点的 SDF 值符号不同（有的为正，有的为负），
// 则必有表面从节点内部穿过。
//
// 优化：
// 只检查边界框的四条边（12条边），而非四个角点（4个点），
// 这样可以检测到边与表面相交但四角共符号的情况。
//=============================================================================
static bool NodeHasSurfaceIntersection(QuadtreeNode* node, const SDFBase* sdf)
{
    const BoundingBox& bounds = node->bounds;
    ImVec2 minP = bounds.min();
    ImVec2 maxP = bounds.max();

    // 采样四个角点的 SDF 值
    float v00 = SampleSDF(ImVec2(minP.x, minP.y), sdf);
    float v10 = SampleSDF(ImVec2(maxP.x, minP.y), sdf);
    float v01 = SampleSDF(ImVec2(minP.x, maxP.y), sdf);
    float v11 = SampleSDF(ImVec2(maxP.x, maxP.y), sdf);

    // 检查符号变化：四个角不可能全正或全负
    bool hasNegative = (v00 < 0.0f) || (v10 < 0.0f) || (v01 < 0.0f) || (v11 < 0.0f);
    bool hasPositive = (v00 > 0.0f) || (v10 > 0.0f) || (v01 > 0.0f) || (v11 > 0.0f);

    // 如果符号不全相同，则表面穿过了这个区域
    return hasNegative && hasPositive;
}

//=============================================================================
// ContourNode - 递归处理四叉树节点
//=============================================================================
// 详细算法说明：
//
// 1. 表面检测：
//    - 首先检查节点是否包含表面穿越
//    - 如果节点没有细分且包含表面，则生成顶点
//    - 如果节点已细分，递归处理子节点
//
// 2. QEF 求解（叶子节点）：
//    - 收集节点四条边与表面的 Hermite 交点
//    - 每个交点提供一个切平面约束
//    - QEF 最小化顶点到所有切平面的距离平方和
//    - 求解结果存储在 context->vertices 中
//
// 3. 递归终止条件：
//    - 达到最大深度
//    - 节点已完全细分
//    - 节点不包含表面
//=============================================================================
void ContourNode(QuadtreeNode* node, ContouringContext* context)
{
    // 防御性检查：确保节点和上下文有效
    if (node == nullptr || context == nullptr || context->sdf == nullptr) {
        return;
    }

    // 步骤1：检查节点是否包含表面穿越
    // 如果没有表面穿越，这个节点不需要处理
    if (!NodeHasSurfaceIntersection(node, context->sdf)) {
        return;
    }

    // 步骤2：判断是叶子节点还是内部节点
    if (!node->subdivided || node->depth >= context->maxDepth) {
        // ========== 叶子节点处理 ==========
        //
        // 终止条件满足，执行 QEF 求解：
        // 1. 收集 Hermite 数据（边交点）
        // 2. 构建 QEF 约束
        // 3. 求解最佳顶点位置
        // 4. 存储顶点到输出数组

        // 查找节点四条边与 SDF 表面的所有交点
        EdgeIntersections intersections = FindEdgeIntersections(node, context->sdf);

        // 如果没有交点（理论上不应该发生，因为已检测到符号变化）
        if (intersections.isEmpty()) {
            return;
        }

        // 构建 QEF 求解器
        QEFSolver qef;

        // 添加所有 Hermite 数据作为约束
        for (const HermiteData& hd : intersections.bottom) {
            qef.AddConstraint(hd);
        }
        for (const HermiteData& hd : intersections.top) {
            qef.AddConstraint(hd);
        }
        for (const HermiteData& hd : intersections.left) {
            qef.AddConstraint(hd);
        }
        for (const HermiteData& hd : intersections.right) {
            qef.AddConstraint(hd);
        }

        // 检查是否有足够的有效约束
        if (!qef.HasValidConstraints()) {
            // 约束不足（所有法线共线），无法求解
            // 这种情况下，使用边界框中心作为默认顶点
            ImVec2 defaultVertex = node->bounds.center;
            context->vertices.push_back(defaultVertex);
            return;
        }

        // 求解 QEF 得到最佳顶点位置
        ImVec2 vertex = qef.Solve();

        // 验证解的质量：检查顶点是否在节点边界内
        // 如果顶点在边界外，将其拉回边界内（C++11 不支持 std::clamp）
        ImVec2 minP = node->bounds.min();
        ImVec2 maxP = node->bounds.max();
        vertex.x = std::fmax(minP.x, std::fmin(vertex.x, maxP.x));
        vertex.y = std::fmax(minP.y, std::fmin(vertex.y, maxP.y));

        // 将顶点添加到结果数组
        context->vertices.push_back(vertex);
    }
    else {
        // ========== 内部节点处理 ==========
        //
        // 递归处理四个子节点（NW, NE, SW, SE）
        // 按照四叉树的结构递归遍历

        if (node->nw != nullptr) {
            ContourNode(node->nw, context);
        }
        if (node->ne != nullptr) {
            ContourNode(node->ne, context);
        }
        if (node->sw != nullptr) {
            ContourNode(node->sw, context);
        }
        if (node->se != nullptr) {
            ContourNode(node->se, context);
        }
    }
}

//=============================================================================
// GenerateContour - 主函数：生成轮廓线顶点
//=============================================================================
// 算法流程：
//
// 1. 参数验证：
//    - sdf 不能为空
//    - root 不能为空
//    - maxDepth 必须为正数
//
// 2. 初始化上下文：
//    - 创建 ContouringContext
//    - 设置 SDF、最大深度和 QEF 阈值
//
// 3. 递归处理：
//    - 从根节点开始调用 ContourNode
//    - 递归遍历所有需要处理的节点
//
// 4. 返回结果：
//    - 返回包含所有顶点的向量
//
// 复杂度分析：
// - 时间复杂度：O(N)，其中 N 是四叉树节点数
// - 空间复杂度：O(V)，其中 V 是生成的顶点数（叶子节点数）
//=============================================================================
std::vector<ImVec2> GenerateContour(const SDFBase* sdf, QuadtreeNode* root, int maxDepth, float qefThreshold)
{
    // 参数验证
    if (sdf == nullptr) {
        return {};
    }
    if (root == nullptr) {
        return {};
    }
    if (maxDepth <= 0) {
        return {};
    }

    // 创建轮廓线生成上下文
    ContouringContext context(sdf, maxDepth, qefThreshold);

    // 从根节点开始递归处理
    ContourNode(root, &context);

    // 返回生成的顶点数组
    return context.vertices;
}

} // namespace dc