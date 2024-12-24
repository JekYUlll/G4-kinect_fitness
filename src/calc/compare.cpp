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
    float calculateJointDistance(const CameraSpacePoint& a, const CameraSpacePoint& b) {
        float dx = a.X - b.X;
        float dy = a.Y - b.Y;
        float dz = a.Z - b.Z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    // 使用 Eigen 优化的向量计算
    using Vector3d = Eigen::Vector3f;

    Vector3d toEigenVector(const CameraSpacePoint& p) {
        return Vector3d(p.X, p.Y, p.Z);
    }

    float calculateAngle(const Vector3d& v1, const Vector3d& v2) {
        float cosAngle = v1.normalized().dot(v2.normalized());
        cosAngle = std::min(1.0f, std::max(-1.0f, cosAngle));
        return std::acos(cosAngle);
    }

    Vector3d calculateRelativePosition(const CameraSpacePoint& joint, 
                                    const CameraSpacePoint& spineMid, 
                                    const CameraSpacePoint& spineBase) {
        Vector3d spineVector = toEigenVector(spineMid) - toEigenVector(spineBase);
        float spineLength = spineVector.norm();
        if (spineLength < 1e-6f) return Vector3d::Zero();
        
        Vector3d relativePos = toEigenVector(joint) - toEigenVector(spineBase);
        return relativePos / spineLength;
    }

    // 计算两帧之间的相似度
    float compareFrames(const FrameData& realFrame, const FrameData& templateFrame) {
        if (realFrame.joints.size() != templateFrame.joints.size()) {
            return 0.0f;
        }

        /* 原始的基于欧氏距离的比较方法（注释保留）
        float totalWeightedSimilarity = 0.0f;
        float totalWeight = 0.0f;
        size_t jointCount = realFrame.joints.size();

        for (size_t i = 0; i < jointCount; ++i) {
            const auto& jointReal = realFrame.joints[i];
            const auto& jointTemplate = templateFrame.joints[i];

            // 获取关节权重，如果没有定义则使用默认权重1.0
            float weight = 1.0f;
            auto it = jointWeights.find(jointReal.type);
            if (it != jointWeights.end()) {
                weight = it->second;
            }

            // 只有当两个关节都被追踪时才计算相似度
            if (jointReal.trackingState == TrackingState_Tracked && 
                jointTemplate.trackingState == TrackingState_Tracked) {
                float distance = calculateJointDistance(jointReal.position, jointTemplate.position);
                float similarity = std::exp(-distance * distance / 0.5f);
                totalWeightedSimilarity += similarity * weight;
                totalWeight += weight;
            }
        }
        */

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
                
                Vector3d boneVectorReal = toEigenVector(joint2Real.position) - toEigenVector(joint1Real.position);
                Vector3d boneVectorTemplate = toEigenVector(joint2Template.position) - toEigenVector(joint1Template.position);
                
                float angle = calculateAngle(boneVectorReal, boneVectorTemplate);
                float angleSimilarity = std::exp(-angle * angle / 1.0f);

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

    // 使用 Eigen 加速的 DTW 算法
    float computeDTW(const std::vector<FrameData>& realFrames, const std::vector<FrameData>& templateFrames) {
        const size_t M = realFrames.size();
        const size_t N = templateFrames.size();

        if (M == 0 || N == 0) {
            return 0.0f;
        }

        // 使用 Eigen 矩阵存储 DTW 计算结果
        Eigen::MatrixXf dtw = Eigen::MatrixXf::Zero(M + 1, N + 1);
        dtw.block(1, 0, M, 1).setConstant(std::numeric_limits<float>::infinity());
        dtw.block(0, 1, 1, N).setConstant(std::numeric_limits<float>::infinity());

        // 预计算所有帧对的相似度
        Eigen::MatrixXf similarityMatrix(M, N);
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                similarityMatrix(i, j) = compareFrames(realFrames[i], templateFrames[j]);
            }
        }

        // DTW 动态规划计算
        for (size_t i = 1; i <= M; ++i) {
            for (size_t j = 1; j <= N; ++j) {
                float cost = 1.0f - similarityMatrix(i-1, j-1);
                dtw(i, j) = cost + std::min({
                    dtw(i-1, j),   // 插入
                    dtw(i, j-1),   // 删除
                    dtw(i-1, j-1)  // 匹配
                });
            }
        }

        // 计算相似度得分
        float dtwDistance = dtw(M, N);
        float similarity = 1.0f / (1.0f + dtwDistance / std::max(M, N));
        
        LOG_D("DTW距离: {:.2f}, 序列长度: {} vs {}, 相似度: {:.2f}%", 
            dtwDistance, M, N, similarity * 100.0f);
        
        return similarity;
    }

    // 比较实时动作缓冲区与标准动作
    float compareActionBuffer(const ActionBuffer& buffer, const ActionTemplate& actionTemplate) {
        const auto& realDeque = buffer.getFrames();
        const auto& templateFrames = actionTemplate.getFrames();

        if (realDeque.empty() || templateFrames.empty()) {
            LOG_E("实时动作或模板动作为空");
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