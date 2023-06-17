#pragma once
#include <string>

namespace rime {
	const std::string & GetProfileDirectory();
	const std::string & GetConfigPath();
	std::string GetConfigOption(const char * section, const char * key);
	bool GetConfigOption_UInt32(const char * section, const char * key, UInt32 * dataOut);
}
