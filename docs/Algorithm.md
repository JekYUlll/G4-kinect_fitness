实验要求
---

对参与者的站位无要求：
- **位置无关性**：
    - 使用相对位置计算，以脊柱为参考系
    - 通过归一化消除了人体大小差异
    - 不依赖于相对于相机的绝对位置
- **方向适应性**：
    - 主要依赖关节角度计算
    - 角度计算基于向量夹角，与朝向无关
    - 骨骼向量都经过了归一化处理

角度相似度计算
---

## 1. **欧氏距离**：用于计算关节点间距离
```cpp
float calculateJointDistance(const CameraSpacePoint& a, const CameraSpacePoint& b) {
    float dx = a.X - b.X;
    float dy = a.Y - b.Y;
    float dz = a.Z - b.Z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}
```

## 2. **余弦相似度**：用于计算骨骼向量夹角
```cpp
float calculateAngle(const Vector3d& v1, const Vector3d& v2) {
    float cosAngle = v1.normalized().dot(v2.normalized());
    cosAngle = std::min(1.0f, std::max(-1.0f, cosAngle));
    return std::acos(cosAngle);
}
```

## 3. **高斯核函数**：将角度差异映射到相似度
```cpp
float calculateAngleSimilarity(float angle, float sigma = 0.8f) {
    return std::exp(-angle * angle / (2.0f * sigma * sigma));
}
```

DTW（动态时间规整）算法
---

## 1. **Sakoe-Chiba带约束**

一种全局路径约束方法，限制DTW路径在主对角线周围的一个带状区域内。

```cpp
// 计算理想的对齐位置（假设线性对齐）
double expected_j = (static_cast<double>(i) * N) / M;
// 计算带约束的搜索范围
j_start = std::max<size_t>(1, std::min<size_t>(N, static_cast<size_t>(std::ceil(expected_j - bandWidth))));
j_end = std::min<size_t>(N, static_cast<size_t>(std::floor(expected_j + bandWidth)));
```

减少计算复杂度从`O(MN)`到`O(M·r)`，其中r是带宽。防止不合理的对齐、保持时间顺序的局部一致性。


## 2. **动态规划矩阵计算**

使用动态规划方法填充DTW矩阵，每个单元格表示两个序列对应部分的最小累积距离。

```cpp
// 使用Eigen矩阵存储DTW计算结果
Matrix dtw = Matrix::Constant(M + 1, N + 1, std::numeric_limits<float>::infinity());
dtw(0, 0) = 0.0f;

// 动态规划填充
for (size_t i = 1; i <= M; ++i) {
    for (size_t j = j_start; j <= j_end; ++j) {
        float cost = 1.0f - similarityMatrix(i-1, j-1);
        float min_prev = std::min({
            dtw(i-1, j),   // 插入
            dtw(i, j-1),   // 删除
            dtw(i-1, j-1)  // 匹配
        });
        dtw(i, j) = cost + min_prev;
    }
}
```
自底向上构建解决方案、保存并重用子问题的解、考虑三种可能的操作：插入、删除、匹配。

## 3. **归一化处理**

- 距离归一化：

```cpp
// 计算最终相似度得分
float similarity = 1.0f / (1.0f + dtwDistance / std::max(M, N));
```
- 后处理优化：

```cpp
float postProcessSimilarity(float rawSimilarity, float sensitivity = 1.2f) {
    // 低相似度惩罚
    if (rawSimilarity < 0.2f) {
        return rawSimilarity * 0.3f;
    }
    
    // Sigmoid映射，增加区分度
    float x = (rawSimilarity - 0.6f) * 6.0f;
    float processed = 1.0f / (1.0f + std::exp(-x * sensitivity));
    
    // 范围映射
    processed = 0.1f + processed * 0.8f;
    processed = std::pow(processed, 1.2f);
    
    // 高相似度修正
    if (processed > 0.75f) {
        processed = 0.75f + (processed - 0.75f) * 0.4f;
    }
    
    return processed;
}
```

消除序列长度差异的影响、将相似度映射到`[0,1]`区间、通过sigmoid函数增加中间值的区分度、对极端值进行合理的惩罚。


---

> 为什么不采用以下算法？

---

动态采样调整
---

动态采样调整(根据实时动作的速度变化，按比例缩放标准动作的帧数)实际上是不必要的，原因如下：

- DTW本身就能处理时间尺度的非线性变形
- 已经能自动适应动作速度的变化
- Sakoe-Chiba带（bandWidth）提供了足够的灵活性

现有实现的特点：

- 使用了带约束的DTW，通过`expected_j = (static_cast<double>(i) * N) / M`自动处理序列长度差异
- bandWidth参数（现在是30%）允许足够的时间扭曲空间
- 已经实现了异步计算，性能良好

动态采样可能带来的问题：

- 增加系统复杂性
- 可能引入采样误差
- 需要额外的计算开销来确定采样率
- 可能丢失细节信息

动作分段
---

动作分段不是必要的，原因如下：

- 当前DTW实现的优势：
    - 已经能够处理不同长度的序列
    - 通过带宽约束确保了局部对齐的合理性
    - 相似度计算已经考虑了关节权重和角度变化
    - 
- 使用固定大小的滑动窗口更简单可靠
    - DTW算法已经能够找到最佳的对齐方式
    - 异步计算保证了实时性