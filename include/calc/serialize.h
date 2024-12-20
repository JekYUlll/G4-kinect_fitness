#pragma once

#include <string>
#include <fstream>
#include <vector>
#include "../KFcommon.h"

namespace kf {

// 序列化单个帧数据到文件
bool SaveFrame(const std::string& filename, const FrameData& frame, bool append = false);

// 从文件加载单个帧数据
bool LoadFrame(const std::string& filename, FrameData& frame);

// 从文件加载标准动作
bool LoadStandardAction(const std::string& filename);

// 异步加载标准动作
std::future<bool> LoadStandardActionAsync(const std::string& filename);

// 工具函数
std::string getStandardActionPath(const std::string& filename);
std::string generateRecordingPath();

} // namespace kf

