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

    auto application = new Application;
    application->Run(hInstance, nShowCmd);

    return 0;
}

//// 注册窗口类和创建窗口
//int APIENTRY wWinMain(
//    _In_ HINSTANCE hInstance,
//    _In_opt_ HINSTANCE hPrevInstance,
//    _In_ LPWSTR lpCmdLine,
//    _In_ int nShowCmd
//)
//{
//    UNREFERENCED_PARAMETER(hPrevInstance);
//    UNREFERENCED_PARAMETER(lpCmdLine);
//
//    // 创建窗口类
//    const wchar_t CLASS_NAME[] = L"WindowClass";
//    WNDCLASS wc = {};
//    wc.lpfnWndProc = WindowProc; // 设置窗口过程函数
//    wc.hInstance = hInstance;
//    wc.lpszClassName = CLASS_NAME;
//
//    if (!RegisterClass(&wc)) {
//        return 0;
//    }
//
//    // 创建窗口
//    HWND hwnd = CreateWindowEx(
//        0,                              // 扩展风格
//        CLASS_NAME,                     // 类名
//        L"健身辅助程序",                 // 窗口标题
//        WS_OVERLAPPEDWINDOW,            // 窗口样式
//        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, // 窗口位置与大小
//        NULL,                           // 父窗口
//        NULL,                           // 菜单
//        hInstance,                     // 应用实例句柄
//        NULL                            // 额外的创建数据
//    );
//
//    if (hwnd == NULL) {
//        return 0;
//    }
//
//    ShowWindow(hwnd, nShowCmd);
//    UpdateWindow(hwnd);
//
//    // 消息循环
//    MSG msg = {};
//    while (GetMessage(&msg, NULL, 0, 0)) {
//        TranslateMessage(&msg);
//        DispatchMessage(&msg);
//    }
//
//    return 0;
//}
