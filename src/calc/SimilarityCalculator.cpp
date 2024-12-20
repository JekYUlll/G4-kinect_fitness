#include "../../include/calc/SimilarityCalculator.h"
#include "../../include/calc/compare.h"
#include "../../include/log/KFLog.h"

namespace kf {

SimilarityCalculator::SimilarityCalculator() {
}

SimilarityCalculator::~SimilarityCalculator() {
}

float SimilarityCalculator::calculateSimilarity(const std::vector<FrameData>& frames) {
    std::lock_guard<std::mutex> lock(g_templateMutex);
    if (!g_actionTemplate) {
        LOG_W("动作模板未设置");
        return 0.0f;
    }

    return compareWithTemplate(frames, *g_actionTemplate);
}

bool SimilarityCalculator::loadTemplate(const std::string& filename) {
    std::lock_guard<std::mutex> lock(g_templateMutex);
    g_actionTemplate = std::make_unique<ActionTemplate>();
    if (!g_actionTemplate->loadFromFile(filename)) {
        LOG_E("加载动作模板失败: {}", filename);
        g_actionTemplate.reset();
        return false;
    }
    LOG_I("成功加载动作模板: {}", filename);
    return true;
}

void SimilarityCalculator::clearTemplate() {
    std::lock_guard<std::mutex> lock(g_templateMutex);
    g_actionTemplate.reset();
    LOG_I("已��除动作模板");
}

bool SimilarityCalculator::hasTemplate() const {
    std::lock_guard<std::mutex> lock(g_templateMutex);
    return g_actionTemplate != nullptr;
}

} // namespace kf 