#include "KFcommon.h"
#include <filesystem>

namespace kfc {
	std::string std_file_path = ""; // 标准动作文件路径
	int window_width = 800;
	int window_height = 600;
	INT64 recordInterval = 33; // ~30fps

	// 确保录制目录存在
	bool ensureDirectoryExists() {
		try {
			// 只在需要录制时创建数据目录
			if (!std::filesystem::exists(KF_DATA_DIR)) {
				if (!std::filesystem::create_directory(KF_DATA_DIR)) {
					LOG_E("Failed to create recording directory: {}", KF_DATA_DIR);
					return false;
				}
				LOG_I("Created recording directory: {}", KF_DATA_DIR);
			}
			return true;
		}
		catch (const std::filesystem::filesystem_error& e) {
			LOG_E("Filesystem error: {}", e.what());
			return false;
		}
	}

	// 获取标准动作文件的完整路径（直接从exe同级目录读取）
	std::string getStandardActionPath(const std::string& filename) {
		return filename; // 直接返回文件名，从exe同级目录读取
	}

	// 获取新录制文件的路径（存储到data目录）
	std::string generateRecordingPath() {
		// 确保录制目录存在
		if (!ensureDirectoryExists()) {
			LOG_E("Failed to ensure recording directory exists");
			return "";
		}

		SYSTEMTIME st;
		GetLocalTime(&st);
		char fileName[256];
		sprintf_s(fileName, "skeleton_record_%04d%02d%02d_%02d%02d%02d.dat",
			st.wYear, st.wMonth, st.wDay,
			st.wHour, st.wMinute, st.wSecond);
		return std::string(KF_DATA_DIR) + "/" + fileName;
	}
}