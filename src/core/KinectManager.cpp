#include "../../include/core/KinectManager.h"
#include "../../include/log/KFLog.h"

namespace kf {
namespace core {

KinectManager::KinectManager()
    : m_pKinectSensor(nullptr)
    , m_pCoordinateMapper(nullptr)
    , m_pBodyFrameReader(nullptr)
    , m_pColorFrameReader(nullptr)
    , m_pDepthFrameReader(nullptr) {
}

KinectManager::~KinectManager() {
    shutdown();
}

bool KinectManager::initialize() {
    if (!initializeKinectSensor()) {
        LOG_E("无法初始化 Kinect 传感器");
        return false;
    }

    if (!initializeCoordinateMapper()) {
        LOG_E("无法初始化坐标映射器");
        return false;
    }

    if (!initializeBodyFrameReader()) {
        LOG_E("无法初始化骨骼帧读取器");
        return false;
    }

    if (!initializeColorFrameReader()) {
        LOG_E("无法初始化彩色帧读取器");
        return false;
    }

    if (!initializeDepthFrameReader()) {
        LOG_E("无法初始化深度帧读取器");
        return false;
    }

    LOG_I("Kinect 管理器初始化成功");
    return true;
}

void KinectManager::shutdown() {
    releaseAll();
}

bool KinectManager::initializeKinectSensor() {
    HRESULT hr = GetDefaultKinectSensor(&m_pKinectSensor);
    if (FAILED(hr) || !m_pKinectSensor) {
        LOG_E("获取默认 Kinect 传感器失败");
        return false;
    }

    hr = m_pKinectSensor->Open();
    if (FAILED(hr)) {
        LOG_E("打开 Kinect 传感器失败");
        return false;
    }

    return true;
}

bool KinectManager::initializeCoordinateMapper() {
    if (!m_pKinectSensor) {
        LOG_E("Kinect 传感器未初始化");
        return false;
    }

    HRESULT hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
    if (FAILED(hr) || !m_pCoordinateMapper) {
        LOG_E("获取坐标映射器失败");
        return false;
    }

    return true;
}

bool KinectManager::initializeBodyFrameReader() {
    if (!m_pKinectSensor) {
        LOG_E("Kinect 传感器未初始化");
        return false;
    }

    IBodyFrameSource* pBodyFrameSource = nullptr;
    HRESULT hr = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
    if (FAILED(hr) || !pBodyFrameSource) {
        LOG_E("获取骨骼帧源失败");
        return false;
    }

    hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
    if (FAILED(hr) || !m_pBodyFrameReader) {
        LOG_E("打开骨骼帧读取器失败");
        pBodyFrameSource->Release();
        return false;
    }

    pBodyFrameSource->Release();
    return true;
}

bool KinectManager::initializeColorFrameReader() {
    if (!m_pKinectSensor) {
        LOG_E("Kinect 传感器未初始化");
        return false;
    }

    IColorFrameSource* pColorFrameSource = nullptr;
    HRESULT hr = m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource);
    if (FAILED(hr) || !pColorFrameSource) {
        LOG_E("获取彩色帧源失败");
        return false;
    }

    hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);
    if (FAILED(hr) || !m_pColorFrameReader) {
        LOG_E("打开彩色帧读取器失败");
        pColorFrameSource->Release();
        return false;
    }

    pColorFrameSource->Release();
    return true;
}

bool KinectManager::initializeDepthFrameReader() {
    if (!m_pKinectSensor) {
        LOG_E("Kinect 传感器未初始化");
        return false;
    }

    IDepthFrameSource* pDepthFrameSource = nullptr;
    HRESULT hr = m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);
    if (FAILED(hr) || !pDepthFrameSource) {
        LOG_E("获取深度帧源失败");
        return false;
    }

    hr = pDepthFrameSource->OpenReader(&m_pDepthFrameReader);
    if (FAILED(hr) || !m_pDepthFrameReader) {
        LOG_E("打开深度帧读取器失败");
        pDepthFrameSource->Release();
        return false;
    }

    pDepthFrameSource->Release();
    return true;
}

void KinectManager::releaseAll() {
    if (m_pDepthFrameReader) { m_pDepthFrameReader->Release(); m_pDepthFrameReader = nullptr; }
    if (m_pColorFrameReader) { m_pColorFrameReader->Release(); m_pColorFrameReader = nullptr; }
    if (m_pBodyFrameReader) { m_pBodyFrameReader->Release(); m_pBodyFrameReader = nullptr; }
    if (m_pCoordinateMapper) { m_pCoordinateMapper->Release(); m_pCoordinateMapper = nullptr; }
    if (m_pKinectSensor) {
        m_pKinectSensor->Close();
        m_pKinectSensor->Release();
        m_pKinectSensor = nullptr;
    }
}

IKinectSensor* KinectManager::getKinectSensor() const {
    return m_pKinectSensor;
}

ICoordinateMapper* KinectManager::getCoordinateMapper() const {
    return m_pCoordinateMapper;
}

IBodyFrameReader* KinectManager::getBodyFrameReader() const {
    return m_pBodyFrameReader;
}

IColorFrameReader* KinectManager::getColorFrameReader() const {
    return m_pColorFrameReader;
}

IDepthFrameReader* KinectManager::getDepthFrameReader() const {
    return m_pDepthFrameReader;
}

} // namespace core
} // namespace kf