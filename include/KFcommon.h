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

	// ȫ�ֱ��������ڱ�ʶ��ʼ/��ͣ״̬
	/*extern bool isTracking;

	extern bool isRecording;*/

	extern std::string std_file_path;

	extern int window_width;
	extern int window_height;

	// ������ -- ��ֹ������̫�ߵĿ���
	extern INT64 recordInterval;


}

#endif // !KFCOMMON_H


