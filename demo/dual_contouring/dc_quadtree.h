#pragma once

#include "dc_sdf.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace dc {

//=============================================================================
// 四叉树节点边界框
// 使用中心点 + 半尺寸表示（更方便计算子节点位置）
//=============================================================================
struct BoundingBox {
    ImVec2  center;   // 边界框中心点
    float halfSize; // 半尺寸（从中心到边缘的距离）
    
    BoundingBox() : center(0.0f, 0.0f), halfSize(0.0f) {}
    BoundingBox(const ImVec2& c, float hs) : center(c), halfSize(hs) {}
    
    // 获取边界框的四个角
    ImVec2 min() const { return ImVec2(center.x - halfSize, center.y - halfSize); }
    ImVec2 max() const { return ImVec2(center.x + halfSize, center.y + halfSize); }
    
    // 获取四个子节点的边界框（各占四分之一大小）
    // 使用 ImVec2 进行计算
    BoundingBox nw() const { return BoundingBox(ImVec2(center.x - halfSize * 0.5f, center.y + halfSize * 0.5f), halfSize * 0.5f); }
    BoundingBox ne() const { return BoundingBox(ImVec2(center.x + halfSize * 0.5f, center.y + halfSize * 0.5f), halfSize * 0.5f); }
    BoundingBox sw() const { return BoundingBox(ImVec2(center.x - halfSize * 0.5f, center.y - halfSize * 0.5f), halfSize * 0.5f); }
    BoundingBox se() const { return BoundingBox(ImVec2(center.x + halfSize * 0.5f, center.y - halfSize * 0.5f), halfSize * 0.5f); }
    
    // 判断点是否在边界框内
    bool contains(const ImVec2& p) const {
        return p.x >= min().x && p.x <= max().x &&
               p.y >= min().y && p.y <= max().y;
    }
    
    // 判断是否与另一个边界框相交
    bool intersects(const BoundingBox& o) const {
        return !(o.min().x > max().x || o.max().x < min().x ||
                 o.min().y > max().y || o.max().y < min().y);
    }
};

//=============================================================================
// 四叉树节点
// 采用指针方式管理子节点，便于递归操作
//=============================================================================
struct QuadtreeNode {
    BoundingBox        bounds;      // 节点边界框
    int                depth;       // 节点深度（根节点=0）
    bool               subdivided;  // 是否已细分
    
    // 四个子节点指针（按方位命名）
    //   NW | NE
    //   -------
    //   SW | SE
    QuadtreeNode*      nw;
    QuadtreeNode*      ne;
    QuadtreeNode*      sw;
    QuadtreeNode*      se;
    
    QuadtreeNode() : bounds(), depth(0), subdivided(false), nw(nullptr), ne(nullptr), sw(nullptr), se(nullptr) {}
    ~QuadtreeNode() {
        delete nw; nw = nullptr;
        delete ne; ne = nullptr;
        delete sw; sw = nullptr;
        delete se; se = nullptr;
    }
};

//=============================================================================
// 四叉树构建器
// 核心功能：根据 SDF 表面位置自适应细分四叉树
//=============================================================================
class Quadtree {
public:
    Quadtree();
    ~Quadtree();
    
    // 创建根节点
    // center: 根节点中心位置
    // size: 根节点半尺寸
    // maxDepth: 最大递归深度
    QuadtreeNode* createRoot(const ImVec2& center, float size, int maxDepth);
    
    // 递归构建四叉树
    // 根据 SDF 表面穿越情况决定是否细分节点
    void build(QuadtreeNode* node, const SDFBase* sdf, int maxDepth);
    
    // 获取根节点
    QuadtreeNode* root() { return m_root; }
    const QuadtreeNode* root() const { return m_root; }
    
    // 获取节点总数
    size_t nodeCount() const { return m_nodeCount; }
    
    // 递归统计子节点数量
    static size_t countNodes(const QuadtreeNode* node);
    
    // 递归释放所有节点
    static void deleteTree(QuadtreeNode* node);
    
private:
    // 细分节点（创建四个子节点）
    void subdivide(QuadtreeNode* node);
    
    // 判断节点是否应该细分
    // 条件：节点边界框内有 SDF 符号变化（表面穿越）
    bool shouldSubdivide(const QuadtreeNode* node, const SDFBase* sdf) const;
    
    // 检查边界框四个角是否有符号变化
    bool hasSignChange(const BoundingBox& bounds, const SDFBase* sdf) const;
    
    QuadtreeNode* m_root;
    int           m_maxDepth;
    size_t        m_nodeCount;
};

//=============================================================================
// 便捷函数接口
//=============================================================================

// 创建并构建完整的四叉树
QuadtreeNode* CreateQuadtree(const ImVec2& center, float size, int maxDepth, const SDFBase* sdf);

// 细分节点（暴露给外部使用）
void Subdivide(QuadtreeNode* node);

// 判断节点是否应该细分
bool ShouldSubdivide(QuadtreeNode* node, const SDFBase* sdf);

// 递归构建四叉树
void BuildQuadtree(QuadtreeNode* node, const SDFBase* sdf, int maxDepth);

} // namespace dc
