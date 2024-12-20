#pragma once

#include <Kinect.h>
#include "../KFcommon.h"

namespace kf {
namespace core {

class KinectManager {
public:
    KinectManager();
    ~KinectManager();

    // 初始化 Kinect
    bool initialize();

    // 获取 Kinect 接口
    IKinectSensor* getSensor() const { return m_pKinectSensor; }
    ICoordinateMapper* getCoordinateMapper() const { return m_pCoordinateMapper; }
    IBodyFrameReader* getBodyFrameReader() const { return m_pBodyFrameReader; }
    IColorFrameReader* getColorFrameReader() const { return m_pColorFrameReader; }
    IDepthFrameReader* getDepthFrameReader() const { return m_pDepthFrameReader; }

private:
    // 初始化传感器
    bool initializeSensor();

    // 初始化数据流
    bool initializeFrameReaders();

private:
    IKinectSensor* m_pKinectSensor;
    ICoordinateMapper* m_pCoordinateMapper;
    IBodyFrameReader* m_pBodyFrameReader;
    IColorFrameReader* m_pColorFrameReader;
    IDepthFrameReader* m_pDepthFrameReader;
};

} // namespace core
} // namespace kf 