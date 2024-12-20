#pragma once

#include <d2d1.h>
#include "../KFcommon.h"

namespace kf {
namespace render {

class SkeletonRenderer {
public:
    SkeletonRenderer();
    ~SkeletonRenderer();

    // 初始化渲染器
    bool initialize(ID2D1HwndRenderTarget* pRenderTarget);

    // 渲染骨骼
    void renderSkeleton(const FrameData& frameData);

private:
    // 渲染关节
    void renderJoint(const JointData& jointData);

    // 渲染骨骼线
    void renderBone(const JointData& joint1, const JointData& joint2);

    // 获取关节颜色
    D2D1_COLOR_F getJointColor(TrackingState trackingState) const;

    // 将相机空间坐标转换为屏幕坐标
    D2D1_POINT_2F transformToScreen(const CameraSpacePoint& position) const;

private:
    ID2D1HwndRenderTarget* m_pRenderTarget;
    ID2D1SolidColorBrush* m_pBrush;
};

} // namespace render
} // namespace kf 