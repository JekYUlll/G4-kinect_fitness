#pragma once

#include <string>
#include <future>
#include <mutex>
#include "calc/serialize.h"

namespace kf::action {

class ActionRecorder {
public:
    static ActionRecorder& getInstance() {
        static ActionRecorder instance;
        return instance;
    }

    void startRecording();
    void stopRecording();
    bool isRecording() const { return m_isRecording; }
    
    void recordFrame(const FrameData& frame);
    std::string getCurrentRecordPath() const { return m_currentRecordPath; }

    void startPlayback();
    void stopPlayback();
    bool isPlaying() const { return m_isPlaying; }
    
    void setPlaybackStartTime(INT64 time) { m_playbackStartTime = time; }
    INT64 getPlaybackStartTime() const { return m_playbackStartTime; }
    
    size_t getCurrentFrameIndex() const { return m_currentFrameIndex; }
    void setCurrentFrameIndex(size_t index) { m_currentFrameIndex = index; }

private:
    ActionRecorder() = default;
    ~ActionRecorder() = default;

    ActionRecorder(const ActionRecorder&) = delete;
    ActionRecorder& operator=(const ActionRecorder&) = delete;

    std::string generateRecordPath() const;

    bool m_isRecording = false;
    bool m_isPlaying = false;
    std::string m_currentRecordPath;
    INT64 m_playbackStartTime = 0;
    size_t m_currentFrameIndex = 0;
    std::mutex m_mutex;
};

} // namespace kf::action 