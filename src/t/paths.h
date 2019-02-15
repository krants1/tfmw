#pragma once

#include <string>

#ifdef __linux__
#include <unistd.h>
#include <linux/limits.h>
#else
#include <windows.h>
#endif

namespace T {

	struct PathHelper {
		static std::string getExePath() {
#ifdef __linux__
			char result[PATH_MAX];
			ssize_t len = readlink("/proc/self/exe", result, PATH_MAX);
			if (len != -1) {
				result[len] = '\0';
				return std::string(result);
			} else
				return "";
#else
			char result[MAX_PATH];
			DWORD size = GetModuleFileNameA(NULL, result, MAX_PATH);
			return (size) ? std::string(result) : "";
#endif
		}

		static void replaceExt(std::string & fileName, const std::string & newExt) {
			std::string::size_type i = fileName.rfind('.', fileName.length());
			if (i != std::string::npos)
				fileName.replace(i + 1, newExt.length(), newExt);
			else
				fileName += "." + newExt;
		}
		static std::string getFilePath(std::string & fileName) {
			std::string::size_type p = fileName.rfind('/', fileName.length());
			if (p != std::string::npos)
				return fileName.substr(0, p + 1);
			else
				return fileName;
		}
	};
}