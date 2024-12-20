#include "calc/compare.h"

namespace kf {

#if defined(USE_LINEAR_REGRESSION)

    // 计算一组位置数据的线性回归误差
    float calculateRegressionError(const std::vector<float>& realValues, const std::vector<float>& templateValues) {
        size_t n = realValues.size();
        if (n != templateValues.size() || n < 2) return std::numeric_limits<float>::infinity();

        // 设计矩阵和因变量向量
        Eigen::MatrixXf X(n, 2); // 设计矩阵 (帧索引 t, 常数项 1)
        Eigen::VectorXf Y_real(n), Y_template(n);

        for (size_t i = 0; i < n; ++i) {
            X(i, 0) = static_cast<float>(i); // 帧索引 (时间维度)
            X(i, 1) = 1.0f;                  // 常数项
            Y_real(i) = realValues[i];
            Y_template(i) = templateValues[i];
        }

        // 使用最小二乘法拟合两个回归模型
        Eigen::Vector2f beta_real = (X.transpose() * X).ldlt().solve(X.transpose() * Y_real);
        Eigen::Vector2f beta_template = (X.transpose() * X).ldlt().solve(X.transpose() * Y_template);

        // 计算两个回归模型之间的残差平方和
        float residualError = 0.0f;
        for (size_t i = 0; i < n; ++i) {
            float predicted_real = beta_real(0) * i + beta_real(1);
            float predicted_template = beta_template(0) * i + beta_template(1);
            residualError += std::pow(predicted_real - predicted_template, 2);
        }

        return std::sqrt(residualError / n); // 返回均方根误差
    }

    // 比较两个动作帧的相似度
    float compareFrames(const kf::FrameData& realFrame, const kf::FrameData& templateFrame) {
        if (realFrame.joints.size() != templateFrame.joints.size()) return std::numeric_limits<float>::infinity();

        float totalError = 0.0f;

        for (size_t i = 0; i < realFrame.joints.size(); ++i) {
            const auto& jointReal = realFrame.joints[i];
            const auto& jointTemplate = templateFrame.joints[i];

            // 将 X、Y、Z 数据分别存储为向量
            std::vector<float> realX = { jointReal.position.X };
            std::vector<float> templateX = { jointTemplate.position.X };
            std::vector<float> realY = { jointReal.position.Y };
            std::vector<float> templateY = { jointTemplate.position.Y };
            std::vector<float> realZ = { jointReal.position.Z };
            std::vector<float> templateZ = { jointTemplate.position.Z };

            // 分别计?? X、Y、Z 的回归误差并累加
            float errorX = calculateRegressionError(realX, templateX);
            float errorY = calculateRegressionError(realY, templateY);
            float errorZ = calculateRegressionError(realZ, templateZ);

            totalError += (errorX + errorY + errorZ);
        }

        return totalError / realFrame.joints.size(); // 返回平均误差
    }

    // 比较实时动作缓冲区与标准动作
    float compareActionBuffer(const kf::ActionBuffer& buffer, const ActionTemplate& actionTemplate) {
        const auto& realFrames = buffer.getFrames();
        const auto& templateFrames = actionTemplate.getFrames();

        if (realFrames.size() != templateFrames.size()) return std::numeric_limits<float>::infinity();

        float totalSimilarity = 0.0f;

        for (size_t i = 0; i < realFrames.size(); ++i) {
            totalSimilarity += compareFrames(realFrames[i], templateFrames[i]);
        }

        return totalSimilarity / realFrames.size(); // 取平均帧相似度
    }

