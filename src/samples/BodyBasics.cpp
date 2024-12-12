//------------------------------------------------------------------------------
// <copyright file="BodyBasics.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "samples/BodyBasics.h"

static const float c_JointThickness = 3.0f;
static const float c_TrackedBoneThickness = 6.0f;
static const float c_InferredBoneThickness = 1.0f;
static const float c_HandSize = 30.0f;

/// <summary>
/// Constructor
/// </summary>
Application::Application() :
    m_hWnd(NULL),
    m_nStartTime(0),
    m_nLastCounter(0),
    m_nFramesSinceUpdate(0),
    m_fFreq(0),
    m_nNextStatusTime(0LL),
    m_pKinectSensor(NULL),
    m_pCoordinateMapper(NULL),
    m_pBodyFrameReader(NULL),
    m_pD2DFactory(NULL),
    m_pRenderTarget(NULL),
    m_pBrushJointTracked(NULL),
    m_pBrushJointInferred(NULL),
    m_pBrushBoneTracked(NULL),
    m_pBrushBoneInferred(NULL),
    m_pBrushHandClosed(NULL),
    m_pBrushHandOpen(NULL),
    m_pBrushHandLasso(NULL)
{
    LARGE_INTEGER qpf = {0};
    if (QueryPerformanceFrequency(&qpf))
    {
        m_fFreq = double(qpf.QuadPart);
    }
}
  

/// <summary>
/// Destructor
/// </summary>
Application::~Application()
{
    DiscardDirect2DResources();

    // clean up Direct2D
    SafeRelease(m_pD2DFactory);

    // done with body frame reader
    SafeRelease(m_pBodyFrameReader);

    // done with coordinate mapper
    SafeRelease(m_pCoordinateMapper);

    // close the Kinect Sensor
    if (m_pKinectSensor)
    {
        m_pKinectSensor->Close();
    }

    SafeRelease(m_pKinectSensor);
}

void Application::HandlePaint()
{
    if (!m_pBodyFrameReader || !m_hWnd)
    {
        return;
    }

    HRESULT hr = EnsureDirect2DResources();
    if (FAILED(hr) || !m_pRenderTarget)
    {
        return;
    }

    // 获取当前时间
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);

    // 计算距离上次绘制的时间间隔
    double deltaTime = (currentTime.QuadPart - m_nLastCounter) / m_fFreq;
    
    // 如果距离上次绘制时间太短，就跳过这一帧
    if (deltaTime < 0.016)  // 约60fps
    {
        return;
    }

    m_pRenderTarget->BeginDraw();
    m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

    Update();

    hr = m_pRenderTarget->EndDraw();

    if (hr == D2DERR_RECREATE_TARGET)
    {
        hr = S_OK;
        DiscardDirect2DResources();
    }

    // 更新上次绘制时间
    m_nLastCounter = currentTime.QuadPart;
}

