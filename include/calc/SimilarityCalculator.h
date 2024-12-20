#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "../action/ActionTemplate.h"

namespace kf {
namespace calc {

class SimilarityCalculator {
public:
    SimilarityCalculator();
    ~SimilarityCalculator();

    // 计算输入帧序列与模板的相似度
    float calculateSimilarity(const std::vector<FrameData>& frames);

    // 加载动作模板
    bool loadTemplate(const std::string& filename);

    // 清除当前模板
    void clearTemplate();

    // 检查是否有模板
    bool hasTemplate() const;

private:
    static std::unique_ptr<action::ActionTemplate> g_actionTemplate;
    static std::mutex g_templateMutex;
};

} // namespace calc
} // namespace kf 