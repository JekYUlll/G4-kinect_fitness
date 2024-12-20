#include "calc/compare.h"
#include <Eigen/Dense>

namespace kf {

    // ���������ؽ�λ�õ�ŷ�Ͼ���
    float calculateJointDistance(const CameraSpacePoint& a, const CameraSpacePoint& b) {
        float dx = a.X - b.X;
        float dy = a.Y - b.Y;
        float dz = a.Z - b.Z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    // ������֮֡������ƶ�
    float compareFrames(const FrameData& realFrame, const FrameData& templateFrame) {
        if (realFrame.joints.size() != templateFrame.joints.size()) {
            return 0.0f;
        }

        float totalSimilarity = 0.0f;
        size_t jointCount = realFrame.joints.size();

        for (size_t i = 0; i < jointCount; ++i) {
            const auto& jointReal = realFrame.joints[i];
            const auto& jointTemplate = templateFrame.joints[i];

            // ����ؽ�λ�õľ���
            float distance = calculateJointDistance(jointReal.position, jointTemplate.position);
            
            // ������ת��Ϊ���ƶȣ�ʹ�ø�˹�ˣ�
            float similarity = std::exp(-distance * distance / 0.5f);
            totalSimilarity += similarity;
        }

        return totalSimilarity / jointCount;
    }

    // ʹ�� Eigen ���ٵ� DTW �㷨
    float computeDTW(const std::vector<FrameData>& realFrames, const std::vector<FrameData>& templateFrames) {
        const size_t M = realFrames.size();
        const size_t N = templateFrames.size();

        if (M == 0 || N == 0) {
            return 0.0f;
        }

        // ʹ�� Eigen ����洢 DTW ������
        Eigen::MatrixXf dtw = Eigen::MatrixXf::Zero(M + 1, N + 1);
        dtw.block(1, 0, M, 1).setConstant(std::numeric_limits<float>::infinity());
        dtw.block(0, 1, 1, N).setConstant(std::numeric_limits<float>::infinity());

        // Ԥ��������֡�Ե����ƶ�
        Eigen::MatrixXf similarityMatrix(M, N);
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                similarityMatrix(i, j) = compareFrames(realFrames[i], templateFrames[j]);
            }
        }

        // DTW ��̬�滮����
        for (size_t i = 1; i <= M; ++i) {
            for (size_t j = 1; j <= N; ++j) {
                float cost = 1.0f - similarityMatrix(i-1, j-1);
                dtw(i, j) = cost + std::min({
                    dtw(i-1, j),   // ����
                    dtw(i, j-1),   // ɾ��
                    dtw(i-1, j-1)  // ƥ��
                });
            }
        }

        // �������ƶȵ÷�
        float dtwDistance = dtw(M, N);
        float similarity = 1.0f / (1.0f + dtwDistance / std::max(M, N));
        
        LOG_D("DTW����: {:.2f}, ���г���: {} vs {}, ���ƶ�: {:.2f}%", 
            dtwDistance, M, N, similarity * 100.0f);
        
        return similarity;
    }

    // �Ƚ�ʵʱ�������������׼����
    float compareActionBuffer(const ActionBuffer& buffer, const ActionTemplate& actionTemplate) {
        const auto& realDeque = buffer.getFrames();
        const auto& templateFrames = actionTemplate.getFrames();

        if (realDeque.empty() || templateFrames.empty()) {
            LOG_E("ʵʱ������ģ�嶯��Ϊ��");
            return 0.0f;
        }

        std::vector<FrameData> realFrames(realDeque.begin(), realDeque.end());
        float similarity = computeDTW(realFrames, templateFrames);
        
        return similarity;
    }

    // �첽�����Ƚ�
    std::future<float> compareActionAsync(const ActionBuffer& buffer) {
        return std::async(std::launch::async, [&]() {
            std::lock_guard<std::mutex> lock(templateMutex);
            return compareActionBuffer(buffer, *g_actionTemplate);
        });
    }

}