#include "dc_mesh.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <unordered_set>

namespace dc {

//=============================================================================
// 对偶网格构建与渲染实现
//=============================================================================
// 
// 对偶网格原理：
// 
// 在 Dual Contouring 中，我们有两套网格：
// 
// 1. 原网格（Primal Mesh）：
//    - 四叉树结构
//    - 叶子节点代表细分空间
//    - 节点边界框用于检测表面穿越
// 
// 2. 对偶网格（Dual Mesh）：
//    - 顶点在原网格的叶子节点内部
//    - 边连接相邻的节点顶点
//    - 形成轮廓线的对偶表示
// 
// 邻居关系（2D 四叉树）：
// 
//        +---+
//        | N |
//    +---+---+---+
//    | W |   | E |
//    +---+---+---+
//        | S |
//        +---+
// 
// 两个叶子节点是邻居的条件：
// - 它们的边界框共享一条边
// - 或者一个节点的边界框边与另一个节点边界框边完全重合
//=============================================================================

//=============================================================================
// 常量定义
//=============================================================================

// 邻居方向的枚举（用于索引）
enum class NeighborDirection : int {
    North = 0,
    South = 1,
    East  = 2,
    West  = 3
};

//=============================================================================
// 辅助函数实现
//=============================================================================

//-----------------------------------------------------------------------------
// 判断浮点数是否近似相等（考虑浮点误差）
//-----------------------------------------------------------------------------
static bool IsNearlyEqual(float a, float b, float epsilon = 1e-6f) {
    return std::abs(a - b) < epsilon;
}

//-----------------------------------------------------------------------------
// 获取两个边界框之间的共享边界长度
//-----------------------------------------------------------------------------
static float GetSharedEdgeLength(const BoundingBox& a, const BoundingBox& b) {
    // 计算两个边界框中心的距离
    float dx = std::abs(a.center.x - b.center.x);
    float dy = std::abs(a.center.y - b.center.y);
    
    // 判断是水平相邻还是垂直相邻
    // 如果是水平邻居（东或西），共享边的长度应该是边界框的高度
    // 如果是垂直邻居（北或南），共享边的长度应该是边界框的宽度
    float halfSizeA = a.halfSize;
    float halfSizeB = b.halfSize;
    
    if (IsNearlyEqual(dx, halfSizeA + halfSizeB)) {
        // 水平相邻（东或西）
        return std::min(a.halfSize * 2.0f, b.halfSize * 2.0f);
    } else if (IsNearlyEqual(dy, halfSizeA + halfSizeB)) {
        // 垂直相邻（北或南）
        return std::min(a.halfSize * 2.0f, b.halfSize * 2.0f);
    }
    
    return 0.0f; // 不相邻
}

//-----------------------------------------------------------------------------
// 判断两个边界框是否共享边界
//-----------------------------------------------------------------------------
static bool HasSharedEdge(const BoundingBox& a, const BoundingBox& b) {
    float sharedLength = GetSharedEdgeLength(a, b);
    return sharedLength > 0.0f;
}

//-----------------------------------------------------------------------------
// 判断节点是否是叶子节点
//-----------------------------------------------------------------------------
static bool IsLeafNode(const QuadtreeNode* node) {
    return !node->subdivided;
}

//-----------------------------------------------------------------------------
// 判断点是否在边界框内
//-----------------------------------------------------------------------------
static bool IsPointInBounds(const ImVec2& point, const BoundingBox& bounds) {
    ImVec2 minP = bounds.min();
    ImVec2 maxP = bounds.max();
    return point.x >= minP.x && point.x <= maxP.x &&
           point.y >= minP.y && point.y <= maxP.y;
}

//-----------------------------------------------------------------------------
// 获取叶子节点的四个潜在邻居方向
//-----------------------------------------------------------------------------
static void GetNeighborDirections(QuadtreeNode* leaf, 
                                   BoundingBox neighborBounds[4],
                                   NeighborDirection directions[4]) {
    if (!leaf || !IsLeafNode(leaf)) {
        return;
    }
    
    float hs = leaf->bounds.halfSize;
    ImVec2 center = leaf->bounds.center;
    
    // 北邻居：共享上边界
    neighborBounds[0] = BoundingBox(ImVec2(center.x, center.y + hs * 2.0f), hs);
    directions[0] = NeighborDirection::North;
    
    // 南邻居：共享下边界
    neighborBounds[1] = BoundingBox(ImVec2(center.x, center.y - hs * 2.0f), hs);
    directions[1] = NeighborDirection::South;
    
    // 东邻居：共享右边界
    neighborBounds[2] = BoundingBox(ImVec2(center.x + hs * 2.0f, center.y), hs);
    directions[2] = NeighborDirection::East;
    
    // 西邻居：共享左边界
    neighborBounds[3] = BoundingBox(ImVec2(center.x - hs * 2.0f, center.y), hs);
    directions[3] = NeighborDirection::West;
}

//-----------------------------------------------------------------------------
// 在节点数组中查找与给定边界框匹配的节点
//-----------------------------------------------------------------------------
static QuadtreeNode* FindNodeByBounds(QuadtreeNode* node, const BoundingBox& targetBounds) {
    if (!node) {
        return nullptr;
    }
    
    // 检查当前节点
    if (IsNearlyEqual(node->bounds.center.x, targetBounds.center.x) &&
        IsNearlyEqual(node->bounds.center.y, targetBounds.center.y) &&
        IsNearlyEqual(node->bounds.halfSize, targetBounds.halfSize)) {
        return node;
    }
    
    // 如果是叶子节点，不再递归
    if (IsLeafNode(node)) {
        return nullptr;
    }
    
    // 递归搜索子节点
    QuadtreeNode* found = nullptr;
    
    found = FindNodeByBounds(node->nw, targetBounds);
    if (found) return found;
    
    found = FindNodeByBounds(node->ne, targetBounds);
    if (found) return found;
    
    found = FindNodeByBounds(node->sw, targetBounds);
    if (found) return found;
    
    found = FindNodeByBounds(node->se, targetBounds);
    if (found) return found;
    
    return nullptr;
}

//-----------------------------------------------------------------------------
// 收集所有叶子节点
//-----------------------------------------------------------------------------
static void CollectLeafNodes(QuadtreeNode* node, std::vector<QuadtreeNode*>& leaves) {
    if (!node) {
        return;
    }
    
    if (IsLeafNode(node)) {
        leaves.push_back(node);
        return;
    }
    
    // 递归收集子节点
    if (node->nw) CollectLeafNodes(node->nw, leaves);
    if (node->ne) CollectLeafNodes(node->ne, leaves);
    if (node->sw) CollectLeafNodes(node->sw, leaves);
    if (node->se) CollectLeafNodes(node->se, leaves);
}

//-----------------------------------------------------------------------------
// 生成边的唯一标识符
//-----------------------------------------------------------------------------
static uint64_t MakeEdgeKey(int idx1, int idx2) {
    // 确保较小的索引在前，这样 (a,b) 和 (b,a) 生成相同的 key
    if (idx1 > idx2) {
        std::swap(idx1, idx2);
    }
    return (static_cast<uint64_t>(idx1) << 32) | static_cast<uint32_t>(idx2);
}

//=============================================================================
// 主函数实现
//=============================================================================

DualMesh BuildDualMesh(QuadtreeNode* root, ContouringContext* context) {
    DualMesh mesh;
    
    if (!root || !context || context->vertices.empty()) {
        return mesh;
    }
    
    // 第一步：复制顶点到网格
    // 每个 contour 顶点对应一个包含表面的叶子节点
    mesh.vertices.reserve(context->vertices.size());
    for (const ImVec2& v : context->vertices) {
        mesh.vertices.push_back(MeshVertex(v));
    }
    
    // 第二步：构建顶点到叶子节点的映射
    // 由于顶点在 ContourNode 中生成时已经隐含了叶子节点信息，
    // 我们需要通过遍历四叉树来重建这个映射关系
    
    struct NodeVertexPair {
        QuadtreeNode* node;
        ImVec2 vertex;
    };
    std::vector<NodeVertexPair> nodeVertexPairs;
    
    // 收集所有叶子节点
    std::vector<QuadtreeNode*> leaves;
    CollectLeafNodes(root, leaves);
    
    // 建立顶点位置到叶子节点的映射
    // 注意：由于浮点精度问题，我们使用近似的空间搜索
    for (QuadtreeNode* leaf : leaves) {
        for (const ImVec2& v : context->vertices) {
            // 检查顶点是否在此叶子节点的边界内
            if (IsPointInBounds(v, leaf->bounds)) {
                NodeVertexPair pair;
                pair.node = leaf;
                pair.vertex = v;
                nodeVertexPairs.push_back(pair);
            }
        }
    }
    
    // 第三步：构建边
    // 对于每对相邻的叶子节点，如果它们都有顶点，则连接它们的顶点
    std::unordered_set<uint64_t> addedEdges;
    mesh.edges.reserve(nodeVertexPairs.size() * 4); // 估计每顶点最多4条边
    
    // 用于存储同一顶点的所有引用（因为可能有多个叶子节点包含同一点）
    struct VertexRef {
        int vertexIdx;
        QuadtreeNode* node;
    };
    std::vector<std::vector<VertexRef>> vertexRefs(mesh.vertices.size());
    
    // 建立 vertex 到索引的映射
    for (size_t i = 0; i < nodeVertexPairs.size(); ++i) {
        const ImVec2& v = nodeVertexPairs[i].vertex;
        // 找到 vertex 在 mesh.vertices 中的索引
        for (size_t j = 0; j < mesh.vertices.size(); ++j) {
            if (IsNearlyEqual(mesh.vertices[j].position.x, v.x) &&
                IsNearlyEqual(mesh.vertices[j].position.y, v.y)) {
                VertexRef ref;
                ref.vertexIdx = static_cast<int>(j);
                ref.node = nodeVertexPairs[i].node;
                vertexRefs[j].push_back(ref);
                break;
            }
        }
    }
    
    // 第四步：对于每对叶子节点，检查它们是否相邻
    // 如果相邻且都有顶点，则添加边
    for (size_t i = 0; i < nodeVertexPairs.size(); ++i) {
        QuadtreeNode* node1 = nodeVertexPairs[i].node;
        
        // 获取 node1 的邻居
        BoundingBox neighborBounds[4];
        NeighborDirection directions[4];
        GetNeighborDirections(node1, neighborBounds, directions);
        
        for (int d = 0; d < 4; ++d) {
            QuadtreeNode* neighbor = FindNodeByBounds(root, neighborBounds[d]);
            
            if (neighbor && IsLeafNode(neighbor)) {
                // 检查 neighbor 是否也在 nodeVertexPairs 中
                for (size_t k = 0; k < nodeVertexPairs.size(); ++k) {
                    if (nodeVertexPairs[k].node == neighbor) {
                        // 找到了相邻的叶子节点
                        // 获取两个顶点的索引
                        int idx1 = -1, idx2 = -1;
                        
                        // 找到 node1 对应的顶点索引
                        for (const VertexRef& ref : vertexRefs[i]) {
                            if (ref.node == node1) {
                                idx1 = ref.vertexIdx;
                                break;
                            }
                        }
                        
                        // 找到 neighbor 对应的顶点索引
                        for (const VertexRef& ref : vertexRefs[k]) {
                            if (ref.node == neighbor) {
                                idx2 = ref.vertexIdx;
                                break;
                            }
                        }
                        
                        // 如果两个顶点都找到了，添加边
                        if (idx1 >= 0 && idx2 >= 0 && idx1 != idx2) {
                            uint64_t edgeKey = MakeEdgeKey(idx1, idx2);
                            
                            // 检查边是否已存在
                            if (addedEdges.find(edgeKey) == addedEdges.end()) {
                                addedEdges.insert(edgeKey);
                                mesh.edges.push_back(MeshEdge(idx1, idx2));
                            }
                        }
                        
                        break; // 找到匹配的邻居后不再继续搜索
                    }
                }
            }
        }
    }
    
    return mesh;
}

//-----------------------------------------------------------------------------
// 获取叶子节点的直接邻居（在同一父节点下的兄弟节点）
//-----------------------------------------------------------------------------
void GetLeafNeighbors(QuadtreeNode* leaf, QuadtreeNode* neighbors[4], int& neighborCount) {
    neighborCount = 0;
    
    if (!leaf || !IsLeafNode(leaf)) {
        return;
    }
    
    // 如果有父节点，检查兄弟节点
    // 注意：这个实现比较简化，主要用于 demo
    // 完整的邻居查找需要考虑更复杂的情况
    (void)neighbors;
    (void)neighborCount;
}

//-----------------------------------------------------------------------------
// 获取两个相邻节点之间的边界中点
//-----------------------------------------------------------------------------
ImVec2 GetSharedEdgeMidpoint(QuadtreeNode* node1, QuadtreeNode* node2) {
    if (!node1 || !node2) {
        return ImVec2(0.0f, 0.0f);
    }
    
    const BoundingBox& b1 = node1->bounds;
    const BoundingBox& b2 = node2->bounds;
    
    // 计算中点
    return ImVec2(
        (b1.center.x + b2.center.x) * 0.5f,
        (b1.center.y + b2.center.y) * 0.5f
    );
}

//-----------------------------------------------------------------------------
// 检查两个节点是否共享边界
//-----------------------------------------------------------------------------
bool AreNeighbors(const QuadtreeNode* node1, const QuadtreeNode* node2) {
    if (!node1 || !node2) {
        return false;
    }
    
    return HasSharedEdge(node1->bounds, node2->bounds);
}

//-----------------------------------------------------------------------------
// 根据顶点位置找到对应的叶子节点
//-----------------------------------------------------------------------------
QuadtreeNode* FindNodeContainingPoint(QuadtreeNode* root, const ImVec2& point) {
    if (!root) {
        return nullptr;
    }
    
    QuadtreeNode* current = root;
    
    while (current) {
        if (IsLeafNode(current)) {
            // 检查点是否在此叶子节点内
            if (IsPointInBounds(point, current->bounds)) {
                return current;
            }
            return nullptr;
        }
        
        // 向下递归查找
        const ImVec2& c = current->bounds.center;
        bool isNorth = point.y > c.y;
        bool isEast = point.x > c.x;
        
        if (isNorth && isEast) {
            current = current->ne;
        } else if (isNorth && !isEast) {
            current = current->nw;
        } else if (!isNorth && isEast) {
            current = current->se;
        } else {
            current = current->sw;
        }
    }
    
    return nullptr;
}

//=============================================================================
// 渲染实现
//=============================================================================

void RenderMesh(const DualMesh& mesh, Renderer* renderer, ImU32 color, float thickness) {
    if (!renderer || !mesh.IsValid()) {
        return;
    }
    
    // 绘制每条边
    for (const MeshEdge& edge : mesh.edges) {
        if (edge.startIdx >= 0 && edge.startIdx < static_cast<int>(mesh.vertices.size()) &&
            edge.endIdx >= 0 && edge.endIdx < static_cast<int>(mesh.vertices.size())) {
            
            ImVec2 p1 = mesh.vertices[edge.startIdx].position;
            ImVec2 p2 = mesh.vertices[edge.endIdx].position;
            
            // 使用公开的 WorldToScreen 方法进行坐标变换（内部有 fallback）
            p1 = renderer->WorldToScreen(p1);
            p2 = renderer->WorldToScreen(p2);
            
            renderer->DrawLine(p1, p2, color, thickness);
        }
    }
}

} // namespace dc
