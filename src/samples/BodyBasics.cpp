//------------------------------------------------------------------------------
// <copyright file="BodyBasics.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "samples/BodyBasics.h"
//#pragma comment(lib, "Dwrite.lib")

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
    m_pBrushHandLasso(NULL),
    m_pColorFrameReader(NULL),
    m_pColorRenderer(NULL),
    m_pColorRGBX(NULL),
    m_pColorBitmap(nullptr),
    m_colorBitmapSize({0, 0}),
    m_isRecording(false),
    m_isCalcing(false),
    m_isPlayingTemplate(false),
    m_playbackStartTime(0),
    m_currentFrameIndex(0),
    m_pBrushJointTemplate(nullptr),
    m_pBrushBoneTemplate(nullptr),
    c_BoneThickness(4.0f),
    m_fCurrentSimilarity(0.0f)
{
    LARGE_INTEGER qpf = {0};
    if (QueryPerformanceFrequency(&qpf))
    {
        m_fFreq = double(qpf.QuadPart);
    }

    /*HRESULT hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
    );
    if (FAILED(hr)) {
        LOG_E("Failed to create DirectWrite Factory");
    }*/
    /*if (SUCCEEDED(hr) && m_pRenderTarget && !m_pBrush) {
        hr = m_pRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Green), &m_pBrush);
        if (FAILED(hr)) {
            LOG_E("Failed to create solid color brush");
        }
    }*/

    // 创建颜色缓冲区
    m_pColorRGBX = new RGBQUAD[cColorWidth * cColorHeight];
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

    // 清理颜色相关资源
    if (m_pColorRenderer)
    {
        delete m_pColorRenderer;
        m_pColorRenderer = NULL;
    }

    if (m_pColorRGBX)
    {
        delete[] m_pColorRGBX;
        m_pColorRGBX = NULL;
    }

    SafeRelease(m_pColorFrameReader);
    SafeRelease(m_pColorBitmap);
}

