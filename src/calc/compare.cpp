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
#elif defined(USE_DTW)
    // 将输入修改为更通用的 单维向量输入，保留线性回归误差计算的逻辑。
    float calculateRegressionError(const std::vector<float>& realValues, const std::vector<float>& templateValues) {
        size_t n = realValues.size();
        if (n != templateValues.size()) return std::numeric_limits<float>::infinity();

        if (n < 2) {
            // 单点情况下，返回绝对误差
            return std::fabs(realValues[0] - templateValues[0]);
        }

        // 正常计算线性回归误差
        Eigen::MatrixXf X(n, 2);
        Eigen::VectorXf Y_real(n), Y_template(n);

        for (size_t i = 0; i < n; ++i) {
            X(i, 0) = static_cast<float>(i);
            X(i, 1) = 1.0f;
            Y_real(i) = realValues[i];
            Y_template(i) = templateValues[i];
        }

        Eigen::Vector2f beta_real = (X.transpose() * X).ldlt().solve(X.transpose() * Y_real);
        Eigen::Vector2f beta_template = (X.transpose() * X).ldlt().solve(X.transpose() * Y_template);

        float residualError = 0.0f;
        for (size_t i = 0; i < n; ++i) {
            float predicted_real = beta_real(0) * i + beta_real(1);
            float predicted_template = beta_template(0) * i + beta_template(1);
            residualError += std::pow(predicted_real - predicted_template, 2);
        }

        return std::sqrt(residualError / n);
    }

    // 比较两个动作帧
    float compareFrames(const FrameData& realFrame, const FrameData& templateFrame) {
        if (realFrame.joints.size() != templateFrame.joints.size())
            return std::numeric_limits<float>::infinity();

        float totalError = 0.0f;

        for (size_t i = 0; i < realFrame.joints.size(); ++i) {
            const auto& jointReal = realFrame.joints[i];
            const auto& jointTemplate = templateFrame.joints[i];

            std::vector<float> realX = { jointReal.position.X };
            std::vector<float> templateX = { jointTemplate.position.X };
            std::vector<float> realY = { jointReal.position.Y };
            std::vector<float> templateY = { jointTemplate.position.Y };
            std::vector<float> realZ = { jointReal.position.Z };
            std::vector<float> templateZ = { jointTemplate.position.Z };

            float errorX = calculateRegressionError(realX, templateX);
            float errorY = calculateRegressionError(realY, templateY);
            float errorZ = calculateRegressionError(realZ, templateZ);

            totalError += (errorX + errorY + errorZ);
        }

        return totalError / realFrame.joints.size();
    }
    // DTW 比较函数
    // 将缓冲区与模板动作中的每一帧调用 compareFrames 进行帧距离计算，并使用动态时间规整（DTW）算法求解最小代价路径。
    float computeDTW(const std::vector<FrameData>& realFrames, const std::vector<FrameData>& templateFrames) {
        size_t M = realFrames.size();
        size_t N = templateFrames.size();

        // 创建代价矩阵
        Eigen::MatrixXf dtwMatrix = Eigen::MatrixXf::Constant(M + 1, N + 1, std::numeric_limits<float>::infinity());
        dtwMatrix(0, 0) = 0.0f;

        // 填充 DTW 代价矩阵
        for (size_t i = 1; i <= M; ++i) {
            for (size_t j = 1; j <= N; ++j) {
                float cost = compareFrames(realFrames[i - 1], templateFrames[j - 1]);
                dtwMatrix(i, j) = cost + std::min({ dtwMatrix(i - 1, j),    // 删除操作
                                                  dtwMatrix(i, j - 1),    // 插入操作
                                                  dtwMatrix(i - 1, j - 1) }); // 匹配操作
            }
        }

        return dtwMatrix(M, N); // 返回 DTW 的最小匹配代价
    }
    // 比较动作缓冲区与标准模板
    float compareActionBuffer(const ActionBuffer& buffer, const ActionTemplate& actionTemplate) {
        // 获取缓冲区中的帧和模板动作帧
        const auto& realDeque = buffer.getFrames();
        const auto& templateFrames = actionTemplate.getFrames();

        if (realDeque.empty() || templateFrames.empty())
            return std::numeric_limits<float>::infinity();
        // 将 deque 转换为 vector
        std::vector<kf::FrameData> realFrames(realDeque.begin(), realDeque.end());
        // 调用 DTW 进行帧级匹配
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