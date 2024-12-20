#include <windows.h>
#include <memory>
#include "samples/BodyBasics.h"
#include "log/KFLog.h"
#include "config.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR lpCmdLine,
                     _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    try {
        // 创建主窗口
        auto pApplication = std::make_unique<kf::BodyBasics>();
        if (!pApplication) {
            LOG_E("Failed to create application instance");
            return 1;
        }

        // 初始化应用程序
        HRESULT hr = pApplication->initialize(hInstance, nCmdShow);
        if (FAILED(hr)) {
            LOG_E("Failed to initialize application");
            return 1;
        }

        // 运行消息循环
        MSG msg = {0};
        while (WM_QUIT != msg.message) {
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            } else {
                pApplication->update();
            }
        }

        return static_cast<int>(msg.wParam);
    } catch (const std::exception& e) {
        LOG_E("Unhandled exception: %s", e.what());
        return 1;
    }
}

int main() {
    return wWinMain(GetModuleHandle(nullptr), nullptr, nullptr, SW_SHOWDEFAULT);
}
