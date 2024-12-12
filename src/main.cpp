#include <Windows.h>
#include "stdafx.h"
#include <strsafe.h>
#include "resource.h"

#include "log/KFLog.h"
#include "KFcommon.h"
#include "ui/KFui.h"
#include "samples/BodyBasics.h"

// ���������
int main(int argc, char** argv)
{
    kf::Logger::Init();
    LOG_I("Initializing G4 Kinect Fitness Platform...");

    Application application;
    return application.Run(GetModuleHandle(NULL), SW_SHOWNORMAL);
}
