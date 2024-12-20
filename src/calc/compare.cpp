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

            // �ֱ��?? X��Y��Z �Ļع����ۼ�
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
    // �ؽ���Ҫ��Ȩ��
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

        // ʹ��Eigen::Vector3f����λ�ò������
        Eigen::Vector3f pos_a(a.position.X, a.position.Y, a.position.Z);
        Eigen::Vector3f pos_b(b.position.X, b.position.Y, b.position.Z);
        
        // ����ŷ�Ͼ���
        float distance = (pos_a - pos_b).norm();

        // ����sigmaֵʹ�����ƶȼ��������
        const float sigma = 1.0f; // ����sigmaֵ��ʹ�����ƶ�˥����ƽ��
        float similarity = std::exp(-distance * distance / (2.0f * sigma * sigma));

        // Ӧ��Ȩ��
        auto weightIt = JOINT_WEIGHTS.find(a.type);
        float weight = weightIt != JOINT_WEIGHTS.end() ? weightIt->second : 0.5f;
        
        return similarity * weight;
    }

    // �޸� compareFrames ����
    float compareFrames(const FrameData& realFrame, const FrameData& templateFrame) {
        if (realFrame.joints.size() != templateFrame.joints.size()) {
            return 0.0f;
        }

        const size_t jointCount = realFrame.joints.size();
        float totalSimilarity = 0.0f;
        float totalWeight = 0.0f;

        // ����ÿ���ؽڵ����ƶ�
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

    // �޸� computeDTW ����
    float computeDTW(const std::vector<FrameData>& realFrames, const std::vector<FrameData>& templateFrames) {
        const size_t M = realFrames.size();
        const size_t N = templateFrames.size();

        if (M == 0 || N == 0) {
            return 0.0f;
        }

        // ʹ��Eigen����洢DTW������
        Eigen::MatrixXf dtw = Eigen::MatrixXf::Constant(M + 1, N + 1, std::numeric_limits<float>::infinity());
        dtw(0, 0) = 0.0f;

        // Ԥ��������֡�Ե����ƶ�
        Eigen::MatrixXf similarityMatrix(M, N);
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                similarityMatrix(i, j) = compareFrames(realFrames[i], templateFrames[j]);
            }
        }

        // DTW��̬�滮����
        for (size_t i = 1; i <= M; ++i) {
            for (size_t j = 1; j <= N; ++j) {
                float cost = 1.0f - similarityMatrix(i-1, j-1);
                float min_cost = std::min({
                    dtw(i-1, j),   // ����
                    dtw(i, j-1),   // ɾ��
                    dtw(i-1, j-1)  // ƥ��
                });
                dtw(i, j) = cost + min_cost;
            }
        }

        // �����һ�������ƶȷ���
        float dtwDistance = dtw(M, N);
        float pathLength = M + N;
        
        // ʹ�øĽ������ƶ�ӳ�亯��
        float normalizedDistance = dtwDistance / pathLength;
        float similarity = 1.0f / (1.0f + normalizedDistance);
        
        // ȷ�����ƶ���[0,1]��Χ��
        return std::max(0.0f, std::min(1.0f, similarity));
    }

    // �޸� compareActionBuffer ����
    float compareActionBuffer(const ActionBuffer& buffer, const ActionTemplate& actionTemplate) {
        const auto& realDeque = buffer.getFrames();
        const auto& templateFrames = actionTemplate.getFrames();

        if (realDeque.empty() || templateFrames.empty()) {
            return 0.0f;
        }

        std::vector<FrameData> realFrames(realDeque.begin(), realDeque.end());
        
        // ���֡������̫�󣬽��м򵥵�֡��ֵ
        if (realFrames.size() > templateFrames.size() * 2 || 
            realFrames.size() * 2 < templateFrames.size()) {
            // �������������֡��ֵ�߼�
            return 0.0f;
        }
        
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