# Dual Contouring Learning - Learnings

## Task 4: ImDrawList 基础渲染框架

### ImDrawList API 关键发现

1. **获取 ImDrawList**:
   ```cpp
   ImDrawList* draw_list = ImGui::GetWindowDrawList();
   ```

2. **绘制线段**:
   ```cpp
   draw_list->AddLine(p1, p2, color, thickness);
   // 签名: void AddLine(const ImVec2& p1, const ImVec2& p2, ImU32 col, float thickness = 1.0f)
   ```

3. **绘制圆（轮廓）**:
   ```cpp
   draw_list->AddCircle(center, radius, color, segments, thickness);
   // 签名: void AddCircle(const ImVec2& center, float radius, ImU32 col,
   //                      int num_segments = 0, float thickness = 1.0f)
   // 当 num_segments = 0 时，ImGui 会根据 CircleSegmentMaxError 自动细分
   ```

4. **绘制填充圆**:
   ```cpp
   draw_list->AddCircleFilled(center, radius, color, segments);
   ```

5. **绘制多边形/四边形**:
   ```cpp
   draw_list->AddPolyline(points, num_points, color, flags, thickness);
   // 使用 ImDrawFlags_Closed 闭合多边形
   ```

### 重要约定

- `vec2` 类型是 `ImVec2` 的别名
- 颜色使用 `IM_COL32(r, g, b, a)` 宏定义
- `ImU32` 是 32 位无符号整数（颜色值）
- 渲染必须在 `ImGui::NewFrame()` 和 `ImGui::EndFrame()` 之间进行

### Marching Squares 算法

用于绘制 SDF 等值线：
- 将搜索区域网格化
- 对每个单元格，根据四个角点的 SDF 值与等值线的关系确定索引 (0-15)
- 使用查表法确定哪些边缘有交线穿过
- 通过线性插值计算交点位置
- 连接相邻单元格的交点形成等值线

### 文件创建

- `demo/dual_contouring/dc_renderer.h` - Renderer 类声明和 SDFBase 前向声明
- `demo/dual_contouring/dc_renderer.cpp` - Marching Squares 等值线绘制实现

---

## Task 5: Hermite 数据存储

### Hermite 数据原理

Hermite 数据存储表面交点的几何信息：
- **position**: 交点在二维空间中的精确位置
- **normal**: 表面在交点处的法向量（指向 SDF 增长方向，即形状外部）

在双轮廓线算法中，Hermite 数据用于：
1. 确定轮廓线顶点在四叉树节点内部的位置
2. 通过法线信息确保顶点正确位于表面上

### 关键实现

1. **HermiteData 结构体**:
   ```cpp
   struct HermiteData {
       vec2 position; // 交点位置
       vec2 normal;   // 表面法线
   };
   ```

2. **EdgeIntersections 结构体**:
   存储节点四条边的交点数据：
   - bottom, top, left, right 各一个 `std::vector<HermiteData>`

3. **二分法边交点查找**:
   - 采样边两端点的 SDF 值
   - 如果符号相同（无交点），返回空
   - 如果一端恰好在表面上，直接使用该点
   - 否则使用二分法迭代逼近零点（8 次迭代足够）

4. **法线计算（有限差分法）**:
   ```cpp
   vec2 ComputeNormal(const vec2& point, const SDFBase* sdf) {
       float epsilon = 0.0001f;
       float dfdx = (sdf(x+ε,y) - sdf(x-ε,y)) / (2ε);
       float dfdy = (sdf(x,y+ε) - sdf(x,y-ε)) / (2ε);
       return normalize(gradient);
   }
   ```

### 文件创建

- `demo/dual_contouring/dc_hermite.h` - Hermite 数据结构和函数声明
- `demo/dual_contouring/dc_hermite.cpp` - 二分法交点查找和法线计算实现

### 验证要点

- [ ] HermiteData 包含 position 和 normal 成员
- [ ] 圆形 SDF 的 Hermite 交点法线指向圆心外（径向）
- [ ] 方形 SDF 的 Hermite 交点法线垂直于边
- [ ] 所有表面穿越边都被正确标记