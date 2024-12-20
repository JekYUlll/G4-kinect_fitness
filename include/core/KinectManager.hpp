#pragma once

#include <memory>
#include <Kinect.h>
#include "log/KFLog.h"

namespace kf::core {

class KinectManager {
public:
    static KinectManager& getInstance() {
        static KinectManager instance;
        return instance;
    }

    bool initialize();
    void shutdown();

    IKinectSensor* getSensor() const { return m_pKinectSensor; }
    ICoordinateMapper* getCoordinateMapper() const { return m_pCoordinateMapper; }
    IBodyFrameReader* getBodyFrameReader() const { return m_pBodyFrameReader; }
    IColorFrameReader* getColorFrameReader() const { return m_pColorFrameReader; }

private:
    KinectManager() = default;
    ~KinectManager();

    KinectManager(const KinectManager&) = delete;
    KinectManager& operator=(const KinectManager&) = delete;

    IKinectSensor* m_pKinectSensor = nullptr;
    ICoordinateMapper* m_pCoordinateMapper = nullptr;
    IBodyFrameReader* m_pBodyFrameReader = nullptr;
    IColorFrameReader* m_pColorFrameReader = nullptr;
};

} // namespace kf::core 