/// <summary>
/// Creates the main window and begins processing
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
int Application::Run(HINSTANCE hInstance, int nCmdShow)
{
    MSG msg = {0};    // 消息结构体，用来接收和处理窗口消息
    WNDCLASS wc;      // 用于注册窗口类的结构体

    // Dialog custom window class
    ZeroMemory(&wc, sizeof(wc));
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.cbWndExtra    = DLGWINDOWEXTRA;
    wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
    wc.hIcon         = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_APP));
    wc.lpfnWndProc   = kf::WindowProc;
    wc.lpszClassName = L"BodyBasicsAppDlgWndClass";
    wc.hInstance     = hInstance;
    
    if (!RegisterClassW(&wc)) {
        LOG_E("Failed to Register Class: {}", GetLastError());
        return 0;
    }

    LOG_I("Creating main application window...");
    HWND hWndApp = CreateWindowExW(
        0,                              
        L"BodyBasicsAppDlgWndClass",   
        L"Kinect Fitness",             
        WS_OVERLAPPEDWINDOW,           
        CW_USEDEFAULT, CW_USEDEFAULT,  
        800, 600,                      
        NULL,                          
        NULL,                          
        hInstance,                     
        this                           
    );

    // 创建用于D2D绘制的子窗口
    HWND hWndVideo = CreateWindowExW(
        0,
        L"STATIC",  // 使用静态控件作为绘制区域
        NULL,
        WS_CHILD | WS_VISIBLE | SS_BLACKRECT,
        10, 50,     // 位置在按钮下方
        780, 500,   // 大小略小于主窗口
        hWndApp,
        (HMENU)IDC_VIDEOVIEW,  // 确保在resource.h中定义了这个ID
        hInstance,
        NULL
    );

    if (!hWndApp || !hWndVideo) {
        LOG_E("Failed to create windows");
        return 0;
    }

    // 存储窗口句柄
    m_hWnd = hWndApp;

    // 初始化 Direct2D
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
    if (FAILED(hr)) {
        LOG_E("Failed to create D2D factory: 0x{:08X}", hr);
        return 0;
    }

    // 初始化 Kinect 传感器
    if (FAILED(InitializeDefaultSensor())) {
        LOG_E("Failed to initialize Kinect sensor");
        return 0;
    }

    // Show window
    ShowWindow(hWndApp, nCmdShow);
    // 可能不需要
    UpdateWindow(hWndApp);

    // 设置一个60Hz的定时器用于更新
    SetTimer(hWndApp, 1, 16, NULL);  // 16ms ≈ 60fps

    // Main message loop
    while (WM_QUIT != msg.message)
    {
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }

            if (hWndApp && IsDialogMessageW(hWndApp, &msg))
            {
                continue;
            }

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        // 让CPU休息一下
        Sleep(1);
    }

    KillTimer(hWndApp, 1);
    return static_cast<int>(msg.wParam);
}

/// <summary>
/// Main processing function
/// </summary>
void Application::Update()
{
    if (!m_pBodyFrameReader)
    {
        return;
    }

    IBodyFrame* pBodyFrame = NULL;
    HRESULT hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);

    if (SUCCEEDED(hr))
    {
        INT64 nTime = 0;
        hr = pBodyFrame->get_RelativeTime(&nTime);
        IBody* ppBodies[BODY_COUNT] = {0};

        if (SUCCEEDED(hr))
        {
            hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
        }

        if (SUCCEEDED(hr))
        {
            ProcessBody(nTime, BODY_COUNT, ppBodies);
        }

        for (int i = 0; i < _countof(ppBodies); ++i)
        {
            SafeRelease(ppBodies[i]);
        }
    }

    SafeRelease(pBodyFrame);
}

/// <summary>
/// Handles window messages, passes most to the class instance to handle
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK Application::MessageRouter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Application* pThis = NULL;
    
    if (WM_INITDIALOG == uMsg)
    {
        pThis = reinterpret_cast<Application*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }
    else
    {
        pThis = reinterpret_cast<Application*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (pThis)
    {
        return pThis->DlgProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

/// <summary>
/// Handle windows messages for the class instance
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK Application::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
        case WM_INITDIALOG:
        {
            // Bind application window handle
            m_hWnd = hWnd;

            // Init Direct2D
            HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
            if (FAILED(hr)) {
                LOG_E("Failed to create D2D factory: 0x{:08X}", hr);
                return FALSE;
            }

            // Get and initialize the default Kinect sensor
            if(InitializeDefaultSensor() < 0) {
                // >>> 添加个判断，测试初始化
                LOG_E("InitializeDefaultSensor");
                exit(1);
            }
        }
        break;

        // If the titlebar X is clicked, destroy app
        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;

        case WM_DESTROY:
            // Quit the main message pump
            PostQuitMessage(0);
            break;

        case WM_SIZE:
            if (m_pRenderTarget)
            {
                RECT rc;
                GetClientRect(GetDlgItem(m_hWnd, IDC_VIDEOVIEW), &rc);
                D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
                
                // 只有当大小真的改变时才调整
                D2D1_SIZE_U currentSize = m_pRenderTarget->GetPixelSize();
                if (size.width != currentSize.width || size.height != currentSize.height)
                {
                    m_pRenderTarget->Resize(size);
                    InvalidateRect(GetDlgItem(m_hWnd, IDC_VIDEOVIEW), NULL, FALSE);
                }
            }
            return 0;

        case WM_TIMER:
            if (wParam == 1)  // 我们的更新定时器
            {
                if (m_pBodyFrameReader)
                {
                    HandlePaint();
                }
                return 0;
            }
            break;
    }

    return FALSE;
}

