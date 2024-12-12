#include "ui/KFui.h"
#include "samples/BodyBasics.h"

namespace kf {
    // Direct3D device and context
    ID3D11Device* g_pd3dDevice = nullptr;
    ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
    IDXGISwapChain* g_pSwapChain = nullptr;

    // Direct2D resources
    ID2D1Factory* m_pD2DFactory = nullptr;
    ID2D1HwndRenderTarget* m_pRenderTarget = nullptr;

    // UI 控件句柄
    HWND hStartButton = nullptr;
    HWND hRecordButton = nullptr;

    // 按钮点击事件处理函数
    void OnStartButtonClick() {
        static bool isStarted = false;
        isStarted = !isStarted;
        SetWindowText(hStartButton, isStarted ? L"Pause" : L"Start");
        LOG_I(isStarted ? "Tracking started." : "Tracking paused.");
    }

    void OnRecordButtonClick() {
        LOG_I("Record button clicked.");
    }

    // 窗口过程函数
    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        Application* pApp = nullptr;
        pApp = reinterpret_cast<Application*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

        switch (uMsg) {
        case WM_NCCREATE: {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            pApp = reinterpret_cast<Application*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pApp));
            LOG_D("Set GWLP_USERDATA in WM_NCCREATE");
            return TRUE;
        }

        case WM_CREATE: {
            // 创建开始/暂停按钮
            hStartButton = CreateWindow(
                L"BUTTON",                          // 窗口类名
                L"Start",                           // 按钮文本
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // 样式
                10,                                 // x 位置
                10,                                 // y 位置
                100,                                // 宽度
                30,                                 // 高度
                hwnd,                               // 父窗口
                (HMENU)1,                          // 菜单/控件ID
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),  // 实例句柄
                NULL                                // 额外参数
            );

            // 创建录制按钮
            hRecordButton = CreateWindow(
                L"BUTTON",                          // 窗口类名
                L"Record",                          // 按钮文本
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // 样式
                120,                                // x 位置
                10,                                 // y 位置
                100,                                // 宽度
                30,                                 // 高度
                hwnd,                               // 父窗口
                (HMENU)2,                          // 菜单/控件ID
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),  // 实例句柄
                NULL                                // 额外参数
            );

            // 设置按钮字体
            HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            SendMessage(hStartButton, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hRecordButton, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            return 0;
        }

        case WM_PAINT: {
            if (pApp) {
                pApp->HandlePaint();
                return 0;
            }
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_SIZE: {
            if (pApp) {
                pApp->HandleResize();
                InvalidateRect(GetDlgItem(hwnd, IDC_VIDEOVIEW), NULL, FALSE);
            }
            return 0;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
            case 1:  // Start 按钮
                OnStartButtonClick();
                break;

            case 2:  // Record 按钮
                OnRecordButtonClick();
                if (!pApp->IsRecording()) {
                    pApp->SetRecording(true);
                    SetWindowText(hRecordButton, L"Stop");
                } else {
                    pApp->SetRecording(false);
                    SetWindowText(hRecordButton, L"Record");
                }
                break;
            }
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }
}