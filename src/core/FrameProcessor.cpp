#include "../../include/core/FrameProcessor.h"
#include "../../include/log/KFLog.h"

namespace kf {
namespace core {

FrameProcessor::FrameProcessor(KinectManager* pKinectManager)
    : m_pKinectManager(pKinectManager)
    , m_pCoordinateMapper(nullptr)
    , m_pBodyFrameReader(nullptr)
    , m_pColorFrameReader(nullptr)
    , m_pDepthFrameReader(nullptr)
    , m_isProcessing(false) {
}

FrameProcessor::~FrameProcessor() {
    stopProcessing();
}

bool FrameProcessor::initialize() {
    if (!m_pKinectManager) {
        LOG_E("Kinect 管理器未设置");
        return false;
    }

    m_pCoordinateMapper = m_pKinectManager->getCoordinateMapper();
    m_pBodyFrameReader = m_pKinectManager->getBodyFrameReader();
    m_pColorFrameReader = m_pKinectManager->getColorFrameReader();
    m_pDepthFrameReader = m_pKinectManager->getDepthFrameReader();

    if (!m_pCoordinateMapper || !m_pBodyFrameReader || 
        !m_pColorFrameReader || !m_pDepthFrameReader) {
        LOG_E("无法获取必要的 Kinect 接口");
        return false;
    }

    return true;
}

void FrameProcessor::startProcessing() {
    if (!m_isProcessing) {
        m_isProcessing = true;
        LOG_I("开始处理帧数据");
    }
}

void FrameProcessor::stopProcessing() {
    if (m_isProcessing) {
        m_isProcessing = false;
        LOG_I("停止处理帧数据");
    }
}

bool FrameProcessor::processNextFrame() {
    if (!m_isProcessing) {
        return false;
    }

    IBodyFrame* pBodyFrame = nullptr;
    HRESULT hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);
    if (SUCCEEDED(hr) && pBodyFrame) {
        processBodyFrame(pBodyFrame);
        pBodyFrame->Release();
    }

    return true;
}

void FrameProcessor::processBodyFrame(IBodyFrame* pBodyFrame) {
    if (!pBodyFrame) {
        return;
    }

    IBody* ppBodies[BODY_COUNT] = {0};
    HRESULT hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
    if (SUCCEEDED(hr)) {
        for (int i = 0; i < BODY_COUNT; ++i) {
            IBody* pBody = ppBodies[i];
            if (pBody) {
                BOOLEAN isTracked = false;
                hr = pBody->get_IsTracked(&isTracked);
                if (SUCCEEDED(hr) && isTracked) {
                    processBody(pBody);
                }
                pBody->Release();
            }
        }
    }
}

void FrameProcessor::processBody(IBody* pBody) {
    if (!pBody) {
        return;
    }

    Joint joints[JointType_Count];
    JointOrientation jointOrientations[JointType_Count];
    HandState leftHandState = HandState_Unknown;
    HandState rightHandState = HandState_Unknown;

    HRESULT hr = pBody->GetJoints(_countof(joints), joints);
    if (SUCCEEDED(hr)) {
        hr = pBody->GetJointOrientations(_countof(jointOrientations), jointOrientations);
        if (SUCCEEDED(hr)) {
            pBody->get_HandLeftState(&leftHandState);
            pBody->get_HandRightState(&rightHandState);

            // 创建帧数据
            FrameData frameData;
            frameData.timestamp = GetTickCount64();
            frameData.joints.reserve(JointType_Count);

            for (int i = 0; i < JointType_Count; ++i) {
                JointData jointData;
                jointData.type = static_cast<JointType>(i);
                jointData.position = joints[i].Position;
                jointData.trackingState = joints[i].TrackingState;
                frameData.joints.push_back(jointData);
            }

            // 通知观察者
            notifyObservers(frameData);
        }
    }
}

void FrameProcessor::addObserver(IFrameObserver* pObserver) {
    if (pObserver) {
        m_observers.push_back(pObserver);
    }
}

void FrameProcessor::removeObserver(IFrameObserver* pObserver) {
    if (pObserver) {
        m_observers.erase(
            std::remove(m_observers.begin(), m_observers.end(), pObserver),
            m_observers.end()
        );
    }
}

void FrameProcessor::notifyObservers(const FrameData& frameData) {
    for (auto* pObserver : m_observers) {
        if (pObserver) {
            pObserver->onFrameProcessed(frameData);
        }
    }
}

} // namespace core
} // namespace kf 