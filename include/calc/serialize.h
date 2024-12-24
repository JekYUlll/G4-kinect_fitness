#ifndef SERIALIZE_H
#define SERIALIZE_H

#include "KFcommon.h"
#include "config/config.h"
#include <vector>
#include <fstream>
#include <deque>
#include <Kinect.h>
#include <future>
#include <mutex>

namespace kfc {

// 用于序列化的关节数据结构
struct JointData {
    JointType type;                // 关节类型
    CameraSpacePoint position;     // 3D空间位置
    TrackingState trackingState;   // 跟踪状态
    
    // 序列化到文件
    void serialize(std::ofstream& out) const;
    
    // 从文件反序列化
    void deserialize(std::ifstream& in);
};

// 一帧中所有关节的数据
struct FrameData {
    INT64 timestamp;                      // 时间戳
    std::vector<JointData> joints;        // 关节数据数组
    
    // 序列化到文件
    void serialize(std::ofstream& out) const;
    
    // 从文件反序列化
    void deserialize(std::ifstream& in);
};

// 序列化一帧的骨骼数据到文件
bool SaveFrame(const std::string& filename, const FrameData& frame, bool append = false);

// 从文件读取一帧骨骼数据
bool LoadFrame(const std::string& filename, FrameData& frame);

class ActionBuffer {
private:
    std::deque<FrameData> _buffer; // 使用 std::deque 维护缓冲区
    size_t _maxFrames;               // 最大帧数

public:
    ActionBuffer(size_t maxFrames) : _maxFrames(maxFrames) {}

    // 添加动作帧到缓冲区
    inline void addFrame(const FrameData& frame) {
        if (_buffer.size() >= _maxFrames) {
            _buffer.pop_front(); // 超过最大帧数时丢弃最早的一帧
        }
        _buffer.push_back(frame);
    }

    // 获取缓冲区中的所有帧
    [[nodiscard]] inline const std::deque<FrameData>& getFrames() const {
        return _buffer;
    }

    // 清空缓冲区
    inline void clear() {
        _buffer.clear();
    }
};

class ActionTemplate {
private:
    std::unique_ptr<std::vector<kfc::FrameData>> _frames; // 使用堆存储标准动作帧

public:
    // 构造函数，直接加载文件
    ActionTemplate(const std::string& filePath);

    // 加载标准动作数据
    bool loadFromFile(const std::string& filename);

    // 把标准动作打印到日志
    void PrintData() const;

    // 获取标准动作的帧数据
    [[nodiscard]] inline const std::vector<kfc::FrameData>& getFrames() const {
        return *_frames; // 解引用智能指针
    }

    // 获取帧数量
    [[nodiscard]] inline size_t getFrameCount() const {
        return _frames->size();
    }

    // 清空数据
    inline void clear() {
        _frames->clear();
    }
};

// 全局变量
extern std::mutex templateMutex;
extern std::unique_ptr<ActionTemplate> g_actionTemplate;

// 异步加载标准动作
std::future<bool> loadStandardActionAsync(const std::string& filePath);

        

}  // namespace kf

#endif // !SERIALIZE_H

