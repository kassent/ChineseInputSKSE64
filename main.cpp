#include "skse64_common/skse_version.h"
#include "skse64_common/Utilities.h"
#include "skse64_common/Relocation.h"
#include "skse64_common/BranchTrampoline.h"
#include "skse64_common/SafeWrite.h"
#include "skse64/PluginManager.h"
#include <shlobj.h>
#include <Windows.h>
#include "D3D11.h"
#include "InputManager.h"
#include "RimeManager.h"
#define PLUGIN_VERSION	MAKE_EXE_VERSION_EX(1, 0, 2, 0)
#define PLUGIN_NAME		"ChineseInput"


IDebugLog					gLog;
PluginHandle				g_pluginHandle = kPluginHandle_Invalid;
//SKSETaskInterface			* g_taskInterface = nullptr;
SKSEMessagingInterface		* g_messaging = nullptr;




//#include "RE/TESFile.h"

namespace test {

	class BSThreadScrapHeapString 
	{
	public:
		BSThreadScrapHeapString(UInt32 size) {
			Allocate(size, 8);
		}
		~BSThreadScrapHeapString() {
			Release();
			buf = nullptr;
		}
		DEF_MEMBER_FN(Allocate, void, 0xC01F10, UInt32 size, UInt32 align);
		DEF_MEMBER_FN(Release, void, 0xC01FA0);

		char		* buf = nullptr;
	};

	class TESFile
	{
	public:
		enum class Error : UInt32
		{
			kNone = 0,
			kNotFound = 1,
			kNoFile = 2,
			kNoForm = 3,
			kNoChunk = 4,
			kNoID = 5,
			kBadFile = 6,
			kBadID = 7,
			kFormOpen = 8,
			kFileOpen = 9,
			kWriteFailure = 10,
			kInvalidFile = 11,
			kFileInUse = 12,
			kCreateFailure = 13
		};


		enum class RecordFlag : UInt32
		{
			kNone = 0,
			kMaster = 1 << 0,
			kAltered = 1 << 1,
			kChecked = 1 << 2,
			kActive = 1 << 3,
			kOptimizedFile = 1 << 4,
			kTempIDOwner = 1 << 5,
			kDelocalized = 1 << 7,
			kPrecalcDataOnly = 1 << 8,
			kSmallFile = 1 << 9
		};

		struct FORM
		{
		public:
			// members
			char   form[4];			// 00
			UInt32 length;			// 04
			UInt32 flags;			// 08
			UInt32 formID;			// 0C
			UInt32 versionControl;	// 10
			UInt16 formVersion;		// 14
			UInt16 vcVersion;		// 16
		};
		STATIC_ASSERT(sizeof(FORM) == 0x18);

		UInt64							unk00[0x58 >> 3];

		char						  fileName[MAX_PATH];				// 058
		char						  path[MAX_PATH];					// 15C
		//char*						  buffer;							// 260
		UInt64						  unk260[(0x280 - 0x260) >> 3];

		UInt32						  currRefOffset;					// 280
		FORM						  currentform;						// 284
		UInt32						  currentchunkID;					// 29C
		UInt32						  actualChunkSize;					// 2A0
		UInt32						  filesize;							// 2A4
		UInt32						  fileOffset;						// 2A8
		UInt32						  formoffset;						// 2AC
		UInt32						  chunkoffset;						// 2B0
		FORM						  saveform;							// 2B4
		UInt32						  saveFormOffset;					// 2CC
		UInt64						  saveChunkOffset;					// 2D0
		UInt64						  unk2D8;							// 2D8
		UInt64						  unk2E0;							// 2E0
		UInt8						  unk2E8;							// 2E8
		bool						  isBigEndian;						// 2E9
		UInt8						  unk2EA;							// 2EA
		UInt8						  pad2EB;							// 2EB
		UInt64						  unk2F0[(0x478 - 0x2F0) >> 3];
		UInt8						  modIndex;			// 478 init to 0xFF
		//TESFile* Duplicate(UInt32 a_cacheSize = 0x4000);
		//UInt32	 GetCurrentSubRecordType();
		//UInt32	 GetCurrentSubRecordSize() const;
		//UInt32	 GetFormType();
		//bool	 IsLoaded() const;
		//bool	 IsLocalized() const;
		//void	 ReadData(void* a_buf, UInt32 a_size);
		//bool	 Seek(UInt32 a_offset);
		//bool	 SeekNextSubrecord();

		DEF_MEMBER_FN(GetFormType, UInt32, 0x17D4C0);
		DEF_MEMBER_FN(IsLocalized, bool, 0x17E320);
		DEF_MEMBER_FN(GetCurrentSubRecordType, UInt32, 0x17D910);
		DEF_MEMBER_FN(InitializeFormFromFormRecord, void, 0x17DE00, void * effectSetting); //EffectSetting *
	};
	STATIC_ASSERT(offsetof(TESFile, currRefOffset) == 0x280);
	STATIC_ASSERT(offsetof(TESFile, modIndex) == 0x478);

	typedef bool(*_IsTESFileLocalized)(TESFile *);
	RelocAddr<_IsTESFileLocalized> IsTESFileLocalized(0x17E320);

	bool IsTESFileLocalized_Hook(TESFile* file)
	{
		UInt32 recordType = file->GetCurrentSubRecordType();
		char buf[5] = { 0 };
		char * characters = (char*)&recordType;
		std::memcpy(buf, &recordType, sizeof(recordType));
		_MESSAGE("FormType: %04X FormID: %08X, Localized: %d RecordType: %s PluginName: %s", file->GetFormType(), file->currentform.formID, file->IsLocalized(), buf, file->fileName);
		return IsTESFileLocalized(file);
	}

