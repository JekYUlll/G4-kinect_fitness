#include "render/SkeletonRenderer.h"
#include "log/KFLog.h"

namespace kf {
namespace render {

SkeletonRenderer::SkeletonRenderer()
    : m_pRenderTarget(nullptr)
    , m_pBrush(nullptr) {
}

SkeletonRenderer::~SkeletonRenderer() {
    if (m_pBrush) {
        m_pBrush->Release();
        m_pBrush = nullptr;
    }
}

bool SkeletonRenderer::initialize(ID2D1HwndRenderTarget* pRenderTarget) {
    if (!pRenderTarget) {
        return false;
    }

    m_pRenderTarget = pRenderTarget;

    // 创建画笔
    HRESULT hr = m_pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Green),
        &m_pBrush
    );

    return SUCCEEDED(hr);
}

void SkeletonRenderer::renderSkeleton(const FrameData& frameData) {
    if (!m_pRenderTarget || !m_pBrush) {
        return;
    }

    // 渲染所有关节
    for (const auto& joint : frameData.joints) {
        renderJoint(joint);
    }

    // 渲染骨骼连接
    // 定义骨骼连接关系
    static const std::pair<JointType, JointType> bones[] = {
        {JointType::Head, JointType::Neck},
        {JointType::Neck, JointType::SpineShoulder},
        {JointType::SpineShoulder, JointType::SpineMid},
        {JointType::SpineMid, JointType::SpineBase},
        {JointType::SpineShoulder, JointType::ShoulderRight},
        {JointType::ShoulderRight, JointType::ElbowRight},
        {JointType::ElbowRight, JointType::WristRight},
        {JointType::WristRight, JointType::HandRight},
        {JointType::HandRight, JointType::HandTipRight},
        {JointType::WristRight, JointType::ThumbRight},
        {JointType::SpineShoulder, JointType::ShoulderLeft},
        {JointType::ShoulderLeft, JointType::ElbowLeft},
        {JointType::ElbowLeft, JointType::WristLeft},
        {JointType::WristLeft, JointType::HandLeft},
        {JointType::HandLeft, JointType::HandTipLeft},
        {JointType::WristLeft, JointType::ThumbLeft},
        {JointType::SpineBase, JointType::HipRight},
        {JointType::HipRight, JointType::KneeRight},
        {JointType::KneeRight, JointType::AnkleRight},
        {JointType::AnkleRight, JointType::FootRight},
        {JointType::SpineBase, JointType::HipLeft},
        {JointType::HipLeft, JointType::KneeLeft},
        {JointType::KneeLeft, JointType::AnkleLeft},
        {JointType::AnkleLeft, JointType::FootLeft},
    };

    // 渲染所有骨骼连接
    for (const auto& bone : bones) {
        const JointData* joint1 = nullptr;
        const JointData* joint2 = nullptr;

        // 查找对应的关节
        for (const auto& joint : frameData.joints) {
            if (joint.type == bone.first) {
                joint1 = &joint;
            }
            else if (joint.type == bone.second) {
                joint2 = &joint;
            }
        }

        // 如果找到了两个关节，就渲染骨骼
        if (joint1 && joint2) {
            renderBone(*joint1, *joint2);
        }
    }
}

void SkeletonRenderer::renderJoint(const JointData& jointData) {
    // 设置关节颜色
    m_pBrush->SetColor(getJointColor(jointData.trackingState));

    // 将相机空间坐标转换为屏幕坐标
    D2D1_POINT_2F screenPoint = transformToScreen(jointData.position);

    // 绘制关节点
    const float JOINT_RADIUS = 5.0f;
    D2D1_ELLIPSE ellipse = D2D1::Ellipse(screenPoint, JOINT_RADIUS, JOINT_RADIUS);
    m_pRenderTarget->FillEllipse(ellipse, m_pBrush);
}

void SkeletonRenderer::renderBone(const JointData& joint1, const JointData& joint2) {
    // 如果任一关节未被跟踪，则不渲染
    if (joint1.trackingState == TrackingState::NotTracked ||
        joint2.trackingState == TrackingState::NotTracked) {
        return;
    }

    // 设置骨骼颜色
    if (joint1.trackingState == TrackingState::Tracked &&
        joint2.trackingState == TrackingState::Tracked) {
        m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Green));
    }
    else {
        m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Yellow, 0.5f));
    }

    // 将相机空间坐标转换为屏幕坐标
    D2D1_POINT_2F point1 = transformToScreen(joint1.position);
    D2D1_POINT_2F point2 = transformToScreen(joint2.position);

    // 绘制骨骼线
    const float BONE_THICKNESS = 3.0f;
    m_pRenderTarget->DrawLine(point1, point2, m_pBrush, BONE_THICKNESS);
}

D2D1_COLOR_F SkeletonRenderer::getJointColor(TrackingState trackingState) const {
    switch (trackingState) {
        case TrackingState::Tracked:
            return D2D1::ColorF(D2D1::ColorF::Green);
        case TrackingState::Inferred:
            return D2D1::ColorF(D2D1::ColorF::Yellow);
        default:
            return D2D1::ColorF(D2D1::ColorF::Red);
    }
}

D2D1_POINT_2F SkeletonRenderer::transformToScreen(const CameraSpacePoint& position) const {
    // 获取渲染目标的大小
    D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

    // 将相机空间坐标映射到屏幕空间
    // 注意：这里的映射是简化的，实际应用中可能需要更复杂的变换
    const float SCALE_X = rtSize.width / 2.0f;
    const float SCALE_Y = rtSize.height / 2.0f;

    return D2D1::Point2F(
        (0.5f + position.X) * SCALE_X,
        (0.5f - position.Y) * SCALE_Y  // Y轴需要翻转
    );
}

} // namespace render
} // namespace kf 