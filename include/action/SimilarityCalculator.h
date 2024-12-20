#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <Eigen/Dense>
#include "../KFcommon.h"

namespace kf {

class SimilarityCalculator {
public:
    SimilarityCalculator();
    ~SimilarityCalculator();

    void startComparing(const ActionBuffer& templateBuffer);
    void stopComparing();
    void compareWithFrame(const JointData& frame);
    double getLastSimilarity() const;
    bool isComparing() const;

private:
    double calculateDTWSimilarity(const ActionBuffer& templateBuffer, const ActionBuffer& realtimeBuffer);
    double calculateJointDistance(const JointData& joint1, const JointData& joint2);
    double gaussianKernel(double distance, double sigma = 0.5);

    bool m_isComparing;
    double m_lastSimilarity;
    ActionBuffer m_realtimeBuffer;
    std::mutex m_mutex;
};

} // namespace kf 