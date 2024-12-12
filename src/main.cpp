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

//// ע�ᴰ����ʹ�������
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
//    // ����������
//    const wchar_t CLASS_NAME[] = L"WindowClass";
//    WNDCLASS wc = {};
//    wc.lpfnWndProc = WindowProc; // ���ô��ڹ��̺���
//    wc.hInstance = hInstance;
//    wc.lpszClassName = CLASS_NAME;
//
//    if (!RegisterClass(&wc)) {
//        return 0;
//    }
//
//    // ��������
//    HWND hwnd = CreateWindowEx(
//        0,                              // ��չ���
//        CLASS_NAME,                     // ����
//        L"����������",                 // ���ڱ���
//        WS_OVERLAPPEDWINDOW,            // ������ʽ
//        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, // ����λ�����С
//        NULL,                           // ������
//        NULL,                           // �˵�
//        hInstance,                     // Ӧ��ʵ�����
//        NULL                            // ����Ĵ�������
//    );
//
//    if (hwnd == NULL) {
//        return 0;
//    }
//
//    ShowWindow(hwnd, nShowCmd);
//    UpdateWindow(hwnd);
//
//    // ��Ϣѭ��
//    MSG msg = {};
//    while (GetMessage(&msg, NULL, 0, 0)) {
//        TranslateMessage(&msg);
//        DispatchMessage(&msg);
//    }
//
//    return 0;
//}
