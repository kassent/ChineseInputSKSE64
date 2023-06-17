#include <Windows.h>
#include <filesystem>
#include <SKSE/API.h>
#include <SKSE/Logger.h>

#include "Hook_DX11.h"
#include "Hook_GameInput.h"
#include "Hook_Misc.h"
#include "Utilities.h"
#include "RimeManager.h"

#define MAKE_EXE_VERSION_EX(major, minor, build, sub)	((((major) & 0xFF) << 24) | (((minor) & 0xFF) << 16) | (((build) & 0xFFF) << 4) | ((sub) & 0xF))
#define MAKE_EXE_VERSION(major, minor, build)			MAKE_EXE_VERSION_EX(major, minor, build, 0)
#define PLUGIN_VERSION	MAKE_EXE_VERSION_EX(1, 0, 2, 0)
#define PLUGIN_NAME		"ChineseInput"


void MessageCallback(SKSE::MessagingInterface::Message * msg)
{
	if (msg->type == SKSE::MessagingInterface::kInputLoaded)
	{
		RimeManager::GetSingleton().StartRimeService();
	}
}


extern "C"
{
	bool SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
	{
		//SKSE::Logger::OpenRelative(FOLDERID_Documents, std::filesystem::path("\\My Games\\Skyrim Special Edition\\SKSE\\ChineseInput"));
		//while (!IsDebuggerPresent()) {
		//	Sleep(100);
		//}
		SKSE::Logger::OpenAbsolute(rime::GetLogDirectory() + PLUGIN_NAME + ".log");
		SKSE::Logger::SetPrintLevel(SKSE::Logger::Level::kDebugMessage);
		SKSE::Logger::SetFlushLevel(SKSE::Logger::Level::kDebugMessage);
		SKSE::Logger::UseLogStamp(true);

		a_info->infoVersion = SKSE::PluginInfo::kVersion;
		a_info->name = PLUGIN_NAME;
		a_info->version = PLUGIN_VERSION;

		if (a_skse->IsEditor()) {
			_FATALERROR("Loaded in editor, marking as incompatible!\n");
			return false;
		}
		return true;
	}



	bool SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
	{
		_MESSAGE("ChineseInput loaded...");
		UInt32 dllVersion = PLUGIN_VERSION;
		if (!rime::GetConfigOption_UInt32("Settings", "iVersion", &dllVersion))
			dllVersion = MAKE_EXE_VERSION(1, 0, 0);

		if (dllVersion != PLUGIN_VERSION) {
			//outdated version.need update database.
			rime::SetConfigOption_UInt32("Settings", "iVersion", PLUGIN_VERSION);
			std::error_code ec;
			std::filesystem::remove_all(rime::GetUserDataDirectory(), ec);
			_MESSAGE("Rebuilt rime user data...");
		}


		if (!SKSE::Init(a_skse)) {
			return false;
		}

		if (!SKSE::AllocTrampoline(1 << 10)) {
			return false;
		}

		SKSE::GetMessagingInterface()->RegisterListener(MessageCallback);

		Misc::InstallHooks();
		D3D11::InstallHooks();
		MenuControlsEx::InitHooks();
		BSWin32KeyboardDeviceEx::InitHooks();

		return true;
	}
};













