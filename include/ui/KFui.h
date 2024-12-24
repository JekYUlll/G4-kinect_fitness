#ifndef KFUI_H
#define KFUI_H

#include <windows.h>
#include <iostream>
#include <d2d1.h>

#include "log/KFLog.h"
#include "KFcommon.h"

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



