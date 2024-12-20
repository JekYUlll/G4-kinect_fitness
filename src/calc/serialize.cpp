#include "../../include/calc/serialize.h"
#include "../../include/log/KFLog.h"
#include <filesystem>

namespace kf {

void serialize(std::ofstream& ofs, const JointData& data) {
    ofs.write(reinterpret_cast<const char*>(&data), sizeof(JointData));
}

void deserialize(std::ifstream& ifs, JointData& data) {
    ifs.read(reinterpret_cast<char*>(&data), sizeof(JointData));
}

void serialize(std::ofstream& ofs, const FrameData& data) {
    size_t jointCount = data.joints.size();
    ofs.write(reinterpret_cast<const char*>(&jointCount), sizeof(jointCount));
    ofs.write(reinterpret_cast<const char*>(&data.timestamp), sizeof(data.timestamp));

    for (const auto& joint : data.joints) {
        serialize(ofs, joint);
    }
}

void deserialize(std::ifstream& ifs, FrameData& data) {
    size_t jointCount;
    ifs.read(reinterpret_cast<char*>(&jointCount), sizeof(jointCount));
    ifs.read(reinterpret_cast<char*>(&data.timestamp), sizeof(data.timestamp));

    data.joints.clear();
    data.joints.reserve(jointCount);
    for (size_t i = 0; i < jointCount; ++i) {
        JointData joint;
        deserialize(ifs, joint);
        data.joints.push_back(joint);
    }
}

bool ensureDirectoryExists(const std::string& path) {
    try {
        std::filesystem::path dirPath = std::filesystem::path(path).parent_path();
        if (!dirPath.empty()) {
            std::filesystem::create_directories(dirPath);
        }
        return true;
    }
    catch (const std::exception& e) {
        LOG_E("创建目录时发生错误: {}", e.what());
        return false;
    }
}

} // namespace kf