/// <summary>
/// Initializes the default Kinect sensor
/// </summary>
/// <returns>indicates success or failure</returns>
HRESULT Application::InitializeDefaultSensor()
{
    HRESULT hr;

    hr = GetDefaultKinectSensor(&m_pKinectSensor);
    if (FAILED(hr)) {
        LOG_E("Failed to get default Kinect sensor: {}", hr);
        return hr;
    }

    if (m_pKinectSensor) {
        // Initialize the Kinect and get coordinate mapper and the body reader
        IBodyFrameSource* pBodyFrameSource = NULL;

        hr = m_pKinectSensor->Open();
        if (FAILED(hr)) {
            LOG_E("Failed to open Kinect sensor: {}", hr);
            return hr;
        }

        hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
        if (FAILED(hr)) {
            LOG_E("Failed to get coordinate mapper: {}", hr);
            return hr;
        }

        hr = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
        if (FAILED(hr)) {
            LOG_E("Failed to get body frame source: {}", hr);
            return hr;
        }

        hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
        if (FAILED(hr)) {
            LOG_E("Failed to open body frame reader: {}", hr);
            return hr;
        }

        SafeRelease(pBodyFrameSource);
    }

    if (!m_pKinectSensor || FAILED(hr)) {
        LOG_E("No ready Kinect found!");
        return E_FAIL;
    }

    return hr;
}

/// <summary>
/// Handle new body data
/// <param name="nTime">timestamp of frame</param>
/// <param name="nBodyCount">body data count</param>
/// <param name="ppBodies">body data in frame</param>
/// </summary>
void Application::ProcessBody(INT64 nTime, int nBodyCount, IBody** ppBodies)
{
    if (!m_hWnd || !m_pCoordinateMapper)
    {
        return;
    }

    // 不在这里调用 EnsureDirect2DResources???因为 HandlePaint 已经调用过了
    if (!m_pRenderTarget)
    {
        return;
    }

    // 获取窗口大小
    RECT rct;
    GetClientRect(GetDlgItem(m_hWnd, IDC_VIDEOVIEW), &rct);
    int width = rct.right;
    int height = rct.bottom;

    // ??历每个捕捉到的身体
    for (int i = 0; i < nBodyCount; ++i)
    {
        IBody* pBody = ppBodies[i];
        if (pBody)
        {
            BOOLEAN bTracked = false;
            HRESULT hr = pBody->get_IsTracked(&bTracked);

            if (SUCCEEDED(hr) && bTracked)
            {
                Joint joints[JointType_Count]; 
                D2D1_POINT_2F jointPoints[JointType_Count]{};
                HandState leftHandState = HandState_Unknown;
                HandState rightHandState = HandState_Unknown;

                pBody->get_HandLeftState(&leftHandState);
                pBody->get_HandRightState(&rightHandState);

                hr = pBody->GetJoints(_countof(joints), joints);
                if (SUCCEEDED(hr))
                {
                    for (int j = 0; j < _countof(joints); ++j)
                    {
                        jointPoints[j] = BodyToScreen(joints[j].Position, width, height);
                    }

                    DrawBody(joints, jointPoints);
                    DrawHand(leftHandState, jointPoints[JointType_HandLeft]);
                    DrawHand(rightHandState, jointPoints[JointType_HandRight]);
                }
            }
        }
    }

    if (!m_nStartTime)
    {
        m_nStartTime = nTime;
    }

    // 更新FPS显示
    double fps = 0.0;
    LARGE_INTEGER qpcNow = {0};
    if (m_fFreq)
    {
        if (QueryPerformanceCounter(&qpcNow))
        {
            if (m_nLastCounter)
            {
                m_nFramesSinceUpdate++;
                fps = m_fFreq * m_nFramesSinceUpdate / double(qpcNow.QuadPart - m_nLastCounter);
            }
        }
    }

    WCHAR szStatusMessage[64];
    StringCchPrintf(szStatusMessage, _countof(szStatusMessage), L" FPS = %0.2f    Time = %I64d", fps, (nTime - m_nStartTime));

    if (SetStatusMessage(szStatusMessage, 1000, false))
    {
        m_nLastCounter = qpcNow.QuadPart;
        m_nFramesSinceUpdate = 0;
    }
}

