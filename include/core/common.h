#ifndef KF_COMMON_H
#define KF_COMMON_H

#include <Windows.h>
#include <Kinect.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "config/config.h"
#include "log/logger.h"

#define NOMINMAX
namespace kfc {

	// 确保必要的目录存在
	bool ensureDirectoryExists();

	// 获取标准动作文件的完整路径
	std::string getStandardActionPath(const std::string& filename);

	// 获取新录制文件的路径
	std::string generateRecordingPath();

}

#endif // !KF_COMMON_H


