#include "../../include/action/ActionRecorder.h"
#include "../../include/log/KFLog.h"

namespace kf::action {

ActionRecorder::ActionRecorder(size_t bufferCapacity)
    : m_buffer(bufferCapacity)
    , m_isRecording(false) {
}

ActionRecorder::~ActionRecorder() {
    stopRecording();
}

void ActionRecorder::startRecording() {
    if (!m_isRecording) {
        m_isRecording = true;
        m_buffer.clear();
        LOG_I("开始录制动作");
    }
}

void ActionRecorder::stopRecording() {
    if (m_isRecording) {
        m_isRecording = false;
        LOG_I("停止录制动作，共记录 {} 帧", m_buffer.size());
    }
}

void ActionRecorder::addFrame(const FrameData& frame) {
    if (m_isRecording) {
        m_buffer.addFrame(frame);
    }
}

bool ActionRecorder::isRecording() const {
    return m_isRecording;
}

const ActionBuffer& ActionRecorder::getBuffer() const {
    return m_buffer;
}

void ActionRecorder::print() const {
    LOG_I("动作记录器状态:");
    LOG_I("正在录制: {}", m_isRecording ? "是" : "否");
    m_buffer.print();
}

} // namespace kf