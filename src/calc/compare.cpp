#include "calc/compare.h"
#include <Eigen/Dense>
#include <map>

namespace kfc {
    // 定义关节权重映射
    static const std::map<JointType, float> jointWeights = {
        // 手部关节权重最高
        {JointType_HandRight, 2.0f},
        {JointType_HandLeft, 2.0f},
        {JointType_HandTipRight, 2.0f},
        {JointType_HandTipLeft, 2.0f},
        {JointType_ThumbRight, 2.0f},
        {JointType_ThumbLeft, 2.0f},
        
        // 手臂关节次之
        {JointType_ElbowRight, 1.5f},
        {JointType_ElbowLeft, 1.5f},
        {JointType_WristRight, 1.5f},
        {JointType_WristLeft, 1.5f},
        {JointType_ShoulderRight, 1.5f},
        {JointType_ShoulderLeft, 1.5f},
        
        // 躯干关节权重适中
        {JointType_SpineShoulder, 1.2f},
        {JointType_SpineMid, 1.2f},
        {JointType_SpineBase, 1.2f},
        
        // 腿部关节权重较低
        {JointType_HipRight, 1.0f},
        {JointType_HipLeft, 1.0f},
        {JointType_KneeRight, 1.0f},
        {JointType_KneeLeft, 1.0f},
        {JointType_AnkleRight, 0.8f},
        {JointType_AnkleLeft, 0.8f},
        {JointType_FootRight, 0.8f},
        {JointType_FootLeft, 0.8f},
        
        // 头部关节
        {JointType_Head, 2.0f},
        {JointType_Neck, 2.0f}
    };

    // 定义骨骼连接关系，用于计算相对角度
    static const std::vector<std::pair<JointType, JointType>> boneConnections = {
        // 躯干
        {JointType_SpineBase, JointType_SpineMid},
        {JointType_SpineMid, JointType_SpineShoulder},
        {JointType_SpineShoulder, JointType_Neck},
        {JointType_Neck, JointType_Head},
        // 右臂
        {JointType_SpineShoulder, JointType_ShoulderRight},
        {JointType_ShoulderRight, JointType_ElbowRight},
        {JointType_ElbowRight, JointType_WristRight},
        {JointType_WristRight, JointType_HandRight},
        // 左臂
        {JointType_SpineShoulder, JointType_ShoulderLeft},
        {JointType_ShoulderLeft, JointType_ElbowLeft},
        {JointType_ElbowLeft, JointType_WristLeft},
        {JointType_WristLeft, JointType_HandLeft}
    };

