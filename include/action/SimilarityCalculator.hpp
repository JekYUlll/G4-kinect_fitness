#pragma once

#include <future>
#include <Eigen/Dense>
#include "calc/serialize.h"

namespace kf::action {

class SimilarityCalculator {
public:
    static SimilarityCalculator& getInstance() {
        static SimilarityCalculator instance;
        return instance;
    }

    void initialize();
    void shutdown();

    float calculateSimilarity(const ActionBuffer& buffer);
    std::future<float> calculateSimilarityAsync(const ActionBuffer& buffer);

    float getCurrentSimilarity() const { return m_currentSimilarity; }
    void setCurrentSimilarity(float similarity) { m_currentSimilarity = similarity; }

private:
    SimilarityCalculator() = default;
    ~SimilarityCalculator() = default;

    SimilarityCalculator(const SimilarityCalculator&) = delete;
    SimilarityCalculator& operator=(const SimilarityCalculator&) = delete;

    float compareFrames(const FrameData& realFrame, const FrameData& templateFrame);
    float calculateJointDistance(const CameraSpacePoint& a, const CameraSpacePoint& b);
    float computeDTW(const std::vector<FrameData>& realFrames, 
                    const std::vector<FrameData>& templateFrames);

    float m_currentSimilarity = 0.0f;
    std::mutex m_mutex;
};

} // namespace kf::action 