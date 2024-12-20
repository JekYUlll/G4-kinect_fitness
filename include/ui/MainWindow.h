#pragma once

#include <windows.h>
#include <d2d1.h>
#include <memory>
#include "../core/FrameProcessor.h"
#include "../core/KinectManager.h"
#include "../render/SkeletonRenderer.h"
#include "../action/ActionRecorder.h"
#include "../calc/SimilarityCalculator.h"

namespace kf {
namespace ui {

class MainWindow : public core::IFrameObserver {
public:
    MainWindow();
    ~MainWindow();

    // 初始化窗口
    bool initialize(HINSTANCE hInstance);

    // 清理资源
    void cleanup();

private:
    // 初始化 Direct2D
    bool initializeD2D();

    // 渲染窗口内容
    void render();

    // 处理窗口消息
    static LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT handleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    // IFrameObserver 接口实现
    void onFrameProcessed(const FrameData& frameData) override;

private:
    HWND m_hWnd;
    HINSTANCE m_hInstance;
    ID2D1Factory* m_pD2DFactory;
    ID2D1HwndRenderTarget* m_pRenderTarget;

    std::unique_ptr<core::KinectManager> m_pKinectManager;
    std::unique_ptr<core::FrameProcessor> m_pFrameProcessor;
    std::unique_ptr<render::SkeletonRenderer> m_pSkeletonRenderer;
    std::unique_ptr<ActionRecorder> m_pActionRecorder;
    std::unique_ptr<SimilarityCalculator> m_pSimilarityCalculator;
};

} // namespace ui
} // namespace kf 