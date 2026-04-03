#include "dc_quadtree.h"

#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace dc {

//=============================================================================
// Quadtree 构造函数/析构函数
//=============================================================================
Quadtree::Quadtree() 
    : m_root(nullptr), m_maxDepth(0), m_nodeCount(0) {
}

Quadtree::~Quadtree() {
    deleteTree(m_root);
    m_root = nullptr;
}

//=============================================================================
// 创建根节点
// center: 根节点中心位置
// size: 根节点半尺寸
// maxDepth: 最大递归深度
//=============================================================================
QuadtreeNode* Quadtree::createRoot(const ImVec2& center, float size, int maxDepth) {
    // 清理旧树
    deleteTree(m_root);
    m_root = nullptr;
    
    m_maxDepth = maxDepth;
    m_nodeCount = 1; // 根节点
    
    // 创建根节点
    m_root = new QuadtreeNode();
    m_root->bounds = BoundingBox(center, size);
    m_root->depth = 0;
    m_root->subdivided = false;
    m_root->nw = m_root->ne = m_root->sw = m_root->se = nullptr;
    
    return m_root;
}

//=============================================================================
// 递归构建四叉树
// 核心算法：
//   1. 检查当前节点是否应该细分（深度限制 or 有表面穿越）
//   2. 如果应该细分，创建四个子节点
//   3. 递归处理每个子节点
//=============================================================================
void Quadtree::build(QuadtreeNode* node, const SDFBase* sdf, int maxDepth) {
    if (!node || !sdf) return;
    
    // 达到最大深度，不继续细分
    if (node->depth >= maxDepth) {
        return;
    }
    
    // 检查是否需要细分
    if (!shouldSubdivide(node, sdf)) {
        return; // 节点内无表面穿越，不细分
    }
    
    // 细分当前节点
    subdivide(node);
    
    // 递归构建子节点
    if (node->nw) build(node->nw, sdf, maxDepth);
    if (node->ne) build(node->ne, sdf, maxDepth);
    if (node->sw) build(node->sw, sdf, maxDepth);
    if (node->se) build(node->se, sdf, maxDepth);
}

//=============================================================================
// 细分节点
// 将节点分为四个相等的子区域：
//   NW (左上) | NE (右上)
//   ----------|----------
//   SW (左下) | SE (右下)
//=============================================================================
void Quadtree::subdivide(QuadtreeNode* node) {
    if (!node || node->subdivided) return;
    
    // 创建四个子节点
    node->nw = new QuadtreeNode();
    node->ne = new QuadtreeNode();
    node->sw = new QuadtreeNode();
    node->se = new QuadtreeNode();
    
    // 设置子节点边界框
    node->nw->bounds = node->bounds.nw();
    node->ne->bounds = node->bounds.ne();
    node->sw->bounds = node->bounds.sw();
    node->se->bounds = node->bounds.se();
    
    // 设置子节点深度
    node->nw->depth = node->ne->depth = node->sw->depth = node->se->depth = node->depth + 1;
    
    // 标记为已细分
    node->subdivided = true;
    
    // 更新节点计数
    m_nodeCount += 4;
}

//=============================================================================
// 判断节点是否应该细分
// 原理：检查节点边界框内是否有 SDF 符号变化
// 如果边界框四个角样本点中存在符号变化，说明表面穿过该节点
//=============================================================================
bool Quadtree::shouldSubdivide(const QuadtreeNode* node, const SDFBase* sdf) const {
    if (!node || !sdf) return false;
    
    return hasSignChange(node->bounds, sdf);
}

//=============================================================================
// 检查边界框是否有符号变化
// 采样边界框四个角的 SDF 值，检测是否有异号情况
// 如果四个角全正或全负，说明无表面穿越
// 如果有正有负，说明表面穿越了该节点
//=============================================================================
bool Quadtree::hasSignChange(const BoundingBox& bounds, const SDFBase* sdf) const {
    // 采样四个角
    ImVec2 corners[4] = {
        bounds.min(),                            // SW corner
        ImVec2(bounds.max().x, bounds.min().y),   // SE corner
        ImVec2(bounds.min().x, bounds.max().y),   // NW corner
        bounds.max()                             // NE corner
    };
    
    // 获取四个角的值
    float values[4];
    for (int i = 0; i < 4; i++) {
        values[i] = sdf->evaluate(corners[i]);
    }
    
    // 检查是否有符号变化
    // 使用 epsilon 处理近零情况
    const float epsilon = 1e-6f;
    
    bool hasNeg = false, hasPos = false, hasNearZero = false;
    
    for (int i = 0; i < 4; i++) {
        if (values[i] < -epsilon) hasNeg = true;
        else if (values[i] > epsilon) hasPos = true;
        else hasNearZero = true; // 刚好在表面上
    }
    
    // 有符号变化 或 有角在表面上（视为穿越）
    return (hasNeg && hasPos) || hasNearZero;
}