/// <summary>
/// Set the status bar message
/// </summary>
/// <param name="szMessage">message to display</param>
/// <param name="showTimeMsec">time in milliseconds to ignore future status messages</param>
/// <param name="bForce">force status update</param>
bool Application::SetStatusMessage(_In_z_ WCHAR* szMessage, DWORD nShowTimeMsec, bool bForce)
{
    INT64 now = GetTickCount64();

    if (m_hWnd && (bForce || (m_nNextStatusTime <= now)))
    {
        SetDlgItemText(m_hWnd, IDC_STATUS, szMessage);
        m_nNextStatusTime = now + nShowTimeMsec;

        return true;
    }

    return false;
}

/// <summary>
/// Ensure necessary Direct2d resources are created
/// </summary>
/// <returns>S_OK if successful, otherwise an error code</returns>
HRESULT Application::EnsureDirect2DResources()
{
    HRESULT hr = S_OK;

    // 确保 D2D Factory 存在
    if (!m_pD2DFactory) {
        LOG_E("D2D Factory is NULL");
        return E_FAIL;
    }

    if (!m_pRenderTarget)
    {
        HWND hWndVideo = GetDlgItem(m_hWnd, IDC_VIDEOVIEW);
        if (!hWndVideo) {
            LOG_E("Failed to get video window handle");
            return E_FAIL;
        }

        RECT rc;
        if (!GetClientRect(hWndVideo, &rc)) {
            LOG_E("Failed to get client rect: {}", GetLastError());
            return E_FAIL;
        }

        // 确保窗口尺寸有效
        if (rc.right <= rc.left || rc.bottom <= rc.top) {
            LOG_E("Invalid window size: {}x{}", rc.right - rc.left, rc.bottom - rc.top);
            return E_FAIL;
        }

        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
        D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
        rtProps.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
        rtProps.type = D2D1_RENDER_TARGET_TYPE_HARDWARE;
        rtProps.usage = D2D1_RENDER_TARGET_USAGE_NONE;
        rtProps.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;

        hr = m_pD2DFactory->CreateHwndRenderTarget(
            rtProps,
            D2D1::HwndRenderTargetProperties(
                hWndVideo,
                size,
                D2D1_PRESENT_OPTIONS_IMMEDIATELY | D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS
            ),
            &m_pRenderTarget
        );

        if (FAILED(hr)) {
            LOG_E("Failed to create render target: 0x{:08X}", hr);
            return hr;
        }

        // 创建画笔资源
        hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.27f, 0.75f, 0.27f), &m_pBrushJointTracked);
        if (FAILED(hr)) {
            LOG_E("Failed to create joint tracked brush: 0x{:08X}", hr);
            return hr;
        }

        hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &m_pBrushJointInferred);
        if (FAILED(hr)) {
            LOG_E("Failed to create joint inferred brush: 0x{:08X}", hr);
            return hr;
        }

        hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &m_pBrushBoneTracked);
        if (FAILED(hr)) {
            LOG_E("Failed to create bone tracked brush: 0x{:08X}", hr);
            return hr;
        }

        // 创建其他必要的画笔
        hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray), &m_pBrushBoneInferred);
        hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_pBrushHandClosed);
        hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &m_pBrushHandOpen);
        hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue), &m_pBrushHandLasso);
    }

    return hr;
}

