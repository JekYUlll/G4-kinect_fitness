#include "../../include/action/ActionTemplate.h"
#include "../../include/calc/serialize.h"
#include "../../include/log/KFLog.h"
#include <fstream>

namespace kf {

ActionTemplate::ActionTemplate() {
}

ActionTemplate::~ActionTemplate() {
    clear();
}

bool ActionTemplate::loadFromFile(const std::string& filename) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs.is_open()) {
        LOG_E("无法打开文件: {}", filename);
        return false;
    }

    try {
        size_t frameCount;
        ifs.read(reinterpret_cast<char*>(&frameCount), sizeof(frameCount));

        m_frames.clear();
        m_frames.reserve(frameCount);

        for (size_t i = 0; i < frameCount; ++i) {
            FrameData frame;
            deserialize(ifs, frame);
            m_frames.push_back(frame);
        }

        LOG_I("从文件加载了 {} 帧数据", frameCount);
        return true;
    }
    catch (const std::exception& e) {
        LOG_E("加载文件时发生错误: {}", e.what());
        return false;
    }
}

bool ActionTemplate::saveToFile(const std::string& filename) const {
    if (!ensureDirectoryExists(filename)) {
        LOG_E("无法创建目录: {}", filename);
        return false;
    }

    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs.is_open()) {
        LOG_E("无法创建文件: {}", filename);
        return false;
    }

    try {
        size_t frameCount = m_frames.size();
        ofs.write(reinterpret_cast<const char*>(&frameCount), sizeof(frameCount));

        for (const auto& frame : m_frames) {
            serialize(ofs, frame);
        }

        LOG_I("保存了 {} 帧数据到文件", frameCount);
        return true;
    }
    catch (const std::exception& e) {
        LOG_E("保存文件时发生错误: {}", e.what());
        return false;
    }
}

void ActionTemplate::addFrame(const FrameData& frame) {
    m_frames.push_back(frame);
}

void ActionTemplate::clear() {
    m_frames.clear();
}

const std::vector<FrameData>& ActionTemplate::getFrames() const {
    return m_frames;
}

void ActionTemplate::print() const {
    LOG_I("动作模板包含 {} 帧数据", m_frames.size());
    for (size_t i = 0; i < m_frames.size(); ++i) {
        LOG_I("帧 {}: {} 个关节点, 时间戳 {}", 
            i, m_frames[i].joints.size(), m_frames[i].timestamp);
    }
}

} // namespace kf 