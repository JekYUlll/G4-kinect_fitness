#include <Windows.h>
#include <strsafe.h>
#include <string>
#include <memory>

#include "stdafx.h"
#include "resource.h"
#include "log/KFLog.h"
#include "KFcommon.h"
#include "ui/KFui.h"
#include "samples/BodyBasics.h"
#include "ConfigReader.hpp"

static std::string config_path = "config.txt";

int main(int argc, char** argv)
{
    kf::Logger::Init();
    LOG_I("Initializing G4 Kinect Fitness Platform...");

    {
        auto config = kf::ReadConfig(config_path);
        kf::std_file_path = config.std_file_path;
        kf::window_width = config.window_width;
        kf::window_height = config.window_height;
        LOG_T("标准动作文件路径: {}", kf::std_file_path);
        LOG_T("分辨率: {0} * {1}", kf::window_width, kf::window_height);
        // 加载标准动作
        kf::g_actionTemplate = std::make_unique<kf::ActionTemplate>(kf::std_file_path);
        if (kf::g_actionTemplate && !kf::g_actionTemplate->getFrames().empty()) {
            LOG_I("标准动作加载成功! 共有 {} 帧数据。", kf::g_actionTemplate->getFrameCount());
        }
    }

    Application application;
    return application.Run(GetModuleHandle(NULL), SW_SHOWNORMAL);
}
