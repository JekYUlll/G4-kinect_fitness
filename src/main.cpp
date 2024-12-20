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

int main(int argc, char** argv)
{
    {
        const int border_length = 42;
        std::string border = "--+" + std::string(border_length, '-') + "+--";
        std::string empty_line = "| " + std::string(border_length, ' ') + " |";
        std::cout << border << std::endl;
        std::cout << "|   ____ _  _         _  _______ ____   |" << std::endl;
        std::cout << "|  / ___| || |       | |/ /  ___/ ___|  |" << std::endl;
        std::cout << "| | |  _| || |_ _____| ' /| |_ | |      |" << std::endl;
        std::cout << "| | |_| |__   _|_____| . \\|  _|| |___   |" << std::endl;
        std::cout << "|  \\____|  |_|       |_|\_\\_|   \\____|  |" << std::endl;
        std::cout << border << std::endl;
    }

    kf::Logger::Init();
    LOG_I("Initializing G4 Kinect Fitness Platform...");
    kf::InitConfig(KF_CONFIG_FILE);

    Application application;
    return application.Run(GetModuleHandle(NULL), SW_SHOWNORMAL);
}
