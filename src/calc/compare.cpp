#include "calc/compare.h"

namespace kf {

#if defined(USE_LINEAR_REGRESSION)

    // ����һ��λ�����ݵ����Իع����
    float calculateRegressionError(const std::vector<float>& realValues, const std::vector<float>& templateValues) {
        size_t n = realValues.size();
        if (n != templateValues.size() || n < 2) return std::numeric_limits<float>::infinity();

        // ��ƾ�������������
        Eigen::MatrixXf X(n, 2); // ��ƾ��� (֡���� t, ������ 1)
        Eigen::VectorXf Y_real(n), Y_template(n);

        for (size_t i = 0; i < n; ++i) {
            X(i, 0) = static_cast<float>(i); // ֡���� (ʱ��ά��)
            X(i, 1) = 1.0f;                  // ������
            Y_real(i) = realValues[i];
            Y_template(i) = templateValues[i];
        }

        // ʹ����С���˷���������ع�ģ��
        Eigen::Vector2f beta_real = (X.transpose() * X).ldlt().solve(X.transpose() * Y_real);
        Eigen::Vector2f beta_template = (X.transpose() * X).ldlt().solve(X.transpose() * Y_template);

        // ���������ع�ģ��֮��Ĳв�ƽ����
        float residualError = 0.0f;
        for (size_t i = 0; i < n; ++i) {
            float predicted_real = beta_real(0) * i + beta_real(1);
            float predicted_template = beta_template(0) * i + beta_template(1);
            residualError += std::pow(predicted_real - predicted_template, 2);
        }

        return std::sqrt(residualError / n); // ���ؾ��������
    }

    // �Ƚ���������֡�����ƶ�
    float compareFrames(const kf::FrameData& realFrame, const kf::FrameData& templateFrame) {
        if (realFrame.joints.size() != templateFrame.joints.size()) return std::numeric_limits<float>::infinity();

        float totalError = 0.0f;

        for (size_t i = 0; i < realFrame.joints.size(); ++i) {
            const auto& jointReal = realFrame.joints[i];
            const auto& jointTemplate = templateFrame.joints[i];

            // �� X��Y��Z ���ݷֱ�洢Ϊ����
            std::vector<float> realX = { jointReal.position.X };
            std::vector<float> templateX = { jointTemplate.position.X };
            std::vector<float> realY = { jointReal.position.Y };
            std::vector<float> templateY = { jointTemplate.position.Y };
            std::vector<float> realZ = { jointReal.position.Z };
            std::vector<float> templateZ = { jointTemplate.position.Z };

            // �ֱ���� X��Y��Z �Ļع����ۼ�
            float errorX = calculateRegressionError(realX, templateX);
            float errorY = calculateRegressionError(realY, templateY);
            float errorZ = calculateRegressionError(realZ, templateZ);

            totalError += (errorX + errorY + errorZ);
        }

        return totalError / realFrame.joints.size(); // ����ƽ�����
    }

    // �Ƚ�ʵʱ�������������׼����
    float compareActionBuffer(const kf::ActionBuffer& buffer, const ActionTemplate& actionTemplate) {
        const auto& realFrames = buffer.getFrames();
        const auto& templateFrames = actionTemplate.getFrames();

        if (realFrames.size() != templateFrames.size()) return std::numeric_limits<float>::infinity();

        float totalSimilarity = 0.0f;

        for (size_t i = 0; i < realFrames.size(); ++i) {
            totalSimilarity += compareFrames(realFrames[i], templateFrames[i]);
        }

        return totalSimilarity / realFrames.size(); // ȡƽ��֡���ƶ�
    }

