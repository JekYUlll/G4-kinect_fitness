#include <Windows.h>
#include "stdafx.h"
#include <strsafe.h>
#include "resource.h"

#include "log/KFLog.h"
#include "KFcommon.h"

#include "samples/BodyBasics.h"

int main(int argc, char** argv)
{
    kf::Logger::Init();
    LOG_T("G4 Kinect Fittness Platform: {0}, {1}, {3}", __FUNCTION__, 1, 0.14f, true);
    LOG_D("G4 Kinect Fittness Platform: {0}, {1}, {3}", __FUNCTION__, 1, 0.14f, true);
    LOG_I("G4 Kinect Fittness Platform: {0}, {1}, {3}", __FUNCTION__, 1, 0.14f, true);
    LOG_W("G4 Kinect Fittness Platform: {0}, {1}, {3}", __FUNCTION__, 1, 0.14f, true);
    LOG_E("G4 Kinect Fittness Platform: {0}, {1}, {3}", __FUNCTION__, 1, 0.14f, true);


    return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLineW(), SW_SHOWNORMAL);
}

/// <summary>
/// Entry point for the application
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="hPrevInstance">always 0</param>
/// <param name="lpCmdLine">command line arguments</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
/// <returns>status</returns>
int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    CBodyBasics application;
    application.Run(hInstance, nShowCmd);

    return 0;
}

