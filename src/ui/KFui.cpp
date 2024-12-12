#include "ui/KFui.h"
#include "samples/BodyBasics.h"

namespace kf {
    
    HWND hStartButton; // 按钮控件的句柄

    // Direct3D device and context
    ID3D11Device* g_pd3dDevice = nullptr;
    ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
    IDXGISwapChain* g_pSwapChain = nullptr;

    // Direct2D resources
    ID2D1Factory* m_pD2DFactory = nullptr;
    ID2D1HwndRenderTarget* m_pRenderTarget = nullptr;

    // 按钮点击事件处理函数
    void OnStartButtonClick() {
        if (isTracking) {
            // 停止数据记录
            LOG_I("Recording paused.");
            isTracking = false;
        }
        else {
            // 开始数据记录
            LOG_I("Recording started.");
            isTracking = true;
        }
    }

    // 窗口过程函数
    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        Application* pApp = nullptr;
        pApp = reinterpret_cast<Application*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

        switch (uMsg) {
        case WM_NCCREATE: {
            // 从 CREATESTRUCT 获取传入的参数
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            pApp = reinterpret_cast<Application*>(pCreate->lpCreateParams);
            // 将指针存储到 GWLP_USERDATA 中
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pApp));
            LOG_D("Set GWLP_USERDATA in WM_NCCREATE");
            return TRUE;
        }
        case WM_CREATE: {
            // 创建开始/暂停按钮
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
                // 让 Application 处理绘制
                pApp->HandlePaint();
                // 注意：不要调用 BeginPaint/EndPaint，因为 HandlePaint 中已经处理了
                return 0;
            }
            // 如果没有 pApp，使用默认处理
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_ERASEBKGND: {
            // 返回1表示我们已经处理了背景擦除
            return 1;
        }
        case WM_SIZE: {
            if (pApp) {
                pApp->HandleResize();
                // 强制重绘
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
    //    // 创建 D3D11 设备
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

    //    // 设置 Direct2D 渲染目标
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
    //    // 设置 ImGui 的 DX11 后端
    //    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    //    return S_OK;
    //}