	void testOnly() {

		//g_branchTrampoline.Write5Call(RelocAddr<uintptr_t>(0x170147), (uintptr_t)IsTESFileLocalized_Hook);
		//g_branchTrampoline.Write5Call(RelocAddr<uintptr_t>(0x190659), (uintptr_t)IsTESFileLocalized_Hook);
		g_branchTrampoline.Write5Call(RelocAddr<uintptr_t>(0x1BB15F), (uintptr_t)IsTESFileLocalized_Hook);
		//g_branchTrampoline.Write5Call(RelocAddr<uintptr_t>(0x1BB29F), (uintptr_t)IsTESFileLocalized_Hook);
		//g_branchTrampoline.Write5Call(RelocAddr<uintptr_t>(0x1BB34B), (uintptr_t)IsTESFileLocalized_Hook);
		//g_branchTrampoline.Write5Call(RelocAddr<uintptr_t>(0x1BB56F), (uintptr_t)IsTESFileLocalized_Hook);
		//g_branchTrampoline.Write5Call(RelocAddr<uintptr_t>(0x38279A), (uintptr_t)IsTESFileLocalized_Hook);
	}
}






#pragma comment (lib, "imm32.lib")
#include <imm.h>
BOOL SetForegroundWindow_Hook(_In_ HWND hWnd)
{
	ImmAssociateContextEx(hWnd, NULL, NULL);
	return SetForegroundWindow(hWnd);
}

void MessageCallback(SKSEMessagingInterface::Message * msg)
{
	if (msg->type == SKSEMessagingInterface::kMessage_InputLoaded)
	{
		RimeManager::GetSingleton().StartRimeService();
	}
}



//#include "versiondb.h"

extern "C"
{
	bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)
	{
		std::unique_ptr<char[]> sPath(new char[MAX_PATH]);
		sprintf_s(sPath.get(), MAX_PATH, "%s%s.log", "\\My Games\\Skyrim Special Edition\\SKSE\\", PLUGIN_NAME);
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, sPath.get());

		_MESSAGE("%s: %08X", PLUGIN_NAME, PLUGIN_VERSION);

		if (skse->runtimeVersion != RUNTIME_VERSION_1_5_97)
			return false;

		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = PLUGIN_NAME;
		info->version = PLUGIN_VERSION;

		g_pluginHandle = skse->GetPluginHandle();

		//plugin_info.plugin_name = PLUGIN_NAME;
		//plugin_info.runtime_version = skse->runtimeVersion;

		if (skse->isEditor)
		{
			_FATALERROR("loaded in editor, marking as incompatible");
			return false;
		}
		g_messaging = (SKSEMessagingInterface *)skse->QueryInterface(kInterface_Messaging);
		if (!g_messaging)
		{
			_FATALERROR("couldn't get messaging interface");
			return false;
		}
		//g_taskInterface = (SKSETaskInterface *)skse->QueryInterface(kInterface_Task);
		//if (!g_taskInterface)
		//{
		//	_FATALERROR("couldn't get task interface");
		//	return false;
		//}
		return true;
	}

	bool SKSEPlugin_Load(const SKSEInterface * f4se)
	{
		if (!g_branchTrampoline.Create(1024 * 64))
		{
			_ERROR("couldn't create branch trampoline. this is fatal. skipping remainder of init process.");
			return false;
		}

		if (!g_localTrampoline.Create(1024 * 64, nullptr))
		{
			_ERROR("couldn't create codegen buffer. this is fatal. skipping remainder of init process.");
			return false;
		}

		if (g_messaging != nullptr)
			g_messaging->RegisterListener(g_pluginHandle, "SKSE", MessageCallback);


		//VersionDb db;

		// Try to load database of version 1.5.62.0 regardless of running executable version.
		//if (!db.Load(1, 5, 97, 0))
		//{
		//	_FATALERROR("Failed to load database for 1.5.62.0!");
		//	return false;
		//}

		// Write out a file called offsets-1.5.62.0.txt where each line is the ID and offset.
		//db.Dump("offsets-1.5.97.0.txt");
		//_MESSAGE("Dumped offsets for 1.5.97.0");
		//return true;

		g_branchTrampoline.Write6Call(RelocAddr<uintptr_t>(0xD72095), (uintptr_t)SetForegroundWindow_Hook);
		// fix crash when (console cmd string len > 512).
		UInt8 codes[] = { 0xBA, 0x99, 0x1, 0, 0 };
		SafeWriteBuf(RelocAddr<uintptr_t>(0x2EF7BE), &codes, sizeof(codes));

		SafeWrite16(RelocAddr<uintptr_t>(0x8B13A5), 0x9090); // fix race menu...
		SafeWrite16(RelocAddr<uintptr_t>(0x863199), 0x9090); // fix enchanting menu...
		SafeWrite16(RelocAddr<uintptr_t>(0x86DED1), 0x9090); // fix race menu...
		SafeWrite16(RelocAddr<uintptr_t>(0x8B0733), 0x9090); // fix race menu...

		D3D11_Init();
		rime::MenuControls::InitHooks();
		rime::BSWin32KeyboardDevice::InitHooks();





		//test only...
		test::testOnly();

		return true;
	}
};
