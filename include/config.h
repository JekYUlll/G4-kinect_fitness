#pragma once

#include <cstdint>
#include <string>

namespace kf {

// Window configuration
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

// Display configuration
constexpr int DEPTH_WIDTH = 512;
constexpr int DEPTH_HEIGHT = 424;
constexpr int COLOR_WIDTH = 1920;
constexpr int COLOR_HEIGHT = 1080;

// File paths
constexpr const char* DATA_DIR = "data/";
constexpr const char* TEMPLATE_DIR = "data/templates/";
constexpr const char* RECORD_DIR = "data/records/";

// Recording configuration
constexpr int ACTION_BUFFER_SIZE = 200;
constexpr int RECORD_INTERVAL = 33; // 30fps

// Rendering configuration
constexpr float JOINT_THICKNESS = 3.0f;
constexpr float TRACKED_BONE_THICKNESS = 6.0f;
constexpr float INFERRED_BONE_THICKNESS = 1.0f;

// Similarity calculation configuration
constexpr double SIMILARITY_THRESHOLD = 0.8;
constexpr double GAUSSIAN_SIGMA = 0.5;
constexpr int MIN_FRAME_COUNT = 30;
constexpr int MAX_FRAME_COUNT = 300;

// Debug configuration
constexpr bool ENABLE_DEBUG_LOG = true;

} // namespace kf