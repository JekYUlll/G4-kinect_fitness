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
    m_fCurrentSimilarity(0.0f),
    m_similarityMutex(),
    m_similarityCV(),
    m_similarityUpdated(false)
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

    // ������ɫ������
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

    // ������ɫ�����Դ
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

    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    double deltaTime = (currentTime.QuadPart - m_nLastCounter) / m_fFreq;
    const double targetFrameTime = 1.0 / 60.0;  // Ŀ��60fps
    if (deltaTime < targetFrameTime) {
        return;
    }

    // ��ʼ����
    m_pRenderTarget->BeginDraw();
    
    // ���»������ݣ���������ɫ֡�Ļ��ƣ����Զ����������
    Update();

    // ����UI��Ϣ
    if (m_isRecording || m_isCalcing) {
        // �����ı���ʽ�����Ի������������������ܣ�
        static IDWriteTextFormat* pTextFormat = nullptr;
        if (!pTextFormat) {
            hr = m_pDWriteFactory->CreateTextFormat(
                L"Arial", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL, 30.0f, L"en-us", &pTextFormat);
            if (SUCCEEDED(hr) && pTextFormat) {
                pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
                pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
            }
        }

        if (SUCCEEDED(hr) && pTextFormat) {
            if (!m_pBrush) {
                hr = m_pRenderTarget->CreateSolidColorBrush(
                    D2D1::ColorF(D2D1::ColorF::White),
                    &m_pBrush
                );
            }

            if (SUCCEEDED(hr) && m_pBrush) {
                // ��ȡ��ǰ���ƶ�ֵ��ԭ�Ӳ�����
                float currentSimilarity = m_fCurrentSimilarity.load(std::memory_order_acquire);
                
                // �������ƶ��ı�
                WCHAR similarityText[64];
                swprintf_s(similarityText, L"Similarity: %.2f%%", currentSimilarity * 100.0f);
                
                // ������͸����ɫ����
                ID2D1SolidColorBrush* pBackgroundBrush = nullptr;
                hr = m_pRenderTarget->CreateSolidColorBrush(
                    D2D1::ColorF(D2D1::ColorF::Black, 0.5f),
                    &pBackgroundBrush
                );

                if (SUCCEEDED(hr) && pBackgroundBrush) {
                    // �������ƶȱ���
                    m_pRenderTarget->FillRectangle(
                        D2D1::RectF(5.0f, 45.0f, 305.0f, 85.0f),
                        pBackgroundBrush
                    );
                    // �������ƶ��ı�
                    m_pRenderTarget->DrawText(
                        similarityText, wcslen(similarityText),
                        pTextFormat,
                        D2D1::RectF(10.0f, 50.0f, 300.0f, 90.0f),
                        m_pBrush
                    );
                    SafeRelease(pBackgroundBrush);
                }
            }
        }
    }

    // ��������
    hr = m_pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        hr = S_OK;
        DiscardDirect2DResources();
        EnsureDirect2DResources();
    }

    // �����ϴλ���ʱ��
    m_nLastCounter = currentTime.QuadPart;
}

