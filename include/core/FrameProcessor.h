#pragma once

#include <vector>
#include <Kinect.h>
#include "../KFcommon.h"

namespace kf {
namespace core {

class KinectManager;

// 帧观察者接口
class IFrameObserver {
public:
    virtual ~IFrameObserver() = default;
    virtual void onFrameProcessed(const FrameData& frameData) = 0;
};

class FrameProcessor {
public:
    explicit FrameProcessor(KinectManager* pKinectManager);
    ~FrameProcessor();

    // 初始化帧处理器
    bool initialize();

    // 开始处理帧数据
    void startProcessing();

    // 停止处理帧数据
    void stopProcessing();

    // 处理下一帧数据
    bool processNextFrame();

    // 添加观察者
    void addObserver(IFrameObserver* pObserver);

    // 移除观察者
    void removeObserver(IFrameObserver* pObserver);

private:
    // 处理骨骼帧
    void processBodyFrame(IBodyFrame* pBodyFrame);

    // 处理单个骨骼
    void processBody(IBody* pBody);

    // 通知观察者
    void notifyObservers(const FrameData& frameData);

private:
    KinectManager* m_pKinectManager;
    ICoordinateMapper* m_pCoordinateMapper;
    IBodyFrameReader* m_pBodyFrameReader;
    IColorFrameReader* m_pColorFrameReader;
    IDepthFrameReader* m_pDepthFrameReader;
    bool m_isProcessing;
    std::vector<IFrameObserver*> m_observers;
};

} // namespace core
} // namespace kf 