    // 异步比较动作
    std::future<float> compareActionAsync(const kf::ActionBuffer& buffer) {
        return std::async(std::launch::async, [&]() {
            std::lock_guard<std::mutex> lock(templateMutex);
            return compareActionBuffer(buffer, *g_actionTemplate);
            });
    }
#elif defined(USE_DTW)
    // 关节重要性权重
    const std::unordered_map<JointType, float> JOINT_WEIGHTS = {
        {JointType_SpineBase, 0.5f},
        {JointType_SpineMid, 0.5f},
        {JointType_SpineShoulder, 0.8f},
        {JointType_Neck, 0.3f},
        {JointType_Head, 0.3f},
        {JointType_ShoulderLeft, 1.0f},
        {JointType_ElbowLeft, 1.0f},
        {JointType_WristLeft, 1.0f},
        {JointType_HandLeft, 0.8f},
        {JointType_ShoulderRight, 1.0f},
        {JointType_ElbowRight, 1.0f},
        {JointType_WristRight, 1.0f},
        {JointType_HandRight, 0.8f},
        {JointType_HipLeft, 0.8f},
        {JointType_KneeLeft, 1.0f},
        {JointType_AnkleLeft, 0.8f},
        {JointType_FootLeft, 0.5f},
        {JointType_HipRight, 0.8f},
        {JointType_KneeRight, 1.0f},
        {JointType_AnkleRight, 0.8f},
        {JointType_FootRight, 0.5f}
    };

    float calculateJointSimilarity(const JointData& a, const JointData& b) {
        if (a.type != b.type || 
            a.trackingState == TrackingState_NotTracked || 
            b.trackingState == TrackingState_NotTracked) {
            return 0.0f;
        }

        // 使用Eigen::Vector3f进行位置差异计算
        Eigen::Vector3f pos_a(a.position.X, a.position.Y, a.position.Z);
        Eigen::Vector3f pos_b(b.position.X, b.position.Y, b.position.Z);
        
        // 计算欧氏距离
        float distance = (pos_a - pos_b).norm();

        // 调整sigma值使得相似度计算更合理
        const float sigma = 1.0f; // 增大sigma值，使得相似度衰减更平缓
        float similarity = std::exp(-distance * distance / (2.0f * sigma * sigma));

        // 应用权重
        auto weightIt = JOINT_WEIGHTS.find(a.type);
        float weight = weightIt != JOINT_WEIGHTS.end() ? weightIt->second : 0.5f;
        
        return similarity * weight;
    }

    // 修改 compareFrames 函数
    float compareFrames(const FrameData& realFrame, const FrameData& templateFrame) {
        if (realFrame.joints.size() != templateFrame.joints.size()) {
            return 0.0f;
        }

        const size_t jointCount = realFrame.joints.size();
        float totalSimilarity = 0.0f;
        float totalWeight = 0.0f;

        // 计算每个关节的相似度
        for (size_t i = 0; i < jointCount; ++i) {
            const auto& jointReal = realFrame.joints[i];
            const auto& jointTemplate = templateFrame.joints[i];
            
            auto weightIt = JOINT_WEIGHTS.find(jointReal.type);
            float weight = weightIt != JOINT_WEIGHTS.end() ? weightIt->second : 0.5f;
            
            float similarity = calculateJointSimilarity(jointReal, jointTemplate);
            totalSimilarity += similarity * weight;
            totalWeight += weight;
        }

        return totalWeight > 0 ? (totalSimilarity / totalWeight) : 0.0f;
    }