/// <summary>
/// Creates the main window and begins processing
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
int Application::Run(HINSTANCE hInstance, int nCmdShow)
{
    MSG msg = {0};
    WNDCLASS wc;

    // Dialog custom window class
    ZeroMemory(&wc, sizeof(wc));
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.cbWndExtra    = DLGWINDOWEXTRA;
    wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
    wc.hIcon         = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_APP));
    wc.lpfnWndProc   = kfc::WindowProc;
    wc.lpszClassName = L"BodyBasicsAppDlgWndClass";
    wc.hInstance     = hInstance;
    
    if (!RegisterClassW(&wc)) {
        LOG_E("Failed to Register Class: {}", GetLastError());
        return 0;
    }

    // �ȴ�������
    LOG_I("Creating main application window...");
    HWND hWndApp = CreateWindowExW(
        0,                              
        L"BodyBasicsAppDlgWndClass",   
        L"Kinect Fitness",             
        WS_OVERLAPPEDWINDOW,           
        CW_USEDEFAULT, CW_USEDEFAULT,  
        kfc::window_width, kfc::window_height,
        NULL,                          
        NULL,                          
        hInstance,                     
        this                           
    );

    if (!hWndApp) {
        LOG_E("Failed to create main window");
        return 0;
    }

    // �洢���ھ��
    m_hWnd = hWndApp;

    // ��ʼ�� Direct2D �� Kinect
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
    if (FAILED(hr)) {
        LOG_E("Failed to create D2D factory: 0x{:08X}", hr);
        return 0;
    }

    if (FAILED(InitializeDefaultSensor())) {
        LOG_E("Failed to initialize Kinect sensor");
        return 0;
    }

    // ��������D2D���Ƶ��Ӵ���
    HWND hWndVideo = CreateWindowExW(
        0,
        L"STATIC",
        NULL,
        WS_CHILD | WS_VISIBLE | SS_BLACKRECT,
        10, 50,
        780, 500,
        hWndApp,
        (HMENU)IDC_VIDEOVIEW,
        hInstance,
        NULL
    );

    if (!hWndVideo) {
        LOG_E("Failed to create video window");
        return 0;
    }

    // ��ʾ����
    ShowWindow(hWndApp, nCmdShow);
    UpdateWindow(hWndApp);

    // ʹ�ø߾��ȶ�ʱ��
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    LARGE_INTEGER lastTime;
    QueryPerformanceCounter(&lastTime);
    const double frameTime = 1.0 / 60.0;

    // ����Ϣѭ��
    while (WM_QUIT != msg.message) {
        if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }

            if (!IsDialogMessageW(hWndApp, &msg)) {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
            continue;
        }

        // ֡�����߼�
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
        double deltaTime = (currentTime.QuadPart - lastTime.QuadPart) / (double)freq.QuadPart;

        if (deltaTime >= frameTime) {
            HandlePaint();
            lastTime = currentTime;

            if (deltaTime > frameTime * 3) {
                lastTime = currentTime;
            }
        } else {
            Sleep(1);
        }
    }

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

    // ���ȴ�����ɫ֡
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
            // �Ȼ�����ɫ֡
            ProcessColor(nTime, pBuffer, nWidth, nHeight);
        }

        SafeRelease(pFrameDescription);
    }
    SafeRelease(pColorFrame);

    // Ȼ�������֡
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
                // >>> ��Ӹ��жϣ����Գ�ʼ��
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
                
                // ֻ�е���С��ĸı�ʱ�ŵ���
                D2D1_SIZE_U currentSize = m_pRenderTarget->GetPixelSize();
                if (size.width != currentSize.width || size.height != currentSize.height)
                {
                    m_pRenderTarget->Resize(size);
                    InvalidateRect(GetDlgItem(m_hWnd, IDC_VIDEOVIEW), NULL, FALSE);
                }
            }
            return 0;

        case WM_TIMER:
            if (wParam == 1)  // ���ǵĸ��¶�ʱ��
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
        // ��ʼ�� Kinect
        hr = m_pKinectSensor->Open();

        if (SUCCEEDED(hr)) {
            // ��ȡ����ӳ����
            hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
        }

        // ��ʼ������֡Դ
        if (SUCCEEDED(hr)) {
            IBodyFrameSource* pBodyFrameSource = NULL;
            hr = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
            if (SUCCEEDED(hr)) {
                hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
            }
            SafeRelease(pBodyFrameSource);
        }

        // ��ʼ����ɫ֡Դ
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

    // ��ȡ���ڴ�С
    RECT rct;
    GetClientRect(GetDlgItem(m_hWnd, IDC_VIDEOVIEW), &rct);
    int width = rct.right;
    int height = rct.bottom;

    static kfc::ActionBuffer actionBuffer(ACTION_BUFFER_SIZE);  // ����������
    static std::future<float> similarityFuture;                // �첽�������ƶ�����
    static std::vector<std::future<void>> saveFutures;         // ����֡��future�б�
    static INT64 lastRecordedTime = 0;                        // �ϴμ�¼ʱ���
    static std::mutex recordMutex;                            // ��¼������
    static INT64 lastCompareTime = 0;                         // �ϴαȽ�ʱ���
    static bool needsUpdate = false;                          // �Ƿ���Ҫ������ʾ

    // ��������ɵı�������
    saveFutures.erase(
        std::remove_if(
            saveFutures.begin(), 
            saveFutures.end(),
            [](std::future<void>& f) {
                return f.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
            }
        ),
        saveFutures.end()
    );

    // ���Ȼ��Ʊ�׼������������������ڼ������ƶȣ�
    if (m_isCalcing && kfc::g_actionTemplate) {
        std::lock_guard<std::mutex> lock(kfc::templateMutex);
        const auto& frames = kfc::g_actionTemplate->getFrames();
        
        // ֻ���ڲ���״̬ʱ����ʾ��׼����
        if (!frames.empty() && m_isPlayingTemplate) {
            // ��ʼ���������ʱ��
            if (m_playbackStartTime == 0) {
                m_playbackStartTime = nTime;
            }

            // ʹ��recordInterval�����Ʋ����ٶ�
            INT64 elapsedTime = nTime - m_playbackStartTime;
            size_t frameIndex = (elapsedTime / kfc::recordInterval) % frames.size();
            
            const auto& templateFrame = frames[frameIndex];
            
            // ׼���ؽڵ�����
            D2D1_POINT_2F templateJointPoints[JointType_Count];
            
            // ��ģ���������ת��Ϊ��Ļ����
            for (const auto& joint : templateFrame.joints) {
                templateJointPoints[joint.type] = BodyToScreen(joint.position, width, height);
            }
            
            // ����ģ�����
            DrawTemplateBody(templateFrame.joints.data(), templateJointPoints);
        }
    }

    if (m_isRecording) {
        // ����¼���ļ���
        if (m_recordFilePath.empty()) {
            m_recordFilePath = kfc::generateRecordingPath();
            LOG_I("Started recording to file: {}", m_recordFilePath);
        }
    }

    // ����ÿ����׽��������
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
                    // ת��Ϊ FrameData ����ӵ�������
                    kfc::FrameData frameData;
                    frameData.timestamp = nTime;

                    for (int j = 0; j < _countof(joints); ++j) {
                        kfc::JointData data;
                        data.type = joints[j].JointType;
                        data.position = joints[j].Position;
                        data.trackingState = joints[j].TrackingState;
                        frameData.joints.push_back(data);
                        jointPoints[j] = BodyToScreen(joints[j].Position, width, height);
                    }

                    actionBuffer.addFrame(frameData); // ��ӵ�����������

                    // ���ڼ������ƶȣ��첽��ÿ��4�Σ�
                    const INT64 similarityInterval = 250 * 10000;  // 250ms = 1/4��
                    if (nTime - lastCompareTime >= similarityInterval) {
                        lastCompareTime = nTime;
                        if (kfc::g_actionTemplate) {
                            // �����һ�εļ����Ƿ����
                            if (!similarityFuture.valid() || 
                                similarityFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
                                // �������һ�εĽ�����Ȼ�ȡ��
                                if (similarityFuture.valid()) {
                                    float lastSimilarity = similarityFuture.get();
                                    if (std::abs(lastSimilarity - m_fCurrentSimilarity.load()) > 0.01f) {
                                        m_fCurrentSimilarity.store(lastSimilarity, std::memory_order_release);
                                        InvalidateRect(GetDlgItem(m_hWnd, IDC_VIDEOVIEW), NULL, FALSE);
                                    }
                                }
                                
                                // �����µ��첽����
                                auto bufferCopy = actionBuffer;  // �����������ĸ���
                                similarityFuture = std::async(
                                    std::launch::async,
                                    [](const kfc::ActionBuffer& buffer) {
                                        auto future = kfc::compareActionAsync(buffer);
                                        return future.get();  // �ȴ������ؽ��
                                    },
                                    std::move(bufferCopy)
                                );
                            }
                        }
                    }

                    // ���ƹ������ֲ�״̬
                    DrawBody(joints, jointPoints);
                    DrawHand(leftHandState, jointPoints[JointType_HandLeft]);
                    DrawHand(rightHandState, jointPoints[JointType_HandRight]);

                    // ���л���ǰ֡���첽��
                    if (m_isRecording && !m_recordFilePath.empty() && 
                        (nTime - lastRecordedTime >= kfc::recordInterval)) {
                        lastRecordedTime = nTime;  // �����ϴμ�¼ʱ��

                        // ���Ʋ����������������
                        if (saveFutures.size() < 5) {  // ���5��������������
                            saveFutures.push_back(
                                std::async(std::launch::async, [this, frameData]() {
                                    std::lock_guard<std::mutex> lock(recordMutex);
                                    kfc::SaveFrame(m_recordFilePath, frameData, true);
                                })
                            );
                        }
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
    if (!m_isPlayingTemplate || !kfc::g_actionTemplate) {
        return; // δ�򿪲��Ż�ģ��δ����
    }

    std::lock_guard<std::mutex> lock(kfc::templateMutex); // ȷ���̰߳�ȫ
    const auto& actionTemplate = *kfc::g_actionTemplate;

    const auto& actionFrames = actionTemplate.getFrames();
    if (actionFrames.empty()) {
        return; // ����Ч֡
    }

    // ��ʼ���������ʱ��
    if (m_playbackStartTime == 0) {
        m_playbackStartTime = nTime;
    }

    // ʹ��recordInterval�����Ʋ����ٶ�
    INT64 elapsedTime = nTime - m_playbackStartTime;
    size_t frameIndex = (elapsedTime / kfc::recordInterval) % actionTemplate.getFrameCount();

    // �����ǰ֡�����仯�����µ�ǰ֡������
    if (frameIndex != m_currentFrameIndex) {
        m_currentFrameIndex = frameIndex;
        const auto& frameData = actionFrames[m_currentFrameIndex];

        // ׼���Ǽܵ�
        D2D1_POINT_2F jointPoints[JointType_Count];
        for (const auto& jointData : frameData.joints) {
            jointPoints[jointData.type] = BodyToScreen(jointData.position, cColorWidth, cColorHeight);
        }

        // ����������ӹ�ϵ
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

        // ���ƹ���
        for (const auto& bone : bonePairs) {
            const auto& joint0 = frameData.joints[bone.first];
            const auto& joint1 = frameData.joints[bone.second];

            if (joint0.trackingState == TrackingState_NotTracked || joint1.trackingState == TrackingState_NotTracked) {
                continue; // ����δ���ٵĹ���
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

        // ���ƹؽ�
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

void Application::DrawTemplateBody(const kfc::JointData* pJoints, const D2D1_POINT_2F* pJointPoints)
{
    if (!m_pRenderTarget || !m_pBrushJointTemplate || !m_pBrushBoneTemplate) {
        LOG_E("Required template D2D resources are NULL");
        return;
    }

    // ���ƹ���
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

    // ���ƹؽڵ�
    for (int i = 0; i < JointType_Count; ++i) {
        D2D1_ELLIPSE ellipse = D2D1::Ellipse(pJointPoints[i], c_JointThickness, c_JointThickness);
        m_pRenderTarget->FillEllipse(ellipse, m_pBrushJointTemplate);
    }
}

void Application::DrawTemplateBone(const kfc::JointData* pJoints, const D2D1_POINT_2F* pJointPoints, JointType joint0, JointType joint1)
{
    D2D1_POINT_2F joint0Position = pJointPoints[joint0];
    D2D1_POINT_2F joint1Position = pJointPoints[joint1];

    // �������ӵ�״̬��飨�����ƶϵ�״̬��
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

    // ȷ�� D2D Factory ����
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

        // ȷ�����ڳߴ���Ч
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

        // ����������Դ
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

        // ����������Ҫ�Ļ���
        hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray), &m_pBrushBoneInferred);
        hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_pBrushHandClosed);
        hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &m_pBrushHandOpen);
        hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue), &m_pBrushHandLasso);

        // ������׼�����Ĺؽڻ��ʣ���ɫ��
        hr = m_pRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Blue), // ��ɫ
            &m_pBrushJointTemplate
        );

        if (FAILED(hr)) {
            LOG_E("Failed to create joint brush for template.");
            return hr;
        }

        // ������׼�����Ĺ�����ˢ��ǳ��ɫ��
        hr = m_pRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::LightBlue), // ǳ��ɫ
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

    // ʹ�����Ա��������λͼ
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

    // ����λͼ����
    D2D1_RECT_U updateRect = D2D1::RectU(0, 0, nWidth, nHeight);
    m_pColorBitmap->CopyFromMemory(&updateRect, pBuffer, nWidth * sizeof(RGBQUAD));

    // ��ȡ��ȾĿ��Ĵ�С
    D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

    // ����Ŀ����Σ������ݺ��
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

    // ����λͼ
    m_pRenderTarget->DrawBitmap(
        m_pColorBitmap,
        destRect,
        1.0f,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
    );
}
