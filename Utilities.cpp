#include "Utilities.h"
#include <boost/dll/runtime_symbol_info.hpp>
namespace rime
{
	const std::string & GetProfileDirectory()
	{
		static std::string s_profileDirectory;
		if (s_profileDirectory.empty()) {
			try {
				s_profileDirectory = (boost::dll::this_line_location().remove_filename() / "ChineseInput").string();
				s_profileDirectory += "\\";
			}
			catch (std::bad_alloc & ex) {

			}
		}
		return s_profileDirectory;
	}

	const std::string & GetConfigPath()
	{
		static std::string s_configPath;

		if (s_configPath.empty())
		{
			std::string	profileDirectory = GetProfileDirectory();
			if (!profileDirectory.empty())
			{
				s_configPath = profileDirectory + "ChineseInput.ini";

				_MESSAGE("config path = %s", s_configPath.c_str());
			}
		}

		return s_configPath;
	}

	std::string GetConfigOption(const char * section, const char * key)
	{
		std::string	result;

		const std::string & configPath = GetConfigPath();
		if (!configPath.empty())
		{
			char	resultBuf[256];
			resultBuf[0] = 0;

			UInt32	resultLen = GetPrivateProfileString(section, key, NULL, resultBuf, sizeof(resultBuf), configPath.c_str());

			result = resultBuf;
		}

		return result;
	}

	bool GetConfigOption_UInt32(const char * section, const char * key, UInt32 * dataOut)
	{
		std::string	data = GetConfigOption(section, key);
		if (data.empty())
			return false;

		return (sscanf_s(data.c_str(), "%u", dataOut) == 1);
	}
}
