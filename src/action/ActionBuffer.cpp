#include "../../include/action/ActionBuffer.h"
#include "../../include/log/KFLog.h"

namespace kf {

ActionBuffer::ActionBuffer(size_t capacity)
    : m_capacity(capacity) {
}

ActionBuffer::~ActionBuffer() {
    clear();
}

void ActionBuffer::addFrame(const FrameData& frame) {
    if (m_frames.size() >= m_capacity) {
        m_frames.pop_front();
    }
    m_frames.push_back(frame);
}

void ActionBuffer::clear() {
    m_frames.clear();
}

bool ActionBuffer::empty() const {
    return m_frames.empty();
}

size_t ActionBuffer::size() const {
    return m_frames.size();
}

size_t ActionBuffer::capacity() const {
    return m_capacity;
}

const std::deque<FrameData>& ActionBuffer::getFrames() const {
    return m_frames;
}

void ActionBuffer::print() const {
    LOG_I("动作缓冲区状态:");
    LOG_I("容量: {}", m_capacity);
    LOG_I("当前帧数: {}", m_frames.size());
    
    for (size_t i = 0; i < m_frames.size(); ++i) {
        LOG_I("帧 {}: {} 个关节点, 时间戳 {}", 
            i, m_frames[i].joints.size(), m_frames[i].timestamp);
    }
}

} // namespace kf 