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

namespace kf {

	extern std::string std_file_path;

	extern int window_width;
	extern int window_height;

	// 保存间隔 -- 防止采样率太高的卡顿
	extern INT64 recordInterval;

}

#endif // !KFCOMMON_H


