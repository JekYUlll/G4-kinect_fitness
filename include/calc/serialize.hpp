#ifndef SERIALIZE_H
#define SERIALIZE_H

#include "KFcommon.h"
#include <vector>
#include <fstream>
#include <Kinect.h>

namespace kf {

// 用于序列化的关节数据结构
struct JointData {
    JointType type;                // 关节类型
    CameraSpacePoint position;     // 3D空间位置
    TrackingState trackingState;   // 跟踪状态
    
    // 序列化到文件
    void serialize(std::ofstream& out) const {
        out.write(reinterpret_cast<const char*>(&type), sizeof(type));
        out.write(reinterpret_cast<const char*>(&position), sizeof(position));
        out.write(reinterpret_cast<const char*>(&trackingState), sizeof(trackingState));
    }
    
    // 从文件反序列化
    void deserialize(std::ifstream& in) {
        in.read(reinterpret_cast<char*>(&type), sizeof(type));
        in.read(reinterpret_cast<char*>(&position), sizeof(position));
        in.read(reinterpret_cast<char*>(&trackingState), sizeof(trackingState));
    }
};

// 一帧中所有关节的数据
struct FrameData {
    INT64 timestamp;                      // 时间戳
    std::vector<JointData> joints;        // 关节数据数组
    
    // 序列化到文件
    void serialize(std::ofstream& out) const {
        out.write(reinterpret_cast<const char*>(&timestamp), sizeof(timestamp));
        size_t jointCount = joints.size();
        out.write(reinterpret_cast<const char*>(&jointCount), sizeof(jointCount));
        for (const auto& joint : joints) {
            joint.serialize(out);
        }
    }
    
    // 从文件反序列化
    void deserialize(std::ifstream& in) {
        in.read(reinterpret_cast<char*>(&timestamp), sizeof(timestamp));
        size_t jointCount;
        in.read(reinterpret_cast<char*>(&jointCount), sizeof(jointCount));
        joints.resize(jointCount);
        for (auto& joint : joints) {
            joint.deserialize(in);
        }
    }
};

// 序列化一帧的骨骼数据到文件
inline bool serializeFrame(const std::string& filename, const FrameData& frame, bool append = false) {
    try {
        std::ofstream out(filename, append ? (std::ios::binary | std::ios::app) : std::ios::binary);
        if (!out) {
            return false;
        }
        frame.serialize(out);
        return true;
    } catch (...) {
        return false;
    }
}

// 从文件读取一帧骨骼数据
inline bool deserializeFrame(const std::string& filename, FrameData& frame) {
    try {
        std::ifstream in(filename, std::ios::binary);
        if (!in) {
            return false;
        }
        frame.deserialize(in);
        return true;
    } catch (...) {
        return false;
    }
}

}  // namespace kf

#endif // !SERIALIZE_H

