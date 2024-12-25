#ifndef KF_UI_WINDOW_H
#define KF_UI_WINDOW_H

#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <d2d1.h>

#include "log/logger.h"
#include "core/common.h"
#include "core/application.h"

namespace kfc {

    // UI 控件句柄
    extern HWND hStartButton;
    extern HWND hRecordButton;
    extern HWND hPrintButton;
    extern HWND hPlayButton;

    // 按钮事件处理函数
    void OnStartButtonClick();
    void OnRecordButtonClick();
    void OnPrintButtonClick();
    void OnPlayButtonClick();

    // 窗口过程函数
    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
}

#endif // !KF_UI_WINDOW_H



