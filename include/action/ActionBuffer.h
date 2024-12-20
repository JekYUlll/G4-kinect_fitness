#pragma once

#include <vector>
#include "../KFcommon.h"

namespace kf {
namespace action {

class ActionBuffer {
public:
    explicit ActionBuffer(size_t capacity);
    ~ActionBuffer() = default;

    // 添加帧数据
    void addFrame(const FrameData& frame);

    // 清空缓冲区
    void clear();

    // 获取帧数据
    const std::vector<FrameData>& getFrames() const;

    // 获取缓冲区大小
    size_t size() const;

    // 检查缓冲区是否为空
    bool empty() const;

    // 打印缓冲区信息
    void print() const;

private:
    std::vector<FrameData> m_frames;
    size_t m_capacity;
};

} // namespace action
} // namespace kf 