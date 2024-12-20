#include "../../include/ui/MainWindow.h"
#include "../../include/log/KFLog.h"
#include "../../include/config.h"
#include <windowsx.h>

namespace kf {
namespace ui {

MainWindow::MainWindow()
    : m_hWnd(nullptr)
    , m_hInstance(nullptr)
    , m_pD2DFactory(nullptr)
    , m_pRenderTarget(nullptr)
    , m_pKinectManager(nullptr)
    , m_pFrameProcessor(nullptr)
    , m_pSkeletonRenderer(nullptr)
    , m_pActionRecorder(nullptr)
    , m_pSimilarityCalculator(nullptr) {
}

MainWindow::~MainWindow() {
    cleanup();
}

bool MainWindow::initialize(HINSTANCE hInstance) {
    m_hInstance = hInstance;

    // 注册窗口类
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWindow::windowProc;
    wc.hInstance = m_hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"KinectFitness";

    if (!RegisterClassEx(&wc)) {
        LOG_E("注册窗口类失败");
        return false;
    }

    // 创建窗口
    m_hWnd = CreateWindow(
        L"KinectFitness",
        L"Kinect Fitness",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr, nullptr,
        m_hInstance,
        this
    );

    if (!m_hWnd) {
        LOG_E("创建窗口失败");
        return false;
    }

    // 初始化 Direct2D
    if (!initializeD2D()) {
        LOG_E("初始化 Direct2D 失败");
        return false;
    }

    // 创建组件
    m_pKinectManager = std::make_unique<core::KinectManager>();
    if (!m_pKinectManager->initialize()) {
        LOG_E("初始化 Kinect 管理器失败");
        return false;
    }

    m_pFrameProcessor = std::make_unique<core::FrameProcessor>(m_pKinectManager.get());
    if (!m_pFrameProcessor->initialize()) {
        LOG_E("初始化帧处理器失败");
        return false;
    }

    m_pSkeletonRenderer = std::make_unique<render::SkeletonRenderer>();
    if (!m_pSkeletonRenderer->initialize(m_pRenderTarget)) {
        LOG_E("初始化骨骼渲染器失败");
        return false;
    }

    m_pActionRecorder = std::make_unique<ActionRecorder>(ACTION_BUFFER_SIZE);
    m_pSimilarityCalculator = std::make_unique<SimilarityCalculator>();

    // 注册为帧观察者
    m_pFrameProcessor->addObserver(this);

    // 显示窗口
    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);

    LOG_I("主窗口初始化成功");
    return true;
}

void MainWindow::cleanup() {
    if (m_pFrameProcessor) {
        m_pFrameProcessor->removeObserver(this);
    }

    m_pSimilarityCalculator.reset();
    m_pActionRecorder.reset();
    m_pSkeletonRenderer.reset();
    m_pFrameProcessor.reset();
    m_pKinectManager.reset();

    if (m_pRenderTarget) {
        m_pRenderTarget->Release();
        m_pRenderTarget = nullptr;
    }

    if (m_pD2DFactory) {
        m_pD2DFactory->Release();
        m_pD2DFactory = nullptr;
    }

    if (m_hWnd) {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
}

bool MainWindow::initializeD2D() {
    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &m_pD2DFactory
    );

    if (FAILED(hr)) {
        LOG_E("创建 D2D 工厂失败");
        return false;
    }

    RECT rc;
    GetClientRect(m_hWnd, &rc);

    hr = m_pD2DFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(
            m_hWnd,
            D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)
        ),
        &m_pRenderTarget
    );

    if (FAILED(hr)) {
        LOG_E("创建渲染目标失败");
        return false;
    }

    return true;
}

void MainWindow::onFrameProcessed(const FrameData& frameData) {
    // 更新动作记录器
    m_pActionRecorder->addFrame(frameData);

    // 计算相似度
    if (m_pSimilarityCalculator->hasTemplate()) {
        const auto& frames = m_pActionRecorder->getBuffer().getFrames();
        float similarity = m_pSimilarityCalculator->calculateSimilarity(
            std::vector<FrameData>(frames.begin(), frames.end())
        );
        // TODO: 显示相似度
    }

    // 触发重绘
    InvalidateRect(m_hWnd, nullptr, FALSE);
}

void MainWindow::render() {
    if (!m_pRenderTarget) {
        return;
    }

    m_pRenderTarget->BeginDraw();
    m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

    // TODO: 渲染骨骼和其他UI元素

    HRESULT hr = m_pRenderTarget->EndDraw();
    if (FAILED(hr)) {
        LOG_E("渲染失败");
    }
}

LRESULT CALLBACK MainWindow::windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = nullptr;

    if (message == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<MainWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->handleMessage(hWnd, message, wParam, lParam);
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT MainWindow::handleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_PAINT:
            render();
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_KEYDOWN:
            switch (wParam) {
                case VK_SPACE:
                    if (m_pActionRecorder->isRecording()) {
                        m_pActionRecorder->stopRecording();
                    } else {
                        m_pActionRecorder->startRecording();
                    }
                    return 0;

                case 'L':
                    // TODO: 加载动作模板
                    return 0;

                case 'S':
                    // TODO: 保存动作模板
                    return 0;
            }
            break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

} // namespace ui
} // namespace kf 