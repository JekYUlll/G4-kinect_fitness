ʵ��Ҫ��
---

�Բ����ߵ�վλ��Ҫ��
- **λ���޹���**��
    - ʹ�����λ�ü��㣬�Լ���Ϊ�ο�ϵ
    - ͨ����һ�������������С����
    - �����������������ľ���λ��
- **������Ӧ��**��
    - ��Ҫ�����ؽڽǶȼ���
    - �Ƕȼ�����������нǣ��볯���޹�
    - ���������������˹�һ������

�Ƕ����ƶȼ���
---

## 1. **ŷ�Ͼ���**�����ڼ���ؽڵ�����
```cpp
float calculateJointDistance(const CameraSpacePoint& a, const CameraSpacePoint& b) {
    float dx = a.X - b.X;
    float dy = a.Y - b.Y;
    float dz = a.Z - b.Z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}
```

## 2. **�������ƶ�**�����ڼ�����������н�
```cpp
float calculateAngle(const Vector3d& v1, const Vector3d& v2) {
    float cosAngle = v1.normalized().dot(v2.normalized());
    cosAngle = std::min(1.0f, std::max(-1.0f, cosAngle));
    return std::acos(cosAngle);
}
```

## 3. **��˹�˺���**�����ǶȲ���ӳ�䵽���ƶ�
```cpp
float calculateAngleSimilarity(float angle, float sigma = 0.8f) {
    return std::exp(-angle * angle / (2.0f * sigma * sigma));
}
```

DTW����̬ʱ��������㷨
---

## 1. **Sakoe-Chiba��Լ��**

һ��ȫ��·��Լ������������DTW·�������Խ�����Χ��һ����״�����ڡ�

```cpp
// ��������Ķ���λ�ã��������Զ��룩
double expected_j = (static_cast<double>(i) * N) / M;
// �����Լ����������Χ
j_start = std::max<size_t>(1, std::min<size_t>(N, static_cast<size_t>(std::ceil(expected_j - bandWidth))));
j_end = std::min<size_t>(N, static_cast<size_t>(std::floor(expected_j + bandWidth)));
```

���ټ��㸴�Ӷȴ�`O(MN)`��`O(M��r)`������r�Ǵ�����ֹ������Ķ��롢����ʱ��˳��ľֲ�һ���ԡ�


## 2. **��̬�滮�������**

ʹ�ö�̬�滮�������DTW����ÿ����Ԫ���ʾ�������ж�Ӧ���ֵ���С�ۻ����롣

```cpp
// ʹ��Eigen����洢DTW������
Matrix dtw = Matrix::Constant(M + 1, N + 1, std::numeric_limits<float>::infinity());
dtw(0, 0) = 0.0f;

// ��̬�滮���
for (size_t i = 1; i <= M; ++i) {
    for (size_t j = j_start; j <= j_end; ++j) {
        float cost = 1.0f - similarityMatrix(i-1, j-1);
        float min_prev = std::min({
            dtw(i-1, j),   // ����
            dtw(i, j-1),   // ɾ��
            dtw(i-1, j-1)  // ƥ��
        });
        dtw(i, j) = cost + min_prev;
    }
}
```
�Ե����Ϲ���������������沢����������Ľ⡢�������ֿ��ܵĲ��������롢ɾ����ƥ�䡣

## 3. **��һ������**

- �����һ����

```cpp
// �����������ƶȵ÷�
float similarity = 1.0f / (1.0f + dtwDistance / std::max(M, N));
```
- �����Ż���

```cpp
float postProcessSimilarity(float rawSimilarity, float sensitivity = 1.2f) {
    // �����ƶȳͷ�
    if (rawSimilarity < 0.2f) {
        return rawSimilarity * 0.3f;
    }
    
    // Sigmoidӳ�䣬�������ֶ�
    float x = (rawSimilarity - 0.6f) * 6.0f;
    float processed = 1.0f / (1.0f + std::exp(-x * sensitivity));
    
    // ��Χӳ��
    processed = 0.1f + processed * 0.8f;
    processed = std::pow(processed, 1.2f);
    
    // �����ƶ�����
    if (processed > 0.75f) {
        processed = 0.75f + (processed - 0.75f) * 0.4f;
    }
    
    return processed;
}
```

�������г��Ȳ����Ӱ�졢�����ƶ�ӳ�䵽`[0,1]`���䡢ͨ��sigmoid���������м�ֵ�����ֶȡ��Լ���ֵ���к���ĳͷ���


---

> Ϊʲô�����������㷨��

---

��̬��������
---

��̬��������(����ʵʱ�������ٶȱ仯�����������ű�׼������֡��)ʵ�����ǲ���Ҫ�ģ�ԭ�����£�

- DTW������ܴ���ʱ��߶ȵķ����Ա���
- �Ѿ����Զ���Ӧ�����ٶȵı仯
- Sakoe-Chiba����bandWidth���ṩ���㹻�������

����ʵ�ֵ��ص㣺

- ʹ���˴�Լ����DTW��ͨ��`expected_j = (static_cast<double>(i) * N) / M`�Զ��������г��Ȳ���
- bandWidth������������30%�������㹻��ʱ��Ť���ռ�
- �Ѿ�ʵ�����첽���㣬��������

��̬�������ܴ��������⣺

- ����ϵͳ������
- ��������������
- ��Ҫ����ļ��㿪����ȷ��������
- ���ܶ�ʧϸ����Ϣ

�����ֶ�
---

�����ֶβ��Ǳ�Ҫ�ģ�ԭ�����£�

- ��ǰDTWʵ�ֵ����ƣ�
    - �Ѿ��ܹ�����ͬ���ȵ�����
    - ͨ������Լ��ȷ���˾ֲ�����ĺ�����
    - ���ƶȼ����Ѿ������˹ؽ�Ȩ�غͽǶȱ仯
    - 
- ʹ�ù̶���С�Ļ������ڸ��򵥿ɿ�
    - DTW�㷨�Ѿ��ܹ��ҵ���ѵĶ��뷽ʽ
    - �첽���㱣֤��ʵʱ��