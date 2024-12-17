#ifndef KFUI_H
#define KFUI_H

#include <windows.h>
#include <iostream>
#include <log/KFLog.h>
#include "KFcommon.h"
#include <d3d11.h>
#include <d2d1.h>

namespace kf {

    // UI �ؼ����
    extern HWND hStartButton;
    extern HWND hRecordButton;

    // ��ť�¼�������
    void OnStartButtonClick();
    void OnRecordButtonClick();

    // ���ڹ��̺���
    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
}

#endif // !KFUI_H



