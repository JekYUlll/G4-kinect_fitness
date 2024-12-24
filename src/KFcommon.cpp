#include "KFcommon.h"
#include <filesystem>

namespace kfc {
	std::string std_file_path = ""; // ��׼�����ļ�·��
	int window_width = 800;
	int window_height = 600;
	INT64 recordInterval = 33; // ~30fps

	// ȷ��¼��Ŀ¼����
	bool ensureDirectoryExists() {
		try {
			// ֻ����Ҫ¼��ʱ��������Ŀ¼
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

	// ��ȡ��׼�����ļ�������·����ֱ�Ӵ�exeͬ��Ŀ¼��ȡ��
	std::string getStandardActionPath(const std::string& filename) {
		return filename; // ֱ�ӷ����ļ�������exeͬ��Ŀ¼��ȡ
	}

	// ��ȡ��¼���ļ���·�����洢��dataĿ¼��
	std::string generateRecordingPath() {
		// ȷ��¼��Ŀ¼����
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