    // �첽�Ƚ϶���
    std::future<float> compareActionAsync(const kf::ActionBuffer& buffer) {
        return std::async(std::launch::async, [&]() {
            std::lock_guard<std::mutex> lock(templateMutex);
            return compareActionBuffer(buffer, *g_actionTemplate);
            });
    }
#elif defined(USE_DTW)
    // �������޸�Ϊ��ͨ�õ� ��ά�������룬�������Իع���������߼���
    float calculateRegressionError(const std::vector<float>& realValues, const std::vector<float>& templateValues) {
        size_t n = realValues.size();
        if (n != templateValues.size()) return std::numeric_limits<float>::infinity();

        if (n < 2) {
            // ��������£����ؾ������
            return std::fabs(realValues[0] - templateValues[0]);
        }

        // �����������Իع����
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

    // �Ƚ���������֡
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
    // DTW �ȽϺ���
    // ����������ģ�嶯���е�ÿһ֡���� compareFrames ����֡������㣬��ʹ�ö�̬ʱ�������DTW���㷨�����С����·����
    float computeDTW(const std::vector<FrameData>& realFrames, const std::vector<FrameData>& templateFrames) {
        size_t M = realFrames.size();
        size_t N = templateFrames.size();

        // �������۾���
        Eigen::MatrixXf dtwMatrix = Eigen::MatrixXf::Constant(M + 1, N + 1, std::numeric_limits<float>::infinity());
        dtwMatrix(0, 0) = 0.0f;

        // ��� DTW ���۾���
        for (size_t i = 1; i <= M; ++i) {
            for (size_t j = 1; j <= N; ++j) {
                float cost = compareFrames(realFrames[i - 1], templateFrames[j - 1]);
                dtwMatrix(i, j) = cost + std::min({ dtwMatrix(i - 1, j),    // ɾ������
                                                  dtwMatrix(i, j - 1),    // �������
                                                  dtwMatrix(i - 1, j - 1) }); // ƥ�����
            }
        }

        return dtwMatrix(M, N); // ���� DTW ����Сƥ�����
    }
    // �Ƚ϶������������׼ģ��
    float compareActionBuffer(const ActionBuffer& buffer, const ActionTemplate& actionTemplate) {
        // ��ȡ�������е�֡��ģ�嶯��֡
        const auto& realDeque = buffer.getFrames();
        const auto& templateFrames = actionTemplate.getFrames();

        if (realDeque.empty() || templateFrames.empty())
            return std::numeric_limits<float>::infinity();
        // �� deque ת��Ϊ vector
        std::vector<kf::FrameData> realFrames(realDeque.begin(), realDeque.end());
        // ���� DTW ����֡��ƥ��
        return computeDTW(realFrames, templateFrames);
    }

    // �첽�����Ƚ�
    std::future<float> compareActionAsync(const ActionBuffer& buffer) {
        return std::async(std::launch::async, [&]() {
            std::lock_guard<std::mutex> lock(templateMutex);
            return compareActionBuffer(buffer, *g_actionTemplate);
            });
    }
#else
    // ���������ؽ����ݵľ���
    float calculateJointDistance(const kf::JointData& a, const kf::JointData& b) {
        if (a.type != b.type) return std::numeric_limits<float>::infinity(); // ȷ���ؽ�����һ��

        float dx = a.position.X - b.position.X;
        float dy = a.position.Y - b.position.Y;
        float dz = a.position.Z - b.position.Z;

        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    // �Ƚ���������֡�����ƶ�
    float compareFrames(const kf::FrameData& realFrame, const kf::FrameData& templateFrame) {
        if (realFrame.joints.size() != templateFrame.joints.size()) return std::numeric_limits<float>::infinity();

        float totalDistance = 0.0f;
        size_t jointCount = realFrame.joints.size();

        for (size_t i = 0; i < jointCount; ++i) {
            totalDistance += calculateJointDistance(realFrame.joints[i], templateFrame.joints[i]);
        }

        return totalDistance / jointCount; // ȡƽ���ؽھ���
    }

    // �Ƚ�ʵʱ�������������׼����
    float compareActionBuffer(const kf::ActionBuffer& buffer, const ActionTemplate& actionTemplate) {
        const auto& realFrames = buffer.getFrames();
        const auto& templateFrames = actionTemplate.getFrames();

        if (realFrames.size() != templateFrames.size()) return std::numeric_limits<float>::infinity();

        float totalSimilarity = 0.0f;

        for (size_t i = 0; i < realFrames.size(); ++i) {
            totalSimilarity += compareFrames(realFrames[i], templateFrames[i]);
        }

        return totalSimilarity / realFrames.size(); // ȡƽ��֡���ƶ�
    }

    // �첽�Ƚ϶���
    std::future<float> compareActionAsync(const kf::ActionBuffer& buffer) {
        return std::async(std::launch::async, [&]() {
            std::lock_guard<std::mutex> lock(templateMutex);
            return compareActionBuffer(buffer, *g_actionTemplate);
            });
    }

#endif

    

}