    // 修改 computeDTW 函数
    float computeDTW(const std::vector<FrameData>& realFrames, const std::vector<FrameData>& templateFrames) {
        const size_t M = realFrames.size();
        const size_t N = templateFrames.size();

        if (M == 0 || N == 0) {
            return 0.0f;
        }

        // 使用Eigen矩阵存储DTW计算结果
        Eigen::MatrixXf dtw = Eigen::MatrixXf::Constant(M + 1, N + 1, std::numeric_limits<float>::infinity());
        dtw(0, 0) = 0.0f;

        // 预计算所有帧对的相似度
        Eigen::MatrixXf similarityMatrix(M, N);
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                similarityMatrix(i, j) = compareFrames(realFrames[i], templateFrames[j]);
            }
        }

        // DTW动态规划计算
        for (size_t i = 1; i <= M; ++i) {
            for (size_t j = 1; j <= N; ++j) {
                float cost = 1.0f - similarityMatrix(i-1, j-1);
                float min_cost = std::min({
                    dtw(i-1, j),   // 插入
                    dtw(i, j-1),   // 删除
                    dtw(i-1, j-1)  // 匹配
                });
                dtw(i, j) = cost + min_cost;
            }
        }

        // 计算归一化的相似度分数
        float dtwDistance = dtw(M, N);
        float pathLength = M + N;
        
        // 使用改进的相似度映射函数
        float normalizedDistance = dtwDistance / pathLength;
        float similarity = 1.0f / (1.0f + normalizedDistance);
        
        // 确保相似度在[0,1]范围内
        return std::max(0.0f, std::min(1.0f, similarity));
    }

    // 修改 compareActionBuffer 函数
    float compareActionBuffer(const ActionBuffer& buffer, const ActionTemplate& actionTemplate) {
        const auto& realDeque = buffer.getFrames();
        const auto& templateFrames = actionTemplate.getFrames();

        if (realDeque.empty() || templateFrames.empty()) {
            return 0.0f;
        }

        std::vector<FrameData> realFrames(realDeque.begin(), realDeque.end());
        
        // 如果帧数差异太大，进行简单的帧插值
        if (realFrames.size() > templateFrames.size() * 2 || 
            realFrames.size() * 2 < templateFrames.size()) {
            // 可以在这里添加帧插值逻辑
            return 0.0f;
        }
        
        return computeDTW(realFrames, templateFrames);
    }

    // 异步动作比较
    std::future<float> compareActionAsync(const ActionBuffer& buffer) {
        return std::async(std::launch::async, [&]() {
            std::lock_guard<std::mutex> lock(templateMutex);
            return compareActionBuffer(buffer, *g_actionTemplate);
            });
    }
#else
    // 计算两个关节数据的距离
    float calculateJointDistance(const kf::JointData& a, const kf::JointData& b) {
        if (a.type != b.type) return std::numeric_limits<float>::infinity(); // 确保关节类型一致

        float dx = a.position.X - b.position.X;
        float dy = a.position.Y - b.position.Y;
        float dz = a.position.Z - b.position.Z;

        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    // 比较两个动作帧的相似度
    float compareFrames(const kf::FrameData& realFrame, const kf::FrameData& templateFrame) {
        if (realFrame.joints.size() != templateFrame.joints.size()) return std::numeric_limits<float>::infinity();

        float totalDistance = 0.0f;
        size_t jointCount = realFrame.joints.size();

        for (size_t i = 0; i < jointCount; ++i) {
            totalDistance += calculateJointDistance(realFrame.joints[i], templateFrame.joints[i]);
        }

        return totalDistance / jointCount; // 取平均关节距离
    }

    // 比较实时动作缓冲区与标准动作
    float compareActionBuffer(const kf::ActionBuffer& buffer, const ActionTemplate& actionTemplate) {
        const auto& realFrames = buffer.getFrames();
        const auto& templateFrames = actionTemplate.getFrames();

        if (realFrames.size() != templateFrames.size()) return std::numeric_limits<float>::infinity();

        float totalSimilarity = 0.0f;

        for (size_t i = 0; i < realFrames.size(); ++i) {
            totalSimilarity += compareFrames(realFrames[i], templateFrames[i]);
        }

        return totalSimilarity / realFrames.size(); // 取平均帧相似度
    }

    // 异步比较动作
    std::future<float> compareActionAsync(const kf::ActionBuffer& buffer) {
        return std::async(std::launch::async, [&]() {
            std::lock_guard<std::mutex> lock(templateMutex);
            return compareActionBuffer(buffer, *g_actionTemplate);
            });
    }

#endif

    

}