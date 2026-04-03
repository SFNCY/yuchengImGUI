#include "dc_qef.h"

namespace dc {

//=============================================================================
// QEF 求解器实现
//=============================================================================

void QEFSolver::AddConstraint(const ImVec2& normal, const ImVec2& point) {
    m_normals.push_back(normal);
    m_points.push_back(point);
}

void QEFSolver::AddConstraint(const HermiteData& data) {
    AddConstraint(data.normal, data.position);
}

void QEFSolver::Clear() {
    m_normals.clear();
    m_points.clear();
}

bool QEFSolver::HasValidConstraints() const {
    // 至少需要 2 个不共线的约束才能在 2D 中求解
    if (m_normals.size() < 2) {
        return false;
    }
    
    // 检查法线是否不全部共线
    // 取第一个法线，检查是否有法线与它不平行
    const ImVec2& firstNormal = m_normals[0];
    float lenFirst = std::hypot(firstNormal.x, firstNormal.y);
    if (lenFirst < 1e-8f) {
        return false;  // 第一个法线是零向量，无效
    }
    
    // 检查是否存在与第一个法线不共线的法线
    for (size_t i = 1; i < m_normals.size(); ++i) {
        const ImVec2& n = m_normals[i];
        float len = std::hypot(n.x, n.y);
        if (len < 1e-8f) {
            continue;  // 跳过零向量
        }
        
        // 检查是否共线：|n1 x n2| ≈ 0 表示共线
        // 在 2D 中，叉积的模 = |x1*y2 - x2*y1|
        float cross = firstNormal.x * n.y - firstNormal.y * n.x;
        if (std::abs(cross) > 1e-6f) {
            return true;  // 找到不共线的法线
        }
    }
    
    return false;  // 所有法线都共线
}

ImVec2 QEFSolver::Solve() {
    // 检查约束数量
    if (m_normals.empty()) {
        return ImVec2(0.0f, 0.0f);  // 无约束，返回原点
    }
    
    if (m_normals.size() == 1) {
        // 只有一个约束，解不唯一，返回法线方向上的点
        // 约束：n · x = n · p
        // 选择 x = p - n * ((n · p) / |n|^2 - ...)，简化为 x = p
        // 实际上一个法线约束不能唯一确定 2D 点
        return m_points[0];
    }
    
    // 构建法线矩阵 A 和向量 b
    // A 的每一行是法线 n_i^T
    // b_i = n_i · p_i
    
    // 使用正规方程求解：A^T A x = A^T b
    // 令 M = A^T A (2x2 对称矩阵)
    // 令 c = A^T b (2维向量)
    
    float m00 = 0.0f;  // A^T A[0][0] = sum of n_i.x * n_i.x
    float m01 = 0.0f;  // A^T A[0][1] = A^T A[1][0] = sum of n_i.x * n_i.y
    float m11 = 0.0f;  // A^T A[1][1] = sum of n_i.y * n_i.y
    float c0 = 0.0f;   // (A^T b)[0] = sum of n_i.x * (n_i · p_i)
    float c1 = 0.0f;   // (A^T b)[1] = sum of n_i.y * (n_i · p_i)
    
    for (size_t i = 0; i < m_normals.size(); ++i) {
        const ImVec2& n = m_normals[i];
        const ImVec2& p = m_points[i];
        
        float nx = n.x;
        float ny = n.y;
        float np = nx * p.x + ny * p.y;  // n · p
        
        m00 += nx * nx;
        m01 += nx * ny;
        m11 += ny * ny;
        c0 += nx * np;
        c1 += ny * np;
    }
    
    // 计算矩阵 M = [[m00, m01], [m01, m11]] 的行列式
    float det = m00 * m11 - m01 * m01;
    
    // 检查矩阵是否奇异或接近奇异
    const float epsilon = 1e-10f;
    
    if (std::abs(det) < epsilon) {
        // 矩阵奇异或接近奇异（秩不足）
        // 使用扰动方法处理
        return SolveSingularCase(m_normals, ImVec2(c0, c1));
    }
    
    // 使用 Cramer's rule 求解 2x2 线性系统
    // M * x = c
    // x = M^-1 * c
    // M^-1 = (1/det) * [[m11, -m01], [-m01, m00]]
    
    float invDet = 1.0f / det;
    float x = invDet * (m11 * c0 - m01 * c1);
    float y = invDet * (-m01 * c0 + m00 * c1);
    
    return ImVec2(x, y);
}

ImVec2 QEFSolver::SolveSingularCase(const std::vector<ImVec2>& normals, const ImVec2& b) {
    // 当 A^T A 奇异时使用扰动方法
    // 添加小量到对角线：M' = M + ε * I
    // 这使得矩阵可逆，代价是解略有偏移
    
    const float perturbation = 1e-8f;
    
    // 重新计算 M 矩阵元素
    float m00 = 0.0f, m01 = 0.0f, m11 = 0.0f;
    
    for (const ImVec2& n : normals) {
        m00 += n.x * n.x;
        m01 += n.x * n.y;
        m11 += n.y * n.y;
    }
    
    // 添加扰动到对角线
    m00 += perturbation;
    m11 += perturbation;
    
    float det = m00 * m11 - m01 * m01;
    
    // 如果仍然奇异（所有法线都为零向量），返回原点
    if (std::abs(det) < 1e-16f) {
        return ImVec2(0.0f, 0.0f);
    }
    
    float invDet = 1.0f / det;
    float x = invDet * (m11 * b.x - m01 * b.y);
    float y = invDet * (-m01 * b.x + m00 * b.y);
    
    return ImVec2(x, y);
}

float QEFSolver::GetError() {
    // 计算 QEF 误差：||Ax - b||^2
    if (m_normals.empty()) {
        return 0.0f;
    }
    
    // 先求解 x
    ImVec2 x = Solve();
    
    // 计算误差
    float error = 0.0f;
    
    for (size_t i = 0; i < m_normals.size(); ++i) {
        const ImVec2& n = m_normals[i];
        const ImVec2& p = m_points[i];
        
        // 计算 n · x - n · p
        // 即约束 residual
        float residual = Dot(n, x) - Dot(n, p);
        error += residual * residual;
    }
    
    return error;
}

} // namespace dc