//=============================================================================
// 统计节点数量（递归）
//=============================================================================
size_t Quadtree::countNodes(const QuadtreeNode* node) {
    if (!node) return 0;
    
    size_t count = 1; // 本身
    
    if (node->subdivided) {
        count += countNodes(node->nw);
        count += countNodes(node->ne);
        count += countNodes(node->sw);
        count += countNodes(node->se);
    }
    
    return count;
}

//=============================================================================
// 释放四叉树所有节点（递归）
//=============================================================================
void Quadtree::deleteTree(QuadtreeNode* node) {
    if (!node) return;
    
    // 递归删除子节点
    deleteTree(node->nw);
    deleteTree(node->ne);
    deleteTree(node->sw);
    deleteTree(node->se);
    
    // 删除自身
    delete node;
}

//=============================================================================
// 便捷函数：创建并构建完整四叉树
//=============================================================================
QuadtreeNode* CreateQuadtree(const ImVec2& center, float size, int maxDepth, const SDFBase* sdf) {
    Quadtree tree;
    QuadtreeNode* root = tree.createRoot(center, size, maxDepth);
    tree.build(root, sdf, maxDepth);
    
    // 返回根节点（调用者负责管理内存）
    // 注意：这里返回的是内部指针，用户需要通过 deleteQuadtree 释放
    // 为了简化，这里返回 nullptr，实际使用时需要保留 Quadtree 对象
    (void)root;
    (void)sdf;
    
    // 重新创建以返回独立对象
    // 实际上我们应该返回整个树，但为了简化API，这里返回根节点指针
    // 调用者需要自己管理内存
    return root;
}

//=============================================================================
// 便捷函数：细分节点
//=============================================================================
void Subdivide(QuadtreeNode* node) {
    if (!node) return;
    
    // 创建四个子节点
    node->nw = new QuadtreeNode();
    node->ne = new QuadtreeNode();
    node->sw = new QuadtreeNode();
    node->se = new QuadtreeNode();
    
    // 计算子节点边界
    float childHalfSize = node->bounds.halfSize * 0.5f;
    
    // NW: 左上
    node->nw->bounds = BoundingBox(
        ImVec2(node->bounds.center.x - childHalfSize, node->bounds.center.y + childHalfSize),
        childHalfSize
    );
    
    // NE: 右上
    node->ne->bounds = BoundingBox(
        ImVec2(node->bounds.center.x + childHalfSize, node->bounds.center.y + childHalfSize),
        childHalfSize
    );
    
    // SW: 左下
    node->sw->bounds = BoundingBox(
        ImVec2(node->bounds.center.x - childHalfSize, node->bounds.center.y - childHalfSize),
        childHalfSize
    );
    
    // SE: 右下
    node->se->bounds = BoundingBox(
        ImVec2(node->bounds.center.x + childHalfSize, node->bounds.center.y - childHalfSize),
        childHalfSize
    );
    
    // 设置深度
    node->nw->depth = node->ne->depth = node->sw->depth = node->se->depth = node->depth + 1;
    
    // 标记细分
    node->subdivided = true;
}

//=============================================================================
// 便捷函数：判断是否应该细分
//=============================================================================
bool ShouldSubdivide(QuadtreeNode* node, const SDFBase* sdf) {
    if (!node || !sdf || node->depth >= 16) return false;
    
    const float epsilon = 1e-6f;
    BoundingBox bounds = node->bounds;
    
    // 采样四个角
    ImVec2 corners[4] = {
        bounds.min(),
        ImVec2(bounds.max().x, bounds.min().y),
        ImVec2(bounds.min().x, bounds.max().y),
        bounds.max()
    };
    
    float values[4];
    for (int i = 0; i < 4; i++) {
        values[i] = sdf->evaluate(corners[i]);
    }
    
    bool hasNeg = false, hasPos = false, hasNearZero = false;
    
    for (int i = 0; i < 4; i++) {
        if (values[i] < -epsilon) hasNeg = true;
        else if (values[i] > epsilon) hasPos = true;
        else hasNearZero = true;
    }
    
    return (hasNeg && hasPos) || hasNearZero;
}

//=============================================================================
// 便捷函数：递归构建四叉树
//=============================================================================
void BuildQuadtree(QuadtreeNode* node, const SDFBase* sdf, int maxDepth) {
    if (!node || !sdf) return;
    if (node->depth >= maxDepth) return;
    
    // 检查是否需要细分
    if (!ShouldSubdivide(node, sdf)) return;
    
    // 细分节点
    Subdivide(node);
    
    // 递归构建子节点
    if (node->nw) BuildQuadtree(node->nw, sdf, maxDepth);
    if (node->ne) BuildQuadtree(node->ne, sdf, maxDepth);
    if (node->sw) BuildQuadtree(node->sw, sdf, maxDepth);
    if (node->se) BuildQuadtree(node->se, sdf, maxDepth);
}

} // namespace dc
