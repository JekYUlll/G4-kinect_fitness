#include "calc/compare.h"

namespace kf {

#ifdef USE_LINEAR_REGRESSION

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

            // 分别计算 X、Y、Z 的回归误差并累加
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