    // 原始的欧氏距离计算方法（保留作为参考）
    static float calculateJointDistance(const CameraSpacePoint& a, const CameraSpacePoint& b) {
        float dx = a.X - b.X;
        float dy = a.Y - b.Y;
        float dz = a.Z - b.Z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    // 使用 Eigen 优化向量计算
    using Vector3d = Eigen::Vector3f;

    static Vector3d toEigenVector(const CameraSpacePoint& p) {
        return Vector3d(p.X, p.Y, p.Z);
    }

    // 计算归一化的骨骼向量
    static Vector3d getNormalizedBoneVector(const CameraSpacePoint& joint1, 
                                          const CameraSpacePoint& joint2) {
        Vector3d boneVector = toEigenVector(joint2) - toEigenVector(joint1);
        float length = boneVector.norm();
        return length > 1e-6f ? boneVector.normalized() : Vector3d::Zero();
    }

    float calculateAngle(const Vector3d& v1, const Vector3d& v2) {
        float cosAngle = v1.normalized().dot(v2.normalized());
        cosAngle = std::min(1.0f, std::max(-1.0f, cosAngle));
        return std::acos(cosAngle);
    }

    // 计算角度相似度，使用可配置的高斯核带宽
    float calculateAngleSimilarity(float angle, float sigma = 0.8f) {
        return std::exp(-angle * angle / (2.0f * sigma * sigma));
    }

    Vector3d calculateRelativePosition(const CameraSpacePoint& joint, 
                                    const CameraSpacePoint& spineMid, 
                                    const CameraSpacePoint& spineBase) {
        Vector3d spineVector = toEigenVector(spineMid) - toEigenVector(spineBase);
        float spineLength = spineVector.norm();
        if (spineLength < 1e-6f) return Vector3d::Zero();
        
        Vector3d relativePos = toEigenVector(joint) - toEigenVector(spineBase);
        return relativePos / spineLength;  // 使用脊柱长度归一化
    }

    // 计算两帧之间的相似度
    float compareFrames(const FrameData& realFrame, const FrameData& templateFrame) {
        if (realFrame.joints.size() != templateFrame.joints.size()) {
            return 0.0f;
        }

        float totalWeightedSimilarity = 0.0f;
        float totalWeight = 0.0f;

        // 1. 计算角度相似度
        for (const auto& bone : boneConnections) {
            const auto& joint1Real = realFrame.joints[bone.first];
            const auto& joint2Real = realFrame.joints[bone.second];
            const auto& joint1Template = templateFrame.joints[bone.first];
            const auto& joint2Template = templateFrame.joints[bone.second];

            if (joint1Real.trackingState == TrackingState_Tracked && 
                joint2Real.trackingState == TrackingState_Tracked &&
                joint1Template.trackingState == TrackingState_Tracked && 
                joint2Template.trackingState == TrackingState_Tracked) {
                
                // 使用归一化的骨骼向量计算角度
                Vector3d boneVectorReal = getNormalizedBoneVector(joint1Real.position, joint2Real.position);
                Vector3d boneVectorTemplate = getNormalizedBoneVector(joint1Template.position, joint2Template.position);
                
                float angle = calculateAngle(boneVectorReal, boneVectorTemplate);
                
                // 根据骨骼类型选择不同的高斯核带宽
                float sigma = 0.8f;  // 默认带宽
                if (bone.first == JointType_SpineBase || bone.first == JointType_SpineMid) {
                    sigma = 0.6f;  // 躯干部分使用更小的带宽，要求更精确
                } else if (bone.first == JointType_HandRight || bone.first == JointType_HandLeft) {
                    sigma = 1.0f;  // 手部使用更大的带宽，允许更大的变化
                }
                
                float angleSimilarity = calculateAngleSimilarity(angle, sigma);

                float weight = 1.0f;
                auto it = jointWeights.find(bone.second);
                if (it != jointWeights.end()) {
                    weight = it->second;
                }

                totalWeightedSimilarity += angleSimilarity * weight;
                totalWeight += weight;
            }
        }

        // 2. 计算相对位置相似度
        const auto& spineBaseReal = realFrame.joints[JointType_SpineBase];
        const auto& spineMidReal = realFrame.joints[JointType_SpineMid];
        const auto& spineBaseTemplate = templateFrame.joints[JointType_SpineBase];
        const auto& spineMidTemplate = templateFrame.joints[JointType_SpineMid];

        if (spineBaseReal.trackingState == TrackingState_Tracked && 
            spineMidReal.trackingState == TrackingState_Tracked &&
            spineBaseTemplate.trackingState == TrackingState_Tracked && 
            spineMidTemplate.trackingState == TrackingState_Tracked) {
            
            // 使用 Eigen 的批量运算
            for (size_t i = 0; i < realFrame.joints.size(); ++i) {
                const auto& jointReal = realFrame.joints[i];
                const auto& jointTemplate = templateFrame.joints[i];

                if (jointReal.trackingState == TrackingState_Tracked && 
                    jointTemplate.trackingState == TrackingState_Tracked) {
                    
                    Vector3d relPosReal = calculateRelativePosition(
                        jointReal.position, spineMidReal.position, spineBaseReal.position);
                    Vector3d relPosTemplate = calculateRelativePosition(
                        jointTemplate.position, spineMidTemplate.position, spineBaseTemplate.position);

                    float posDistance = (relPosReal - relPosTemplate).norm();
                    float posSimilarity = std::exp(-posDistance * posDistance / 0.5f);

                    float weight = 1.0f;
                    auto it = jointWeights.find(static_cast<JointType>(i));
                    if (it != jointWeights.end()) {
                        weight = it->second;
                    }

                    totalWeightedSimilarity += posSimilarity * weight;
                    totalWeight += weight;
                }
            }
        }

        if (totalWeight < 0.001f) {
            return 0.0f;
        }

        return totalWeightedSimilarity / totalWeight;
    }

    // 相似度后处理函数，使结果分布更加均匀
    float postProcessSimilarity(float rawSimilarity, float sensitivity = 1.2f) {
        // 如果原始相似度太低，直接返回更低的值
        if (rawSimilarity < 0.2f) {
            return rawSimilarity * 0.3f;  // 进一步降低很差的匹配
        }
        
        // 将相似度从[0.2,1]映射到[-3,3]的范围内
        float x = (rawSimilarity - 0.6f) * 6.0f;
        
        // 使用sigmoid函数进行S型映射
        float processed = 1.0f / (1.0f + std::exp(-x * sensitivity));
        
        // 将结果映射到[0.1,0.9]区间，保持合理的分布
        processed = 0.1f + processed * 0.8f;
        
        // 对结果进行指数变换，增加区分度
        processed = std::pow(processed, 1.2f);
        
        // 对高相似度进行额外的惩罚
        if (processed > 0.75f) {
            processed = 0.75f + (processed - 0.75f) * 0.4f;
        }
        
        LOG_D("Raw similarity: {:.2f}%, Processed: {:.2f}%", 
            rawSimilarity * 100.0f, processed * 100.0f);
            
        return processed;
    }

    // 使用 Eigen 加速的 DTW 算法
    float computeDTW(const std::vector<FrameData>& realFrames, const std::vector<FrameData>& templateFrames, size_t bandWidth = 0) {
        const size_t M = realFrames.size();
        const size_t N = templateFrames.size();

        if (M == 0 || N == 0) {
            return 0.0f;
        }

        // 如果没有指定带宽，设置初始带宽为序列长度的20%
        if (bandWidth == 0) {
            bandWidth = std::max<size_t>(
                static_cast<size_t>(std::min<size_t>(M, N) * 0.2f), 
                static_cast<size_t>(10)
            );
        }
        LOG_D("DTW band width: {}", bandWidth);

        // 使用 Eigen 矩阵存储 DTW 计算结果，使用 RowMajor 布局以提高缓存命中率
        using Matrix = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
        Matrix dtw = Matrix::Constant(M + 1, N + 1, std::numeric_limits<float>::infinity());
        dtw(0, 0) = 0.0f;

        // 预计算所有帧对的相似度，同样使用 RowMajor 布局
        Matrix similarityMatrix = Matrix::Zero(M, N);
        #pragma omp parallel for collapse(2) if(M * N > 1000)
        for (int i = 0; i < static_cast<int>(M); ++i) {
            for (int j = 0; j < static_cast<int>(N); ++j) {
                similarityMatrix(i, j) = compareFrames(realFrames[i], templateFrames[j]);
            }
        }

        // DTW 动态规划计算，带Sakoe-Chiba带约束
        for (size_t i = 1; i <= M; ++i) {
            // 计算当前行的带约束范围
            size_t j_start = 1;
            size_t j_end = N;
            
            // 应用Sakoe-Chiba带约束
            if (bandWidth < N) {
                // 计算理想的对齐位置（假设线性对齐）
                double expected_j = (static_cast<double>(i) * N) / M;
                j_start = std::max<size_t>(1, std::min<size_t>(N, static_cast<size_t>(std::ceil(expected_j - bandWidth))));
                j_end = std::min<size_t>(N, static_cast<size_t>(std::floor(expected_j + bandWidth)));
            }

            // 使用向量化操作计算当前行
            for (size_t j = j_start; j <= j_end; ++j) {
                float cost = 1.0f - similarityMatrix(i-1, j-1);
                float min_prev = std::min({
                    dtw(i-1, j),   // 插入
                    dtw(i, j-1),   // 删除
                    dtw(i-1, j-1)  // 匹配
                });
                
                if (std::isinf(min_prev)) {
                    dtw(i, j) = cost * static_cast<float>(i);
                } else {
                    dtw(i, j) = cost + min_prev;
                }
            }
        }

        // 计算相似度得分
        float dtwDistance = dtw(M, N);
        if (std::isinf(dtwDistance)) {
            // 如果带宽已经接近序列长度的一半，尝试使用最近的有效值
            if (bandWidth >= std::min<size_t>(M, N) / 3) {
                float minDistance = std::numeric_limits<float>::infinity();
                
                // 在最后几行和列中寻找最小的有效距离
                Eigen::Matrix<float, 5, 5> endRegion = dtw.block(M-4, N-4, 5, 5);
                float validMin = endRegion.array().isFinite().select(endRegion, std::numeric_limits<float>::infinity()).minCoeff();
                
                if (!std::isinf(validMin)) {
                    dtwDistance = validMin * (static_cast<float>(M + N) / static_cast<float>(M + N - 5));
                    LOG_W("Using approximate DTW distance: {:.2f}", dtwDistance);
                } else if (bandWidth >= std::min<size_t>(M, N) / 2) {
                    LOG_W("DTW path not found even with wide band, sequences might be too different");
                    return 0.0f;
                } else {
                    // 增加带宽并重试
                    LOG_W("DTW path not found within the band width {}, increasing to {}", bandWidth, bandWidth * 2);
                    return computeDTW(realFrames, templateFrames, bandWidth * 2);
                }
            } else {
                // 增加带宽并重试
                LOG_W("DTW path not found within the band width {}, increasing to {}", bandWidth, bandWidth * 2);
                return computeDTW(realFrames, templateFrames, bandWidth * 2);
            }
        }
        
        float similarity = 1.0f / (1.0f + dtwDistance / std::max(M, N));
        
        LOG_D("DTW distance: {:.2f}, sequence length: {} vs {}, raw similarity: {:.2f}%", 
            dtwDistance, M, N, similarity * 100.0f);
        
        // 应用后处理
        return postProcessSimilarity(similarity);
    }

    // 比较实时动作缓冲区与标准动作
    float compareActionBuffer(const ActionBuffer& buffer, const ActionTemplate& actionTemplate) {
        const auto& realDeque = buffer.getFrames();
        const auto& templateFrames = actionTemplate.getFrames();

        if (realDeque.empty() || templateFrames.empty()) {
            LOG_E("Real action or template action is empty");
            return 0.0f;
        }

        std::vector<FrameData> realFrames(realDeque.begin(), realDeque.end());
        float similarity = computeDTW(realFrames, templateFrames);
        
        return similarity;
    }

    // 异步动作比较
    std::future<float> compareActionAsync(const ActionBuffer& buffer) {
        return std::async(std::launch::async, [&]() {
            std::lock_guard<std::mutex> lock(templateMutex);
            return compareActionBuffer(buffer, *g_actionTemplate);
        });
    }

}