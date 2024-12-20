#include "ui/KFui.h"
#include "samples/BodyBasics.h"

namespace kf {

    // UI �ؼ����
    HWND hStartButton = nullptr;
    HWND hRecordButton = nullptr;
    HWND hPrintButton = nullptr;
    HWND hPlayButton = nullptr;

    // ��ť����¼�������
    void OnStartButtonClick() {
        
    }

    void OnRecordButtonClick() {

    }

    void OnPrintButtonClick() {
        LOG_D("Begin printing standard move.");
        kf::g_actionTemplate->PrintData();
        LOG_D("Finish printing standard move.");
    }

    void OnAidButtonClick() {

    }

    // ���ڹ��̺���
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
            // ������ʼ/��ͣ��ť
            hStartButton = CreateWindow(
                L"BUTTON",                          // ��������
                L"Start",                           // ��ť�ı�
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // ��ʽ
                10,                                 // x λ��
                10,                                 // y λ��
                100,                                // ���
                30,                                 // �߶�
                hwnd,                               // ������
                (HMENU)1,                          // �˵�/�ؼ�ID
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),  // ʵ�����
                NULL                                // �������
            );

            // ����¼�ư�ť
            hRecordButton = CreateWindow(
                L"BUTTON",                          // ��������
                L"Record",                          // ��ť�ı�
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // ��ʽ
                120,                                // x λ��
                10,                                 // y λ��
                100,                                // ���
                30,                                 // �߶�
                hwnd,                               // ������
                (HMENU)2,                          // �˵�/�ؼ�ID
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),  // ʵ�����
                NULL                                // �������
            );

            // ������ӡ��ť
            hPrintButton = CreateWindow(
                L"BUTTON",                          // ��������
                L"Print",                          // ��ť�ı�
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // ��ʽ
                230,                                // x λ��
                10,                                 // y λ��
                100,                                // ���
                30,                                 // �߶�
                hwnd,                               // ������
                (HMENU)3,                          // �˵�/�ؼ�ID
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),  // ʵ�����
                NULL                                // �������
            );

            // �������Ű�ť
            hPlayButton = CreateWindow(
                L"BUTTON",                          // ��������
                L"Aid",                          // ��ť�ı�
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // ��ʽ
                340,                                // x λ��
                10,                                 // y λ��
                100,                                // ���
                30,                                 // �߶�
                hwnd,                               // ������
                (HMENU)4,                          // �˵�/�ؼ�ID
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),  // ʵ�����
                NULL                                // �������
            );

            // ���ð�ť����
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
            ValidateRect(hwnd, NULL);  // ����ϵͳ�����Ѹ��£������ظ����� WM_PAINT
            return 0;
        }

        case WM_ERASEBKGND:
            return TRUE;

        case WM_SIZE: {
            if (pApp) {
                // ��ȡ�µĿ�Ⱥ͸߶�
                UINT width = LOWORD(lParam);
                UINT height = HIWORD(lParam);
                // ������ȾĿ���С
                if (pApp->GetRenderTarget()) {
                    D2D1_SIZE_U size = D2D1::SizeU(width, height);
                    pApp->GetRenderTarget()->Resize(&size);
                }
                // �����ػ洰��
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
            case 1:  // Start ��ť
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

            case 2:  // Record ��ť
                OnRecordButtonClick();
                if (!pApp->IsRecording()) {
                    if (!pApp->IsCalcing()) { // �����ڼ���״̬�²���¼��
                        LOG_E("must start Calcing before Recording");
                        break;
                    }
                    pApp->SetRecording(true);
                    SetWindowText(hRecordButton, L"Stop");
                    LOG_I("Start Recording...");
                } else {
                    pApp->SetRecording(false);
                    SetWindowText(hRecordButton, L"Record");
                    LOG_I("Finish Recording...");
                }
                break;

            case 3: // Print ��ť
                OnPrintButtonClick();
                break;

            case 4: // Play ��ť
                OnAidButtonClick();
                if (pApp->SwitchPlayingTemplate()) {
                    SetWindowText(hPlayButton, L"Pause");
                    pApp->SetPlaybackStartTime(0); // ���ò������
                    LOG_I("Started playing action template.");
                }
                else {
                    SetWindowText(hPlayButton, L"Play");
                    LOG_I("Stopped playing action template.");
                }
                //InvalidateRect(m_hWnd, NULL, FALSE); // ���������ػ�
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