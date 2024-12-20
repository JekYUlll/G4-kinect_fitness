#include "calc/serialize.h"

namespace kf {

    // 全局变量
    std::mutex templateMutex;
    std::unique_ptr<ActionTemplate> g_actionTemplate;

    // 序列化到文件
    void JointData::serialize(std::ofstream& out) const {
        out.write(reinterpret_cast<const char*>(&type), sizeof(type));
        out.write(reinterpret_cast<const char*>(&position), sizeof(position));
        out.write(reinterpret_cast<const char*>(&trackingState), sizeof(trackingState));
    }

    // 从文件反序列化
    void JointData::deserialize(std::ifstream& in) {
        in.read(reinterpret_cast<char*>(&type), sizeof(type));
        in.read(reinterpret_cast<char*>(&position), sizeof(position));
        in.read(reinterpret_cast<char*>(&trackingState), sizeof(trackingState));
    }

    // 序列化到文件
    void FrameData::serialize(std::ofstream& out) const {
        out.write(reinterpret_cast<const char*>(&timestamp), sizeof(timestamp));
        size_t jointCount = joints.size();
        out.write(reinterpret_cast<const char*>(&jointCount), sizeof(jointCount));
        for (const auto& joint : joints) {
            joint.serialize(out);
        }
    }

    // 从文件反序列化
    void FrameData::deserialize(std::ifstream& in) {
        in.read(reinterpret_cast<char*>(&timestamp), sizeof(timestamp));
        size_t jointCount;
        in.read(reinterpret_cast<char*>(&jointCount), sizeof(jointCount));
        joints.resize(jointCount);
        for (auto& joint : joints) {
            joint.deserialize(in);
        }
    }

    // 序列化一帧的骨骼数据到文件
    bool SaveFrame(const std::string& filename, const FrameData& frame, bool append) {
        try {
            std::ofstream out(filename, append ? (std::ios::binary | std::ios::app) : std::ios::binary);
            if (!out) {
                return false;
            }
            frame.serialize(out);
            return true;
        }
        catch (...) {
            return false;
        }
    }

    // 从文件读取一帧骨骼数据
    bool LoadFrame(const std::string& filename, FrameData& frame) {
        try {
            std::ifstream in(filename, std::ios::binary);
            if (!in) {
                return false;
            }
            frame.deserialize(in);
            return true;
        }
        catch (...) {
            return false;
        }
    }

    // 构造函数，加载标准动作文件
    ActionTemplate::ActionTemplate(const std::string& filePath) {
        frames = std::make_unique<std::vector<kf::FrameData>>();
        if (!loadFromFile(filePath)) {
            throw std::runtime_error("Failed to load action template from file: " + filePath);
            LOG_E("Failed to load action template from file: {}", filePath);
        }
    }

    // 加载标准动作数据
    bool ActionTemplate::loadFromFile(const std::string& filename) {
        try {
            std::ifstream in(filename, std::ios::binary);
            if (!in) return false;

            frames->clear();
            while (in.peek() != EOF) {
                kf::FrameData frame;
                frame.deserialize(in);
                frames->push_back(frame);
            }

            in.close();
            // 打印加载的动作帧日志
            LOG_I("Successfully loaded action template from file: {}", filename);
            LOG_I("Total frames loaded: {}", frames->size());

            //PrintData();

            return true;
        }
        catch (...) {
            return false;
        }
    }

    // 把标准动作打印到日志
    void ActionTemplate::PrintData() const {
        for (size_t i = 0; i < frames->size(); ++i) {
            const auto& frame = (*frames)[i];
            LOG_D("Frame {} - Timestamp: {}", i, frame.timestamp);
            LOG_D("Number of joints: {}", frame.joints.size());

            for (const auto& joint : frame.joints) {
                LOG_T("  Joint Type: {}, Position: ({:.2f}, {:.2f}, {:.2f}), TrackingState: {}",
                    joint.type, joint.position.X, joint.position.Y, joint.position.Z, joint.trackingState);
            }
        }
    }

    // 异步加载标准动作
    std::future<bool> loadStandardActionAsync(const std::string& filePath) {
        return std::async(std::launch::async, [&]() {
            std::lock_guard<std::mutex> lock(templateMutex);
            return g_actionTemplate->loadFromFile(filePath);
            });
    }


}