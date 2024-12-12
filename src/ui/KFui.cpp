#include "ui/KFui.h"
#include "samples/BodyBasics.h"

namespace kf {
    
    HWND hStartButton; // ��ť�ؼ��ľ��

    // Direct3D device and context
    ID3D11Device* g_pd3dDevice = nullptr;
    ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
    IDXGISwapChain* g_pSwapChain = nullptr;

    // Direct2D resources
    ID2D1Factory* m_pD2DFactory = nullptr;
    ID2D1HwndRenderTarget* m_pRenderTarget = nullptr;

    // ��ť����¼�������
    void OnStartButtonClick() {
        if (isTracking) {
            // ֹͣ���ݼ�¼
            LOG_I("Recording paused.");
            isTracking = false;
        }
        else {
            // ��ʼ���ݼ�¼
            LOG_I("Recording started.");
            isTracking = true;
        }
    }

    // ���ڹ��̺���
    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        Application* pApp = nullptr;
        pApp = reinterpret_cast<Application*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

        switch (uMsg) {
        case WM_NCCREATE: {
            // �� CREATESTRUCT ��ȡ����Ĳ���
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            pApp = reinterpret_cast<Application*>(pCreate->lpCreateParams);
            // ��ָ��洢�� GWLP_USERDATA ��
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pApp));
            LOG_D("Set GWLP_USERDATA in WM_NCCREATE");
            return TRUE;
        }
        case WM_CREATE: {
            // ������ʼ/��ͣ��ť
            hStartButton = CreateWindow(
                L"BUTTON",
                L"Start",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                10,
                10,
                100,
                30,
                hwnd,
                (HMENU)1,
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                NULL);

            if (!hStartButton) {
                LOG_E("Failed to create button: {}", GetLastError());
                return -1;
            }
            return 0;
        }
        case WM_PAINT: {
            if (pApp) {
                // �� Application �������
                pApp->HandlePaint();
                // ע�⣺��Ҫ���� BeginPaint/EndPaint����Ϊ HandlePaint ���Ѿ�������
                return 0;
            }
            // ���û�� pApp��ʹ��Ĭ�ϴ���
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_ERASEBKGND: {
            // ����1��ʾ�����Ѿ������˱�������
            return 1;
        }
        case WM_SIZE: {
            if (pApp) {
                pApp->HandleResize();
                // ǿ���ػ�
                InvalidateRect(GetDlgItem(hwnd, IDC_VIDEOVIEW), NULL, FALSE);
            }
            return 0;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                OnStartButtonClick();
                if (isTracking) {
                    SetWindowText(hStartButton, L"Pause");
                }
                else {
                    SetWindowText(hStartButton, L"Start");
                }
            }
            return 0;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }
}

    //HRESULT CreateDeviceAndContext(HWND hwnd) {
    //    // ���� D3D11 �豸
    //    DXGI_SWAP_CHAIN_DESC sd = {};
    //    sd.BufferCount = 1;
    //    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    //    sd.BufferDesc.Width = 800;
    //    sd.BufferDesc.Height = 600;
    //    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    //    sd.OutputWindow = hwnd;
    //    sd.SampleDesc.Count = 1;
    //    sd.Windowed = TRUE;

    //    D3D_FEATURE_LEVEL featureLevel;
    //    HRESULT hr = D3D11CreateDeviceAndSwapChain(
    //        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
    //        D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);

    //    if (FAILED(hr)) {
    //        return hr;
    //    }

    //    // ���� Direct2D ��ȾĿ��
    //    ID3D11Texture2D* pBackBuffer = nullptr;
    //    hr = g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    //    if (FAILED(hr)) {
    //        return hr;
    //    }

    //    hr = m_pD2DFactory->CreateHwndRenderTarget(
    //        D2D1::RenderTargetProperties(),
    //        D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(800, 600)),
    //        &m_pRenderTarget);

    //    if (FAILED(hr)) {
    //        return hr;
    //    }

    //    // Setup Dear ImGui context
    //    IMGUI_CHECKVERSION();
    //    ImGui::CreateContext();
    //    ImGuiIO& io = ImGui::GetIO();
    //    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //    // Setup Platform/Renderer backends
    //    ImGui_ImplWin32_Init(hwnd);
    //    // ���� ImGui �� DX11 ���
    //    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    //    return S_OK;
    //}
