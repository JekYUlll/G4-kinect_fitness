#ifndef KFCOMMON_H
#define KFCOMMON_H

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <Kinect.h>
#include <vector>
#include <string>

#include "config.h"
#include "ui/KFui.h"

namespace kfc {

	extern std::string std_file_path;

	extern int window_width;
	extern int window_height;

	// 保存间隔 -- 防止采样率太高的卡顿
	extern INT64 recordInterval;

	// 确保必要的目录存在
	bool ensureDirectoryExists();

	// 获取标准动作文件的完整路径
	std::string getStandardActionPath(const std::string& filename);

	// 获取新录制文件的路径
	std::string generateRecordingPath();

}

#endif // !KFCOMMON_H


