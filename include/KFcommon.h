#ifndef KFCOMMON_H
#define KFCOMMON_H

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <Kinect.h>
#include <vector>

#include "config.h"
#include "ui/KFui.h"

//#include "imgui/imgui.h"
//#include "imgui/imgui_impl_win32.h"
//#include "imgui/imgui_impl_dx11.h"

namespace kf {

	// 全局变量，用于标识开始/暂停状态
	extern bool isTracking;

	extern bool isRecording;


}

#endif // !KFCOMMON_H


