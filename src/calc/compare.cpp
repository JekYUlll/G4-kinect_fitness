#include "calc/compare.h"
#include <Eigen/Dense>

namespace kf {

    // 计算两个关节位置的欧氏距离
    float calculateJointDistance(const CameraSpacePoint& a, const CameraSpacePoint& b) {
        float dx = a.X - b.X;
        float dy = a.Y - b.Y;
        float dz = a.Z - b.Z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    // 计算两帧之间的相似度
    float compareFrames(const FrameData& realFrame, const FrameData& templateFrame) {
        if (realFrame.joints.size() != templateFrame.joints.size()) {
            return 0.0f;
        }

        float totalSimilarity = 0.0f;
        size_t jointCount = realFrame.joints.size();

        for (size_t i = 0; i < jointCount; ++i) {
            const auto& jointReal = realFrame.joints[i];
            const auto& jointTemplate = templateFrame.joints[i];

            // 计算关节位置的距离
            float distance = calculateJointDistance(jointReal.position, jointTemplate.position);
            
            // 将距离转换为相似度（使用高斯核）
            float similarity = std::exp(-distance * distance / 0.5f);
            totalSimilarity += similarity;
        }

        return totalSimilarity / jointCount;
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