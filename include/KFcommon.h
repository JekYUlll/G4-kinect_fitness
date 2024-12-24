#ifndef KFCOMMON_H
#define KFCOMMON_H

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <Kinect.h>
#include <vector>
#include <string>

#include "config/config.h"
#include "config/ConfigReader.h"
#include "log/KFLog.h"

#define NOMINMAX
namespace kfc {

	// 确保必要的目录存在
	bool ensureDirectoryExists();

	// 获取标准动作文件的完整路径
	std::string getStandardActionPath(const std::string& filename);

	// 获取新录制文件的路径
	std::string generateRecordingPath();

}

#endif // !KFCOMMON_H


