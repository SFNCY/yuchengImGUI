#pragma once

#include "dc_sdf.h"
#include "dc_quadtree.h"
#include "dc_hermite.h"
#include "dc_qef.h"

#include <vector>

namespace dc {

//=============================================================================
// Dual Contouring (双轮廓线) 算法
//=============================================================================
// 算法原理：
// 双轮廓线是一种从体素网格（如四叉树/八叉树）生成高质量轮廓线的方法。
// 它结合了 Marching Cubes 的表面穿越检测和基于 QEF 的顶点优化。
//
// 核心思想：
// 1. 在每个包含表面的四叉树节点内部放置一个顶点
// 2. 顶点位置通过最小化 QEF（Quadratic Error Function）确定
// 3. QEF 使得顶点倾向于位于所有切平面的交集处（表面中心）
// 4. 通过连接相邻节点的顶点形成轮廓线
//
// 关键特性：
// - 顶点位于节点内部，不会在表面穿越处
// - 生成的轮廓线是流形（manifold）的
// - 自动处理尖锐特征（角落）
//
// 参考论文：
// - "Dual Contouring of Hermite Data" (Ju et al., 2002)
// - "Reconstructing Triangle Meshes from Multi-View Images" (Lovett et al.)
//=============================================================================

//=============================================================================
// ContouringContext - 轮廓线生成的上下文信息
//=============================================================================
// 存储 DC 算法执行过程中需要的所有状态和数据
//
// 成员说明：
// - sdf: 指向 SDF 函数的指针，用于计算距离和检测表面穿越
// - maxDepth: 四叉树最大递归深度，控制轮廓线细节级别
// - qefThreshold: QEF 求解误差阈值，用于判断解的质量
// - vertices: 输出参数，存储生成的顶点位置数组
//=============================================================================
struct ContouringContext {
    const SDFBase*  sdf;            // SDF 函数指针
    int             maxDepth;       // 最大递归深度
    float           qefThreshold;   // QEF 误差阈值
    std::vector<ImVec2> vertices;     // 生成的顶点位置数组

    ContouringContext()
        : sdf(nullptr)
        , maxDepth(0)
        , qefThreshold(1e-6f)
    {}

    ContouringContext(const SDFBase* inSdf, int inMaxDepth, float inThreshold = 1e-6f)
        : sdf(inSdf)
        , maxDepth(inMaxDepth)
        , qefThreshold(inThreshold)
    {}
};

//=============================================================================
// ContourNode - 递归处理四叉树节点
//=============================================================================
// 核心递归函数，对四叉树节点执行双轮廓线算法
//
// 算法步骤：
// 1. 检查节点是否包含表面穿越（通过边的符号变化检测）
// 2. 如果是叶子节点：
//    a. 收集节点四条边与表面的所有 Hermite 交点数据
//    b. 使用 Hermite 数据构建 QEF（切平面约束）
//    c. 求解 QEF 得到最佳顶点位置
//    d. 将顶点添加到结果数组
// 3. 如果是内部节点：
//    a. 递归处理四个子节点（NW, NE, SW, SE）
//
// 参数：
//   node: 四叉树节点指针
//   context: 轮廓线生成上下文
//
// 返回值：
//   无（顶点通过 context->vertices 输出）
//
// 注意事项：
// - 顶点只生成在包含表面的叶子节点
// - 内部节点不生成顶点，只用于引导递归
//=============================================================================
void ContourNode(QuadtreeNode* node, ContouringContext* context);

//=============================================================================
// GenerateContour - 主函数：生成轮廓线顶点
//=============================================================================
// 双轮廓线算法的主入口函数
//
// 算法流程：
// 1. 验证输入参数（sdf 不能为空，maxDepth > 0）
// 2. 创建轮廓线生成上下文
// 3. 从根节点开始递归处理四叉树
// 4. 返回生成的顶点数组
//
// 参数：
//   sdf: SDF 函数指针，定义要提取的表面形状
//   root: 四叉树根节点指针（由 Quadtree 创建）
//   maxDepth: 最大递归深度
//   qefThreshold: QEF 求解误差阈值（可选，默认 1e-6）
//
// 返回值：
//   std::vector<vec2>: 包含所有生成顶点位置的数组
//
// 使用示例：
//   Quadtree quadtree;
//   QuadtreeNode* root = quadtree.createRoot(vec2(0, 0), 2.0f, 5);
//   quadtree.build(root, &circleSdf, 5);
//   std::vector<vec2> verts = GenerateContour(&circleSdf, root, 5);
//=============================================================================
std::vector<ImVec2> GenerateContour(const SDFBase* sdf, QuadtreeNode* root, int maxDepth, float qefThreshold = 1e-6f);

} // namespace dc