/// <summary>
/// Dispose Direct2d resources 
/// </summary>
void Application::DiscardDirect2DResources()
{
    SafeRelease(m_pRenderTarget);

    SafeRelease(m_pBrushJointTracked);
    SafeRelease(m_pBrushJointInferred);
    SafeRelease(m_pBrushBoneTracked);
    SafeRelease(m_pBrushBoneInferred);

    SafeRelease(m_pBrushHandClosed);
    SafeRelease(m_pBrushHandOpen);
    SafeRelease(m_pBrushHandLasso);
}

/// <summary>
/// Converts a body point to screen space
/// </summary>
/// <param name="bodyPoint">body point to tranform</param>
/// <param name="width">width (in pixels) of output buffer</param>
/// <param name="height">height (in pixels) of output buffer</param>
/// <returns>point in screen-space</returns>
D2D1_POINT_2F Application::BodyToScreen(const CameraSpacePoint& bodyPoint, int width, int height)
{
    // Calculate the body's position on the screen
    DepthSpacePoint depthPoint = {0};
    m_pCoordinateMapper->MapCameraPointToDepthSpace(bodyPoint, &depthPoint);

    float screenPointX = static_cast<float>(depthPoint.X * width) / cDepthWidth;
    float screenPointY = static_cast<float>(depthPoint.Y * height) / cDepthHeight;

    return D2D1::Point2F(screenPointX, screenPointY);
}

/// <summary>
/// Draws a body 
/// </summary>
/// <param name="pJoints">joint data</param>
/// <param name="pJointPoints">joint positions converted to screen space</param>
void Application::DrawBody(const Joint* pJoints, const D2D1_POINT_2F* pJointPoints)
{
    if (!m_pRenderTarget || !m_pBrushJointTracked || !m_pBrushJointInferred || 
        !m_pBrushBoneTracked || !m_pBrushBoneInferred) {
        LOG_E("Required D2D resources are NULL");
        return;
    }

    // Draw the bones

    // Torso
    DrawBone(pJoints, pJointPoints, JointType_Head, JointType_Neck);
    DrawBone(pJoints, pJointPoints, JointType_Neck, JointType_SpineShoulder);
    DrawBone(pJoints, pJointPoints, JointType_SpineShoulder, JointType_SpineMid);
    DrawBone(pJoints, pJointPoints, JointType_SpineMid, JointType_SpineBase);
    DrawBone(pJoints, pJointPoints, JointType_SpineShoulder, JointType_ShoulderRight);
    DrawBone(pJoints, pJointPoints, JointType_SpineShoulder, JointType_ShoulderLeft);
    DrawBone(pJoints, pJointPoints, JointType_SpineBase, JointType_HipRight);
    DrawBone(pJoints, pJointPoints, JointType_SpineBase, JointType_HipLeft);
    
    // Right Arm    
    DrawBone(pJoints, pJointPoints, JointType_ShoulderRight, JointType_ElbowRight);
    DrawBone(pJoints, pJointPoints, JointType_ElbowRight, JointType_WristRight);
    DrawBone(pJoints, pJointPoints, JointType_WristRight, JointType_HandRight);
    DrawBone(pJoints, pJointPoints, JointType_HandRight, JointType_HandTipRight);
    DrawBone(pJoints, pJointPoints, JointType_WristRight, JointType_ThumbRight);

    // Left Arm
    DrawBone(pJoints, pJointPoints, JointType_ShoulderLeft, JointType_ElbowLeft);
    DrawBone(pJoints, pJointPoints, JointType_ElbowLeft, JointType_WristLeft);
    DrawBone(pJoints, pJointPoints, JointType_WristLeft, JointType_HandLeft);
    DrawBone(pJoints, pJointPoints, JointType_HandLeft, JointType_HandTipLeft);
    DrawBone(pJoints, pJointPoints, JointType_WristLeft, JointType_ThumbLeft);

    // Right Leg
    DrawBone(pJoints, pJointPoints, JointType_HipRight, JointType_KneeRight);
    DrawBone(pJoints, pJointPoints, JointType_KneeRight, JointType_AnkleRight);
    DrawBone(pJoints, pJointPoints, JointType_AnkleRight, JointType_FootRight);

    // Left Leg
    DrawBone(pJoints, pJointPoints, JointType_HipLeft, JointType_KneeLeft);
    DrawBone(pJoints, pJointPoints, JointType_KneeLeft, JointType_AnkleLeft);
    DrawBone(pJoints, pJointPoints, JointType_AnkleLeft, JointType_FootLeft);

    // Draw the joints
    for (int i = 0; i < JointType_Count; ++i)
    {
        D2D1_ELLIPSE ellipse = D2D1::Ellipse(pJointPoints[i], c_JointThickness, c_JointThickness);

        if (pJoints[i].TrackingState == TrackingState_Inferred)
        {
            m_pRenderTarget->FillEllipse(ellipse, m_pBrushJointInferred);
        }
        else if (pJoints[i].TrackingState == TrackingState_Tracked)
        {
            m_pRenderTarget->FillEllipse(ellipse, m_pBrushJointTracked);
        }
    }
}