void Application::HandlePaint()
{
    if (!m_pBodyFrameReader || !m_hWnd) {
        return;
    }
    HRESULT hr = EnsureDirect2DResources();
    if (FAILED(hr) || !m_pRenderTarget) {
        return;
    }

    // 获取当前时间
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);

    // 计算离上次绘制的时间间隔
    double deltaTime = (currentTime.QuadPart - m_nLastCounter) / m_fFreq;
    // 使用固定的帧率控制
    const double targetFrameTime = 1.0 / 60.0;  // 目标60fps
    if (deltaTime < targetFrameTime) {
        //Sleep(1);  // 短暂休眠以减少 CPU 使用率
        return;
    }
    // 开始绘制
    m_pRenderTarget->BeginDraw();
    // m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black)); // 画面闪烁的原因在这
    Update();

    if (m_isRecording) {
        // 使用 Direct2D 绘制 "Recording..."
        WCHAR recordingText[] = L"Recording...";
        IDWriteTextFormat* pTextFormat = nullptr;
        HRESULT hr = m_pDWriteFactory->CreateTextFormat(
            L"Arial", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL, 30.0f, L"en-us", &pTextFormat);

        if (SUCCEEDED(hr)) {
            m_pRenderTarget->DrawText(
                recordingText, wcslen(recordingText),
                pTextFormat,
                D2D1::RectF(10.0f, 10.0f, 300.0f, 50.0f), // 确定文本显示的区域
                m_pBrush); // 使用现有画刷绘制文本
        }
        SafeRelease(pTextFormat);
    }

    if (m_isCalcing) {
        // 绘制相似度文本
        WCHAR similarityText[64];
        swprintf_s(similarityText, L"Similarity: %.2f%%", m_fCurrentSimilarity * 100.0f);

        IDWriteTextFormat* pTextFormat = nullptr;
        hr = m_pDWriteFactory->CreateTextFormat(
            L"Arial", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL, 22.0f, L"en-us", &pTextFormat);

        if (SUCCEEDED(hr)) {
            m_pRenderTarget->DrawText(
                similarityText, wcslen(similarityText),
                pTextFormat,
                D2D1::RectF(10.0f, 50.0f, 300.0f, 100.0f),
                m_pBrush);
        }
        SafeRelease(pTextFormat);
    }

    hr = m_pRenderTarget->EndDraw();

    if (hr == D2DERR_RECREATE_TARGET) {
        hr = S_OK;
        DiscardDirect2DResources();
        EnsureDirect2DResources();  // 立即重建资源
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
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // new
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
        kf::window_width, kf::window_height,
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
        (HMENU)IDC_VIDEOVIEW,
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
    if (!m_pColorFrameReader || !m_pBodyFrameReader)
    {
        return;
    }

    // 首先处理颜色帧
    IColorFrame* pColorFrame = NULL;
    HRESULT hr = m_pColorFrameReader->AcquireLatestFrame(&pColorFrame);
    if (SUCCEEDED(hr))
    {
        INT64 nTime = 0;
        IFrameDescription* pFrameDescription = NULL;
        int nWidth = 0;
        int nHeight = 0;
        ColorImageFormat imageFormat = ColorImageFormat_None;
        UINT nBufferSize = 0;
        RGBQUAD *pBuffer = NULL;

        hr = pColorFrame->get_RelativeTime(&nTime);

        if (SUCCEEDED(hr))
        {
            hr = pColorFrame->get_FrameDescription(&pFrameDescription);
        }

        if (SUCCEEDED(hr))
        {
            hr = pFrameDescription->get_Width(&nWidth);
        }

        if (SUCCEEDED(hr))
        {
            hr = pFrameDescription->get_Height(&nHeight);
        }

        if (SUCCEEDED(hr))
        {
            hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
        }

        if (SUCCEEDED(hr))
        {
            if (imageFormat == ColorImageFormat_Bgra)
            {
                hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize, reinterpret_cast<BYTE**>(&pBuffer));
            }
            else
            {
                hr = pColorFrame->CopyConvertedFrameDataToArray(
                    cColorWidth * cColorHeight * sizeof(RGBQUAD),
                    reinterpret_cast<BYTE*>(m_pColorRGBX),
                    ColorImageFormat_Bgra
                );
                pBuffer = m_pColorRGBX;
            }
        }

        if (SUCCEEDED(hr))
        {
            // 先绘制颜色帧
            ProcessColor(nTime, pBuffer, nWidth, nHeight);
        }

        SafeRelease(pFrameDescription);
    }
    SafeRelease(pColorFrame);

    // 然后处理骨骼帧
    IBodyFrame* pBodyFrame = NULL;
    hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);
    if (SUCCEEDED(hr))
    {
        INT64 nTime = 0;
        hr = pBodyFrame->get_RelativeTime(&nTime);
        IBody* ppBodies[BODY_COUNT] = {0};

        if (SUCCEEDED(hr)) {
            hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
        }

        if (SUCCEEDED(hr)) {
            ProcessBody(nTime, BODY_COUNT, ppBodies);
        }

        for (int i = 0; i < _countof(ppBodies); ++i) {
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
    HRESULT hr = GetDefaultKinectSensor(&m_pKinectSensor);
    if (FAILED(hr)) {
        return hr;
    }

    if (m_pKinectSensor) {
        // 初始化 Kinect
        hr = m_pKinectSensor->Open();

        if (SUCCEEDED(hr)) {
            // 获取坐标映射器
            hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
        }

        // 初始化骨骼帧源
        if (SUCCEEDED(hr)) {
            IBodyFrameSource* pBodyFrameSource = NULL;
            hr = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
            if (SUCCEEDED(hr)) {
                hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
            }
            SafeRelease(pBodyFrameSource);
        }

        // 初始化颜色帧源
        if (SUCCEEDED(hr)) {
            IColorFrameSource* pColorFrameSource = NULL;
            hr = m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource);
            if (SUCCEEDED(hr)) {
                hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);
            }
            SafeRelease(pColorFrameSource);
        }
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
void Application::ProcessBody(INT64 nTime, int nBodyCount, IBody** ppBodies) {
    if (!m_pRenderTarget || !m_isCalcing) {
        return;
    }

    // 获取窗口大小
    RECT rct;
    GetClientRect(GetDlgItem(m_hWnd, IDC_VIDEOVIEW), &rct);
    int width = rct.right;
    int height = rct.bottom;

    static kf::ActionBuffer actionBuffer(ACTION_BUFFER_SIZE);  // 动作缓冲区
    static std::future<void> comparisonFuture;                // 异步计算相似度任务
    static INT64 lastRecordedTime = 0;                        // 上次记录时间戳
    static std::mutex recordMutex;                            // 记录互斥锁
    static INT64 lastCompareTime = 0;                         // 上次比较时间戳

    // 首先绘制标准动作（如果存在且正在计算相似度）
    if (m_isCalcing && kf::g_actionTemplate) {
        std::lock_guard<std::mutex> lock(kf::templateMutex);
        const auto& frames = kf::g_actionTemplate->getFrames();
        
        // 只有在播放状态时才显示标准动作
        if (!frames.empty() && m_isPlayingTemplate) {
            // 初始化播放起点时间
            if (m_playbackStartTime == 0) {
                m_playbackStartTime = nTime;
            }

            // 使用recordInterval来控制播放速度
            INT64 elapsedTime = nTime - m_playbackStartTime;
            size_t frameIndex = (elapsedTime / kf::recordInterval) % frames.size();
            
            const auto& templateFrame = frames[frameIndex];
            
            // 准备关节点数据
            D2D1_POINT_2F templateJointPoints[JointType_Count];
            
            // 将模板骨骼数据转换为屏幕坐标
            for (const auto& joint : templateFrame.joints) {
                templateJointPoints[joint.type] = BodyToScreen(joint.position, width, height);
            }
            
            // 绘制模板骨骼
            DrawTemplateBody(templateFrame.joints.data(), templateJointPoints);
        }
    }

    if (m_isRecording) {
        // 创建录制文件名
        if (m_recordFilePath.empty()) {
            m_recordFilePath = kf::generateRecordingPath();
            LOG_I("Started recording to file: {}", m_recordFilePath);
        }
    }

    // 遍历每个捕捉到的身体
    for (int i = 0; i < nBodyCount; ++i) {
        IBody* pBody = ppBodies[i];
        if (pBody) {
            BOOLEAN bTracked = false;
            HRESULT hr = pBody->get_IsTracked(&bTracked);

            if (SUCCEEDED(hr) && bTracked) {
                Joint joints[JointType_Count];
                D2D1_POINT_2F jointPoints[JointType_Count] = {};
                HandState leftHandState = HandState_Unknown;
                HandState rightHandState = HandState_Unknown;

                pBody->get_HandLeftState(&leftHandState);
                pBody->get_HandRightState(&rightHandState);

                hr = pBody->GetJoints(_countof(joints), joints);
                if (SUCCEEDED(hr)) {
                    // 转换为 FrameData 并添加到缓冲区
                    kf::FrameData frameData;
                    frameData.timestamp = nTime;

                    for (int j = 0; j < _countof(joints); ++j) {
                        kf::JointData data;
                        data.type = joints[j].JointType;
                        data.position = joints[j].Position;
                        data.trackingState = joints[j].TrackingState;
                        frameData.joints.push_back(data);
                        jointPoints[j] = BodyToScreen(joints[j].Position, width, height);
                    }

                    actionBuffer.addFrame(frameData); // 添加到动作缓冲区

                    // 定期计算相似度
                    if (nTime - lastCompareTime >= kf::recordInterval) {
                        lastCompareTime = nTime;
                        if (kf::g_actionTemplate) {
                            auto future = kf::compareActionAsync(actionBuffer);
                            m_fCurrentSimilarity = future.get();
                        }
                    }

                    // 绘制骨骼和手部状态
                    DrawBody(joints, jointPoints);
                    DrawHand(leftHandState, jointPoints[JointType_HandLeft]);
                    DrawHand(rightHandState, jointPoints[JointType_HandRight]);

                    // 序列化当前帧
                    if (m_isRecording && !m_recordFilePath.empty() && 
                        (nTime - lastRecordedTime >= kf::recordInterval)) {
                        lastRecordedTime = nTime;  // 更新上次记录时间

                        // 异步保存当前帧
                        std::async(std::launch::async, [this, frameData]() {
                            std::lock_guard<std::mutex> lock(recordMutex);
                            kf::SaveFrame(m_recordFilePath, frameData, true);
                        });
                    }
                }
            }
        }
    }
}


void Application::DrawRealtimeSkeletons(INT64 nTime, int nBodyCount, IBody** ppBodies)
{
}

void Application::PlayActionTemplate(INT64 nTime) {
    if (!m_isPlayingTemplate || !kf::g_actionTemplate) {
        return; // 未打开播放或模板未加载
    }

    std::lock_guard<std::mutex> lock(kf::templateMutex); // 确保线程安全
    const auto& actionTemplate = *kf::g_actionTemplate;

    const auto& actionFrames = actionTemplate.getFrames();
    if (actionFrames.empty()) {
        return; // 无有效帧
    }

    // 初始化播放起点时间
    if (m_playbackStartTime == 0) {
        m_playbackStartTime = nTime;
    }

    // 使用recordInterval来控制播放速度
    INT64 elapsedTime = nTime - m_playbackStartTime;
    size_t frameIndex = (elapsedTime / kf::recordInterval) % actionTemplate.getFrameCount();

    // 如果当前帧发生变化，更新当前帧并绘制
    if (frameIndex != m_currentFrameIndex) {
        m_currentFrameIndex = frameIndex;
        const auto& frameData = actionFrames[m_currentFrameIndex];

        // 准备骨架点
        D2D1_POINT_2F jointPoints[JointType_Count];
        for (const auto& jointData : frameData.joints) {
            jointPoints[jointData.type] = BodyToScreen(jointData.position, cColorWidth, cColorHeight);
        }

        // 定义骨骼连接关系
        static const std::vector<std::pair<JointType, JointType>> bonePairs = {
            {JointType_Head, JointType_Neck},
            {JointType_Neck, JointType_SpineShoulder},
            {JointType_SpineShoulder, JointType_SpineMid},
            {JointType_SpineMid, JointType_SpineBase},
            {JointType_SpineShoulder, JointType_ShoulderRight},
            {JointType_SpineShoulder, JointType_ShoulderLeft},
            {JointType_SpineBase, JointType_HipRight},
            {JointType_SpineBase, JointType_HipLeft},
            {JointType_ShoulderRight, JointType_ElbowRight},
            {JointType_ElbowRight, JointType_WristRight},
            {JointType_WristRight, JointType_HandRight},
            {JointType_ShoulderLeft, JointType_ElbowLeft},
            {JointType_ElbowLeft, JointType_WristLeft},
            {JointType_WristLeft, JointType_HandLeft},
            {JointType_HipRight, JointType_KneeRight},
            {JointType_KneeRight, JointType_AnkleRight},
            {JointType_AnkleRight, JointType_FootRight},
            {JointType_HipLeft, JointType_KneeLeft},
            {JointType_KneeLeft, JointType_AnkleLeft},
            {JointType_AnkleLeft, JointType_FootLeft},
        };

        // 绘制骨骼
        for (const auto& bone : bonePairs) {
            const auto& joint0 = frameData.joints[bone.first];
            const auto& joint1 = frameData.joints[bone.second];

            if (joint0.trackingState == TrackingState_NotTracked || joint1.trackingState == TrackingState_NotTracked) {
                continue; // 跳过未跟踪的骨骼
            }

            auto brush = (joint0.trackingState == TrackingState_Tracked && joint1.trackingState == TrackingState_Tracked)
                ? m_pBrushBoneTracked
                : m_pBrushBoneInferred;

            m_pRenderTarget->DrawLine(
                jointPoints[bone.first],
                jointPoints[bone.second],
                brush,
                2.0f
            );
        }

        // 绘制关节
        for (const auto& jointData : frameData.joints) {
            auto brush = (jointData.trackingState == TrackingState_Tracked)
                ? m_pBrushJointTracked
                : m_pBrushJointInferred;

            m_pRenderTarget->DrawEllipse(
                D2D1::Ellipse(jointPoints[jointData.type], 5.0f, 5.0f),
                brush
            );
        }
    }
}

void Application::DrawTemplateBody(const kf::JointData* pJoints, const D2D1_POINT_2F* pJointPoints)
{
    if (!m_pRenderTarget || !m_pBrushJointTemplate || !m_pBrushBoneTemplate) {
        LOG_E("Required template D2D resources are NULL");
        return;
    }

    // 绘制骨骼
    // Torso
    DrawTemplateBone(pJoints, pJointPoints, JointType_Head, JointType_Neck);
    DrawTemplateBone(pJoints, pJointPoints, JointType_Neck, JointType_SpineShoulder);
    DrawTemplateBone(pJoints, pJointPoints, JointType_SpineShoulder, JointType_SpineMid);
    DrawTemplateBone(pJoints, pJointPoints, JointType_SpineMid, JointType_SpineBase);
    DrawTemplateBone(pJoints, pJointPoints, JointType_SpineShoulder, JointType_ShoulderRight);
    DrawTemplateBone(pJoints, pJointPoints, JointType_SpineShoulder, JointType_ShoulderLeft);
    DrawTemplateBone(pJoints, pJointPoints, JointType_SpineBase, JointType_HipRight);
    DrawTemplateBone(pJoints, pJointPoints, JointType_SpineBase, JointType_HipLeft);

    // Right Arm
    DrawTemplateBone(pJoints, pJointPoints, JointType_ShoulderRight, JointType_ElbowRight);
    DrawTemplateBone(pJoints, pJointPoints, JointType_ElbowRight, JointType_WristRight);
    DrawTemplateBone(pJoints, pJointPoints, JointType_WristRight, JointType_HandRight);
    DrawTemplateBone(pJoints, pJointPoints, JointType_HandRight, JointType_HandTipRight);
    DrawTemplateBone(pJoints, pJointPoints, JointType_WristRight, JointType_ThumbRight);

    // Left Arm
    DrawTemplateBone(pJoints, pJointPoints, JointType_ShoulderLeft, JointType_ElbowLeft);
    DrawTemplateBone(pJoints, pJointPoints, JointType_ElbowLeft, JointType_WristLeft);
    DrawTemplateBone(pJoints, pJointPoints, JointType_WristLeft, JointType_HandLeft);
    DrawTemplateBone(pJoints, pJointPoints, JointType_HandLeft, JointType_HandTipLeft);
    DrawTemplateBone(pJoints, pJointPoints, JointType_WristLeft, JointType_ThumbLeft);

    // Right Leg
    DrawTemplateBone(pJoints, pJointPoints, JointType_HipRight, JointType_KneeRight);
    DrawTemplateBone(pJoints, pJointPoints, JointType_KneeRight, JointType_AnkleRight);
    DrawTemplateBone(pJoints, pJointPoints, JointType_AnkleRight, JointType_FootRight);

    // Left Leg
    DrawTemplateBone(pJoints, pJointPoints, JointType_HipLeft, JointType_KneeLeft);
    DrawTemplateBone(pJoints, pJointPoints, JointType_KneeLeft, JointType_AnkleLeft);
    DrawTemplateBone(pJoints, pJointPoints, JointType_AnkleLeft, JointType_FootLeft);

    // 绘制关节点
    for (int i = 0; i < JointType_Count; ++i) {
        D2D1_ELLIPSE ellipse = D2D1::Ellipse(pJointPoints[i], c_JointThickness, c_JointThickness);
        m_pRenderTarget->FillEllipse(ellipse, m_pBrushJointTemplate);
    }
}

void Application::DrawTemplateBone(const kf::JointData* pJoints, const D2D1_POINT_2F* pJointPoints, JointType joint0, JointType joint1)
{
    D2D1_POINT_2F joint0Position = pJointPoints[joint0];
    D2D1_POINT_2F joint1Position = pJointPoints[joint1];

    // 骨骼连接的状态检查（忽略推断的状态）
    if (pJoints[joint0].trackingState == TrackingState_NotTracked ||
        pJoints[joint1].trackingState == TrackingState_NotTracked) {
        return;
    }

    m_pRenderTarget->DrawLine(joint0Position, joint1Position, m_pBrushBoneTemplate, c_BoneThickness);
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

    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
    );
    if (FAILED(hr)) {
        LOG_E("Failed to create DirectWrite Factory");
    }

    if (!m_pRenderTarget) {
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

        hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &m_pBrush);
        if (FAILED(hr)) {
            LOG_E("Failed to create brush: 0x{:08X}", hr);
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

        // 创建标准动作的关节画笔（蓝色）
        hr = m_pRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Blue), // 蓝色
            &m_pBrushJointTemplate
        );

        if (FAILED(hr)) {
            LOG_E("Failed to create joint brush for template.");
            return hr;
        }

        // 创建标准动作的骨骼画刷（浅蓝色）
        hr = m_pRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::LightBlue), // 浅蓝色
            &m_pBrushBoneTemplate
        );

        if (FAILED(hr)) {
            LOG_E("Failed to create bone brush for template.");
            return hr;
        }
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

