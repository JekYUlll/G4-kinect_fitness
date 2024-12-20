#pragma once

#include <string>
#include <vector>
#include "../KFcommon.h"

namespace kf {
namespace action {

class ActionTemplate {
public:
    ActionTemplate() = default;
    ~ActionTemplate() = default;

    // 加载动作模板
    bool loadFromFile(const std::string& filename);

    // 保存动作模板
    bool saveToFile(const std::string& filename) const;

    // 添加帧数据
    void addFrame(const FrameData& frame);

    // 清空数据
    void clear();

    // 获取帧数据
    const std::vector<FrameData>& getFrames() const { return m_frames; }

    // 获取帧数量
    size_t size() const { return m_frames.size(); }

    // 检查是否为空
    bool empty() const { return m_frames.empty(); }

    // 打印信息
    void print() const;

private:
    std::vector<FrameData> m_frames;
};

} // namespace action
} // namespace kf 