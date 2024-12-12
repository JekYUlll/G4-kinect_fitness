#include <Windows.h>
#include "stdafx.h"
#include <strsafe.h>
#include "resource.h"

#include "log/KFLog.h"
#include "KFcommon.h"
#include "ui/KFui.h"
#include "samples/BodyBasics.h"

int main(int argc, char** argv)
{
    kf::Logger::Init();
    LOG_I("Initializing G4 Kinect Fitness Platform...");

    return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLineW(), SW_SHOWNORMAL);
}

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    Application application;
    return application.Run(hInstance, nShowCmd);
}
