#pragma once
#include <string>

namespace rime {
	const std::string& GetLogDirectory();

	std::string GetUserDataDirectory();

	const std::string & GetProfileDirectory();

	const std::string & GetConfigPath();

	std::string GetConfigOption(const char * section, const char * key);

	bool GetConfigOption_UInt32(const char * section, const char * key, UInt32 * dataOut);

	void SetConfigOption(const char* section, const char* key, const char * dataIn);

	void SetConfigOption_UInt32(const char* section, const char* key, UInt32 dataIn);
}
