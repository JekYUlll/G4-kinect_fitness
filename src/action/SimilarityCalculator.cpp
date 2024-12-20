#include "action/SimilarityCalculator.hpp"
#include "calc/compare.h"
#include "log/KFLog.h"

namespace kf::action {

SimilarityCalculator::SimilarityCalculator()
    : m_lastSimilarity(0.0f)
    , m_isComparing(false)
{
}

float SimilarityCalculator::calculateSimilarity(const ActionBuffer& realtimeAction) {
    if (realtimeAction.size() == 0) {
        LOG_E("Real-time action buffer is empty");
        return 0.0f;
    }

    m_isComparing = true;
    m_lastSimilarity = compareWithTemplate(realtimeAction);
    m_isComparing = false;

    return m_lastSimilarity;
}

float SimilarityCalculator::getLastSimilarity() const {
    return m_lastSimilarity;
}

bool SimilarityCalculator::isComparing() const {
    return m_isComparing;
}

} // namespace kf 