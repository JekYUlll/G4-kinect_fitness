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

//#include "imgui/imgui.h"
//#include "imgui/imgui_impl_win32.h"
//#include "imgui/imgui_impl_dx11.h"

namespace kf {

	// 全局变量，用于标识开始/暂停状态
	/*extern bool isTracking;

	extern bool isRecording;*/

	extern std::string std_file_path;

	extern int window_width;
	extern int window_height;

	// 保存间隔 -- 防止采样率太高的卡顿
	extern INT64 recordInterval;


}

#endif // !KFCOMMON_H


