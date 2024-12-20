#include "../../include/render/SkeletonRenderer.h"
#include "../../include/log/KFLog.h"

namespace kf {
namespace render {

SkeletonRenderer::SkeletonRenderer()
    : m_pRenderTarget(nullptr)
    , m_pBrushJointTracked(nullptr)
    , m_pBrushJointInferred(nullptr)
    , m_pBrushBoneTracked(nullptr)
    , m_pBrushBoneInferred(nullptr)
    , m_pBrushHandClosed(nullptr)
    , m_pBrushHandOpen(nullptr)
    , m_pBrushHandLasso(nullptr) {
}

SkeletonRenderer::~SkeletonRenderer() {
    release();
}

bool SkeletonRenderer::initialize(ID2D1HwndRenderTarget* pRenderTarget) {
    if (!pRenderTarget) {
        LOG_E("渲染目标为空");
        return false;
    }

    release();
    m_pRenderTarget = pRenderTarget;
    createBrushes();
    return true;
}

void SkeletonRenderer::release() {
    releaseBrushes();
    m_pRenderTarget = nullptr;
}

void SkeletonRenderer::drawSkeleton(const Joint* pJoints, const JointOrientation* pJointOrientations) {
    if (!m_pRenderTarget || !pJoints) {
        return;
    }

    // 绘制骨骼连接
    drawBone(pJoints[JointType_Head], pJoints[JointType_Neck]);
    drawBone(pJoints[JointType_Neck], pJoints[JointType_SpineShoulder]);
    drawBone(pJoints[JointType_SpineShoulder], pJoints[JointType_SpineMid]);
    drawBone(pJoints[JointType_SpineMid], pJoints[JointType_SpineBase]);
    drawBone(pJoints[JointType_SpineShoulder], pJoints[JointType_ShoulderRight]);
    drawBone(pJoints[JointType_SpineShoulder], pJoints[JointType_ShoulderLeft]);
    drawBone(pJoints[JointType_ShoulderRight], pJoints[JointType_ElbowRight]);
    drawBone(pJoints[JointType_ElbowRight], pJoints[JointType_WristRight]);
    drawBone(pJoints[JointType_WristRight], pJoints[JointType_HandRight]);
    drawBone(pJoints[JointType_HandRight], pJoints[JointType_HandTipRight]);
    drawBone(pJoints[JointType_WristRight], pJoints[JointType_ThumbRight]);
    drawBone(pJoints[JointType_ShoulderLeft], pJoints[JointType_ElbowLeft]);
    drawBone(pJoints[JointType_ElbowLeft], pJoints[JointType_WristLeft]);
    drawBone(pJoints[JointType_WristLeft], pJoints[JointType_HandLeft]);
    drawBone(pJoints[JointType_HandLeft], pJoints[JointType_HandTipLeft]);
    drawBone(pJoints[JointType_WristLeft], pJoints[JointType_ThumbLeft]);
    drawBone(pJoints[JointType_SpineBase], pJoints[JointType_HipRight]);
    drawBone(pJoints[JointType_SpineBase], pJoints[JointType_HipLeft]);
    drawBone(pJoints[JointType_HipRight], pJoints[JointType_KneeRight]);
    drawBone(pJoints[JointType_KneeRight], pJoints[JointType_AnkleRight]);
    drawBone(pJoints[JointType_AnkleRight], pJoints[JointType_FootRight]);
    drawBone(pJoints[JointType_HipLeft], pJoints[JointType_KneeLeft]);
    drawBone(pJoints[JointType_KneeLeft], pJoints[JointType_AnkleLeft]);
    drawBone(pJoints[JointType_AnkleLeft], pJoints[JointType_FootLeft]);

    // 绘制关节点
    for (int i = 0; i < JointType_Count; ++i) {
        drawJoint(pJoints[i]);
    }
}

void SkeletonRenderer::drawJoint(const Joint& joint) {
    if (!m_pRenderTarget) {
        return;
    }

    D2D1_ELLIPSE ellipse = D2D1::Ellipse(
        D2D1::Point2F(joint.Position.X, joint.Position.Y),
        JOINT_RADIUS,
        JOINT_RADIUS
    );

    ID2D1SolidColorBrush* pBrush = (joint.TrackingState == TrackingState_Tracked) ?
        m_pBrushJointTracked : m_pBrushJointInferred;

    m_pRenderTarget->FillEllipse(ellipse, pBrush);
}

void SkeletonRenderer::drawBone(const Joint& joint0, const Joint& joint1) {
    if (!m_pRenderTarget) {
        return;
    }

    TrackingState trackingState0 = joint0.TrackingState;
    TrackingState trackingState1 = joint1.TrackingState;

    // 如果两个关节都没有被跟踪到，则不绘制
    if (trackingState0 == TrackingState_NotTracked || trackingState1 == TrackingState_NotTracked) {
        return;
    }

    D2D1_POINT_2F point0 = D2D1::Point2F(joint0.Position.X, joint0.Position.Y);
    D2D1_POINT_2F point1 = D2D1::Point2F(joint1.Position.X, joint1.Position.Y);

    // 选择画笔
    ID2D1SolidColorBrush* pBrush = m_pBrushBoneTracked;
    if (trackingState0 == TrackingState_Inferred || trackingState1 == TrackingState_Inferred) {
        pBrush = m_pBrushBoneInferred;
    }

    m_pRenderTarget->DrawLine(point0, point1, pBrush, BONE_THICKNESS);
}

void SkeletonRenderer::drawHand(HandState handState, const Joint& joint) {
    if (!m_pRenderTarget) {
        return;
    }

    D2D1_ELLIPSE ellipse = D2D1::Ellipse(
        D2D1::Point2F(joint.Position.X, joint.Position.Y),
        HAND_RADIUS,
        HAND_RADIUS
    );

    ID2D1SolidColorBrush* pBrush = nullptr;
    switch (handState) {
        case HandState_Closed:
            pBrush = m_pBrushHandClosed;
            break;
        case HandState_Open:
            pBrush = m_pBrushHandOpen;
            break;
        case HandState_Lasso:
            pBrush = m_pBrushHandLasso;
            break;
        default:
            return;
    }

    m_pRenderTarget->FillEllipse(ellipse, pBrush);
}

void SkeletonRenderer::createBrushes() {
    if (!m_pRenderTarget) {
        return;
    }

    // 创建关节画笔
    m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 1.0f, 0.0f), &m_pBrushJointTracked);
    m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.5f, 0.0f), &m_pBrushJointInferred);

    // 创建骨骼画笔
    m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f), &m_pBrushBoneTracked);
    m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.5f, 0.5f, 0.5f), &m_pBrushBoneInferred);

    // 创建手势画笔
    m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.0f, 0.0f), &m_pBrushHandClosed);
    m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 1.0f, 0.0f), &m_pBrushHandOpen);
    m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 1.0f), &m_pBrushHandLasso);
}

void SkeletonRenderer::releaseBrushes() {
    if (m_pBrushJointTracked) { m_pBrushJointTracked->Release(); m_pBrushJointTracked = nullptr; }
    if (m_pBrushJointInferred) { m_pBrushJointInferred->Release(); m_pBrushJointInferred = nullptr; }
    if (m_pBrushBoneTracked) { m_pBrushBoneTracked->Release(); m_pBrushBoneTracked = nullptr; }
    if (m_pBrushBoneInferred) { m_pBrushBoneInferred->Release(); m_pBrushBoneInferred = nullptr; }
    if (m_pBrushHandClosed) { m_pBrushHandClosed->Release(); m_pBrushHandClosed = nullptr; }
    if (m_pBrushHandOpen) { m_pBrushHandOpen->Release(); m_pBrushHandOpen = nullptr; }
    if (m_pBrushHandLasso) { m_pBrushHandLasso->Release(); m_pBrushHandLasso = nullptr; }
}

} // namespace render
} // namespace kf