/// <summary>
/// Draws one bone of a body (joint to joint)
/// </summary>
/// <param name="pJoints">joint data</param>
/// <param name="pJointPoints">joint positions converted to screen space</param>
/// <param name="pJointPoints">joint positions converted to screen space</param>
/// <param name="joint0">one joint of the bone to draw</param>
/// <param name="joint1">other joint of the bone to draw</param>
void Application::DrawBone(const Joint* pJoints, const D2D1_POINT_2F* pJointPoints, JointType joint0, JointType joint1)
{
    if (!m_pRenderTarget || !m_pBrushBoneTracked || !m_pBrushBoneInferred) {
        LOG_E("Required D2D resources are NULL");
        return;
    }

    TrackingState joint0State = pJoints[joint0].TrackingState;
    TrackingState joint1State = pJoints[joint1].TrackingState;

    // If we can't find either of these joints, exit
    if ((joint0State == TrackingState_NotTracked) || (joint1State == TrackingState_NotTracked))
    {
        return;
    }

    // Don't draw if both points are inferred
    if ((joint0State == TrackingState_Inferred) && (joint1State == TrackingState_Inferred))
    {
        return;
    }

    // We assume all drawn bones are inferred unless BOTH joints are tracked
    if ((joint0State == TrackingState_Tracked) && (joint1State == TrackingState_Tracked))
    {
        m_pRenderTarget->DrawLine(pJointPoints[joint0], pJointPoints[joint1], m_pBrushBoneTracked, c_TrackedBoneThickness);
    }
    else
    {
        m_pRenderTarget->DrawLine(pJointPoints[joint0], pJointPoints[joint1], m_pBrushBoneInferred, c_InferredBoneThickness);
    }
}

/// <summary>
/// Draws a hand symbol if the hand is tracked: red circle = closed, green circle = opened; blue circle = lasso
/// </summary>
/// <param name="handState">state of the hand</param>
/// <param name="handPosition">position of the hand</param>
void Application::DrawHand(HandState handState, const D2D1_POINT_2F& handPosition)
{
    D2D1_ELLIPSE ellipse = D2D1::Ellipse(handPosition, c_HandSize, c_HandSize);

    switch (handState)
    {
        case HandState_Closed:
            m_pRenderTarget->FillEllipse(ellipse, m_pBrushHandClosed);
            break;

        case HandState_Open:
            m_pRenderTarget->FillEllipse(ellipse, m_pBrushHandOpen);
            break;

        case HandState_Lasso:
            m_pRenderTarget->FillEllipse(ellipse, m_pBrushHandLasso);
            break;
    }
}