void Application::ProcessColor(INT64 nTime, RGBQUAD* pBuffer, int nWidth, int nHeight)
{
    if (!m_pRenderTarget || !pBuffer)
    {
        return;
    }

    // 使用类成员变量管理位图
    if (!m_pColorBitmap || 
        m_colorBitmapSize.width != nWidth || 
        m_colorBitmapSize.height != nHeight)
    {
        SafeRelease(m_pColorBitmap);
        m_colorBitmapSize = D2D1::SizeU(nWidth, nHeight);

        D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties(
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)
        );

        HRESULT hr = m_pRenderTarget->CreateBitmap(
            m_colorBitmapSize, 
            props, 
            &m_pColorBitmap
        );

        if (FAILED(hr) || !m_pColorBitmap)
        {
            return;
        }
    }

    // 更新位图数据
    D2D1_RECT_U updateRect = D2D1::RectU(0, 0, nWidth, nHeight);
    m_pColorBitmap->CopyFromMemory(&updateRect, pBuffer, nWidth * sizeof(RGBQUAD));

    // 获取渲染目标的大小
    D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

    // 计算目标矩形，保持纵横比
    float aspectRatio = static_cast<float>(nWidth) / nHeight;
    float targetWidth = rtSize.width;
    float targetHeight = rtSize.height;

    if (targetWidth / targetHeight > aspectRatio)
    {
        targetWidth = targetHeight * aspectRatio;
    }
    else
    {
        targetHeight = targetWidth / aspectRatio;
    }

    float x = (rtSize.width - targetWidth) / 2;
    float y = (rtSize.height - targetHeight) / 2;

    D2D1_RECT_F destRect = D2D1::RectF(x, y, x + targetWidth, y + targetHeight);

    // 绘制位图
    m_pRenderTarget->DrawBitmap(
        m_pColorBitmap,
        destRect,
        1.0f,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
    );
}
