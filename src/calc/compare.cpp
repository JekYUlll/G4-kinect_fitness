#include "../../include/calc/compare.h"
#include "../../include/log/KFLog.h"
#include <algorithm>
#include <cmath>

namespace kf {

float calculateJointDistance(const JointData& j1, const JointData& j2) {
    if (j1.trackingState == TrackingState_NotTracked || 
        j2.trackingState == TrackingState_NotTracked) {
        return std::numeric_limits<float>::max();
    }

    float dx = j1.position.X - j2.position.X;
    float dy = j1.position.Y - j2.position.Y;
    float dz = j1.position.Z - j2.position.Z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

float calculateFrameSimilarity(const FrameData& f1, const FrameData& f2) {
    if (f1.joints.size() != f2.joints.size()) {
        LOG_W("֡�Ĺؽ�������ƥ��: {} vs {}", f1.joints.size(), f2.joints.size());
        return 0.0f;
    }

    float totalDistance = 0.0f;
    size_t validJointCount = 0;

    for (size_t i = 0; i < f1.joints.size(); ++i) {
        float distance = calculateJointDistance(f1.joints[i], f2.joints[i]);
        if (distance != std::numeric_limits<float>::max()) {
            totalDistance += distance;
            ++validJointCount;
        }
    }

    if (validJointCount == 0) {
        return 0.0f;
    }

    float avgDistance = totalDistance / validJointCount;
    // ������ת��Ϊ���ƶȣ�����Խ�����ƶ�ԽС
    return std::max(0.0f, 1.0f - avgDistance);
}

float compareWithTemplate(const std::vector<FrameData>& frames, const ActionTemplate& actionTemplate) {
    const auto& templateFrames = actionTemplate.getFrames();
    if (templateFrames.empty() || frames.empty()) {
        LOG_W("ģ�������֡Ϊ��");
        return 0.0f;
    }

    // ʹ�ö�̬�滮�������ƥ��
    std::vector<std::vector<float>> dp(frames.size() + 1, 
        std::vector<float>(templateFrames.size() + 1, 0.0f));

    // ��ʼ����һ�к͵�һ��
    for (size_t i = 1; i <= frames.size(); ++i) {
        dp[i][0] = 0.0f;
    }
    for (size_t j = 1; j <= templateFrames.size(); ++j) {
        dp[0][j] = 0.0f;
    }

    // ���DP��
    for (size_t i = 1; i <= frames.size(); ++i) {
        for (size_t j = 1; j <= templateFrames.size(); ++j) {
            float similarity = calculateFrameSimilarity(frames[i-1], templateFrames[j-1]);
            dp[i][j] = std::max({dp[i-1][j], dp[i][j-1], dp[i-1][j-1] + similarity});
        }
    }

    // �������յ����ƶȵ÷�
    float maxScore = dp[frames.size()][templateFrames.size()];
    float normalizedScore = maxScore / std::min(frames.size(), templateFrames.size());

    LOG_I("�������ƶȵ÷�: {}", normalizedScore);
    return normalizedScore;
}

std::mutex g_templateMutex;
std::unique_ptr<ActionTemplate> g_actionTemplate;

} // namespace kf