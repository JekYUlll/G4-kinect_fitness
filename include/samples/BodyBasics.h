//------------------------------------------------------------------------------
// <copyright file="BodyBasics.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#ifndef BODYBASIC_H
#define BODYBASIC_H

#include <Windows.h>
#include <thread>
#include <vector>  // 添加 vector 头文件
#include <string>  // 添加 string 头文件

#include "stdafx.h"
#include <strsafe.h>
#include "resource.h"
#include "config.h"
#include "ui/KFui.h"
#include "samples/ImageRenderer.h"
#include <dwrite.h>  // 添加 DirectWrite 支持
#include "calc/serialize.hpp"  // 只保留这个必要的头文件



class Application
{
    static const int        cDepthWidth = 512;
    static const int        cDepthHeight = 424;
    static const int        cColorWidth = 1920;
    static const int        cColorHeight = 1080;

public:
    /// <summary>
    /// Constructor
    /// </summary>
    Application();

    /// <summary>
    /// Destructor
    /// </summary>
    ~Application();

    void HandlePaint();

    /// <summary>
    /// Handles window messages, passes most to the class instance to handle
    /// </summary>
    /// <param name="hWnd">window message is for</param>
    /// <param name="uMsg">message</param>
    /// <param name="wParam">message data</param>
    /// <param name="lParam">additional message data</param>
    /// <returns>result of message processing</returns>
    static LRESULT CALLBACK MessageRouter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /// <summary>
    /// Handle windows messages for a class instance
    /// </summary>
    /// <param name="hWnd">window message is for</param>
    /// <param name="uMsg">message</param>
    /// <param name="wParam">message data</param>
    /// <param name="lParam">additional message data</param>
    /// <returns>result of message processing</returns>
    LRESULT CALLBACK        DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /// <summary>
    /// Creates the main window and begins processing
    /// </summary>
    /// <param name="hInstance"></param>
    /// <param name="nCmdShow"></param>
    int                     Run(HINSTANCE hInstance, int nCmdShow);

    void HandleResize() {
        DiscardDirect2DResources();
    }

    void ProcessColor(INT64 nTime, RGBQUAD* pBuffer, int nWidth, int nHeight);

    // Getter 和 Setter for m_isRecording
    void SetRecording(bool isRecording) { m_isRecording = isRecording; }
    bool IsRecording() const { return m_isRecording; }

private:
    HWND                    m_hWnd;
    INT64                   m_nStartTime;
    INT64                   m_nLastCounter;
    double                  m_fFreq;
    INT64                   m_nNextStatusTime;
    DWORD                   m_nFramesSinceUpdate;

    // Current Kinect
    IKinectSensor* m_pKinectSensor;
    ICoordinateMapper* m_pCoordinateMapper;

    // Body reader
    IBodyFrameReader* m_pBodyFrameReader;

    // Direct2D
    ID2D1Factory* m_pD2DFactory;

    // Body/hand drawing
    ID2D1HwndRenderTarget* m_pRenderTarget;
    ID2D1SolidColorBrush* m_pBrushJointTracked;
    ID2D1SolidColorBrush* m_pBrushJointInferred;
    ID2D1SolidColorBrush* m_pBrushBoneTracked;
    ID2D1SolidColorBrush* m_pBrushBoneInferred;
    ID2D1SolidColorBrush* m_pBrushHandClosed;
    ID2D1SolidColorBrush* m_pBrushHandOpen;
    ID2D1SolidColorBrush* m_pBrushHandLasso;

    // Color Frame 相关
    IColorFrameReader*      m_pColorFrameReader;
    class ImageRenderer*    m_pColorRenderer;
    RGBQUAD*               m_pColorRGBX;
    
    // 颜色位图相关
    ID2D1Bitmap*           m_pColorBitmap;
    D2D1_SIZE_U           m_colorBitmapSize;

    // 录制状态相关
    bool m_isRecording;        // 是否正在录制
    std::string m_recordFilePath;  // 添加文件路径成员

    /// <summary>
    /// Main processing function
    /// </summary>
    void                    Update();

    /// <summary>
    /// Initializes the default Kinect sensor
    /// </summary>
    /// <returns>S_OK on success, otherwise failure code</returns>
    HRESULT                 InitializeDefaultSensor();

    /// <summary>
    /// Handle new body data
    /// <param name="nTime">timestamp of frame</param>
    /// <param name="nBodyCount">body data count</param>
    /// <param name="ppBodies">body data in frame</param>
    /// </summary>
    void                    ProcessBody(INT64 nTime, int nBodyCount, IBody** ppBodies);

    /// <summary>
    /// Set the status bar message
    /// </summary>
    /// <param name="szMessage">message to display</param>
    /// <param name="nShowTimeMsec">time in milliseconds for which to ignore future status messages</param>
    /// <param name="bForce">force status update</param>
    bool                    SetStatusMessage(_In_z_ WCHAR* szMessage, DWORD nShowTimeMsec, bool bForce);

    /// <summary>
    /// Ensure necessary Direct2d resources are created
    /// </summary>
    /// <returns>S_OK if successful, otherwise an error code</returns>
    HRESULT EnsureDirect2DResources();

    /// <summary>
    /// Dispose Direct2d resources 
    /// </summary>
    void DiscardDirect2DResources();

    /// <summary>
    /// Converts a body point to screen space
    /// </summary>
    /// <param name="bodyPoint">body point to tranform</param>
    /// <param name="width">width (in pixels) of output buffer</param>
    /// <param name="height">height (in pixels) of output buffer</param>
    /// <returns>point in screen-space</returns>
    D2D1_POINT_2F           BodyToScreen(const CameraSpacePoint& bodyPoint, int width, int height);

    /// <summary>
    /// Draws a body 
    /// </summary>
    /// <param name="pJoints">joint data</param>
    /// <param name="pJointPoints">joint positions converted to screen space</param>
    void                    DrawBody(const Joint* pJoints, const D2D1_POINT_2F* pJointPoints);

    /// <summary>
    /// Draws a hand symbol if the hand is tracked: red circle = closed, green circle = opened; blue circle = lasso
    /// </summary>
    /// <param name="handState">state of the hand</param>
    /// <param name="handPosition">position of the hand</param>
    void                    DrawHand(HandState handState, const D2D1_POINT_2F& handPosition);

    /// <summary>
    /// Draws one bone of a body (joint to joint)
    /// </summary>
    /// <param name="pJoints">joint data</param>
    /// <param name="pJointPoints">joint positions converted to screen space</param>
    /// <param name="pJointPoints">joint positions converted to screen space</param>
    /// <param name="joint0">one joint of the bone to draw</param>
    /// <param name="joint1">other joint of the bone to draw</param>
    void                    DrawBone(const Joint* pJoints, const D2D1_POINT_2F* pJointPoints, JointType joint0, JointType joint1);
};



#endif // !BODYBASIC_H

