#pragma once

#include <d2d1.h>
#include <memory>
#include <vector>
#include "calc/serialize.h"

namespace kf::render {

class SkeletonRenderer {
public:
    static SkeletonRenderer& getInstance() {
        static SkeletonRenderer instance;
        return instance;
    }

    bool initialize(ID2D1HwndRenderTarget* renderTarget);
    void shutdown();

    void drawBody(const Joint* joints, const D2D1_POINT_2F* jointPoints);
    void drawTemplateBody(const JointData* joints, const D2D1_POINT_2F* jointPoints);
    void drawHand(HandState handState, const D2D1_POINT_2F& position);

    D2D1_POINT_2F bodyToScreen(const CameraSpacePoint& bodyPoint, int width, int height);

private:
    SkeletonRenderer() = default;
    ~SkeletonRenderer();

    SkeletonRenderer(const SkeletonRenderer&) = delete;
    SkeletonRenderer& operator=(const SkeletonRenderer&) = delete;

    void drawBone(const Joint* joints, const D2D1_POINT_2F* jointPoints, 
                 JointType joint0, JointType joint1);
    void drawTemplateBone(const JointData* joints, const D2D1_POINT_2F* jointPoints,
                         JointType joint0, JointType joint1);

    // D2D Resources
    ID2D1HwndRenderTarget* m_pRenderTarget = nullptr;
    ID2D1SolidColorBrush* m_pBrushJointTracked = nullptr;
    ID2D1SolidColorBrush* m_pBrushJointInferred = nullptr;
    ID2D1SolidColorBrush* m_pBrushBoneTracked = nullptr;
    ID2D1SolidColorBrush* m_pBrushBoneInferred = nullptr;
    ID2D1SolidColorBrush* m_pBrushHandClosed = nullptr;
    ID2D1SolidColorBrush* m_pBrushHandOpen = nullptr;
    ID2D1SolidColorBrush* m_pBrushHandLasso = nullptr;
    ID2D1SolidColorBrush* m_pBrushJointTemplate = nullptr;
    ID2D1SolidColorBrush* m_pBrushBoneTemplate = nullptr;

    // Bone structure
    static const std::vector<std::pair<JointType, JointType>> BONE_PAIRS;
};

} // namespace kf::render 