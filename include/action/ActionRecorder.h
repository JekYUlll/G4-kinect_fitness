#pragma once

#include "../KFcommon.h"
#include "ActionBuffer.h"

namespace kf {
namespace action {

class ActionRecorder {
public:
    explicit ActionRecorder(size_t bufferCapacity = ACTION_BUFFER_SIZE);
    ~ActionRecorder();

    // 开始录制
    void startRecording();

    // 停止录制
    void stopRecording();

    // 添加帧数据
    void addFrame(const FrameData& frame);

    // 检查是否正在录制
    bool isRecording() const { return m_isRecording; }

    // 获取缓冲区
    const ActionBuffer& getBuffer() const { return m_buffer; }

    // 打印状态信息
    void print() const;

private:
    ActionBuffer m_buffer;
    bool m_isRecording;
};

} // namespace action
} // namespace kf 