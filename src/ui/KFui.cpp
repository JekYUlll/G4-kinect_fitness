#include "ui/KFui.h"
#include "samples/BodyBasics.h"

namespace kf {

    // UI 控件句柄
    HWND hStartButton = nullptr;
    HWND hRecordButton = nullptr;
    HWND hPrintButton = nullptr;

    // 按钮点击事件处理函数
    void OnStartButtonClick() {
        
    }

    void OnRecordButtonClick() {

    }

    void OnPrintButtonClick() {
        LOG_D("Begin printing standard move.");
        kf::g_actionTemplate->PrintData();
        LOG_D("Finish printing standard move.");
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
            //LOG_D("Set GWLP_USERDATA in WM_NCCREATE");
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

            // 创建打印按钮
            hPrintButton = CreateWindow(
                L"BUTTON",                          // 窗口类名
                L"Print",                          // 按钮文本
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // 样式
                230,                                // x 位置
                10,                                 // y 位置
                100,                                // 宽度
                30,                                 // 高度
                hwnd,                               // 父窗口
                (HMENU)3,                          // 菜单/控件ID
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),  // 实例句柄
                NULL                                // 额外参数
            );

            // 设置按钮字体
            HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            SendMessage(hStartButton, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hRecordButton, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hPrintButton, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            return 0;
        }

        case WM_PAINT: {
            if (pApp) {
                pApp->HandlePaint();
                return 0;
            }
            // PAINTSTRUCT ps;
            // HDC hdc = BeginPaint(hwnd, &ps);
            // EndPaint(hwnd, &ps);
            ValidateRect(hwnd, NULL);  // 告诉系统区域已更新，避免重复发送 WM_PAINT
            return 0;
        }

        case WM_ERASEBKGND:
            return TRUE;

        case WM_SIZE: {
            if (pApp) {
                // 获取新的宽度和高度
                UINT width = LOWORD(lParam);
                UINT height = HIWORD(lParam);
                // 调整渲染目标大小
                if (pApp->GetRenderTarget()) {
                    D2D1_SIZE_U size = D2D1::SizeU(width, height);
                    pApp->GetRenderTarget()->Resize(&size);
                }
                // 立即重绘窗口
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
            case 1:  // Start 按钮
                OnStartButtonClick();
                if (!pApp->IsCalcing()) {
                    pApp->SetCalcing(true);
                    SetWindowText(hStartButton, L"Pause");
                    LOG_I("Start Calculating...");
                }
                else {
                    pApp->SetCalcing(false);
                    SetWindowText(hStartButton, L"Start");
                    LOG_I("Pause Calculating...");
                }
                break;

            case 2:  // Record 按钮
                OnRecordButtonClick();
                if (!pApp->IsRecording()) {
                    pApp->SetRecording(true);
                    SetWindowText(hRecordButton, L"Stop");
                    LOG_I("Start Recording...");
                } else {
                    pApp->SetRecording(false);
                    SetWindowText(hRecordButton, L"Record");
                    LOG_I("Finish Recording...");
                }
                break;

            case 3: // Print 按钮
                OnPrintButtonClick();
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