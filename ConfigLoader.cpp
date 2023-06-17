#include "ConfigLoader.h"
#include "Utilities.h"

using namespace rime;
ConfigLoader::ConfigLoader()
{
	UInt32 option;
	if (GetConfigOption_UInt32("Settings", "CharacterSet", &option))
		characterSet = option;
	if (GetConfigOption_UInt32("Settings", "PosX", &option))
		uiOffsetX = option;
	if (GetConfigOption_UInt32("Settings", "PosY", &option))
		uiOffsetY = option;
}


ConfigLoader& ConfigLoader::GetSingleton()
{
	static ConfigLoader s_inst;
	return s_inst;
}
