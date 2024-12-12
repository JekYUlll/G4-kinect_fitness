#ifndef KFUI_H
#define KFUI_H

#include <windows.h>
#include <iostream>
#include <log/KFLog.h>
#include "KFcommon.h"
#include <d3d11.h>
#include <d2d1.h>

namespace kf {
    // Direct3D device and context
    extern ID3D11Device* g_pd3dDevice;
    extern ID3D11DeviceContext* g_pd3dDeviceContext;
    extern IDXGISwapChain* g_pSwapChain;

    // Direct2D resources
    extern ID2D1Factory* m_pD2DFactory;
    extern ID2D1HwndRenderTarget* m_pRenderTarget;

    // UI 控件句柄
    extern HWND hStartButton;
    extern HWND hRecordButton;

    // 按钮事件处理函数
    void OnStartButtonClick();
    void OnRecordButtonClick();

    // 窗口过程函数
    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
}

#endif // !KFUI_H



