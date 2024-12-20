#include <filesystem>
#include <chrono>
#include <format>
#include "KFcommon.h"
#include "log/KFLog.h"

namespace kf {

// ȫ�ֱ�������
std::mutex g_templateMutex;
std::unique_ptr<ActionTemplate> g_actionTemplate;

bool ensureDirectoryExists(const std::string& path) {
	try {
		if (!std::filesystem::exists(DATA_DIR)) {
			if (!std::filesystem::create_directory(DATA_DIR)) {
				LOG_E("Failed to create directory: %s", DATA_DIR);
				return false;
			}
		}

		if (!std::filesystem::exists(path)) {
			LOG_I("Creating directory: %s", path.c_str());
			return std::filesystem::create_directory(path);
		}
	} catch (const std::exception& e) {
		LOG_E("Error ensuring directory exists: %s", e.what());
		return false;
	}
	return true;
}

std::string getStandardActionPath(const std::string& filename) {
	return std::format("{}/{}", TEMPLATE_DIR, filename);
}

std::string generateRecordingPath() {
	auto now = std::chrono::system_clock::now();
	auto timestamp = std::chrono::system_clock::to_time_t(now);
	std::string filename = std::format("{}/record_{:%Y%m%d_%H%M%S}.dat", 
		RECORD_DIR, *std::localtime(&timestamp));
	return filename;
}

// ActionTemplate ���캯��ʵ��
ActionTemplate::ActionTemplate(const std::string& filename) {
	loadFromFile(filename);
}

} // namespace kf