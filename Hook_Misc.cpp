#include "Hook_Misc.h"
#include <imm.h>
#include <SKSE/API.h>
#include <SKSE/SafeWrite.h>

#include "Hook_DX11.h"
#include "Hook_GameInput.h"

#pragma comment (lib, "imm32.lib")
namespace Misc 
{
	BOOL SetForegroundWindow_Hook(_In_ HWND hWnd)
	{
		ImmAssociateContextEx(hWnd, NULL, NULL);
		return SetForegroundWindow(hWnd);
	}

	void InstallHooks()
	{
		SKSE::GetTrampoline()->Write6Call(REL::ID(75591).GetAddress() + 0xD72095 - 0xD71F00, (uintptr_t)SetForegroundWindow_Hook);
		//g_branchTrampoline.Write6Call(RelocAddr<uintptr_t>(0xD72095), (uintptr_t)SetForegroundWindow_Hook);
		// fix crash when (console cmd string len > 512).
		UInt8 codes[] = { 0xBA, 0x99, 0x1, 0, 0 };
		//21495	2ef6f0
		//SKSE::SafeWriteBuf(RelocAddr<uintptr_t>(0x2EF7BE), &codes, sizeof(codes));
		SKSE::SafeWriteBuf(REL::ID(21495).GetAddress() + 0x2EF7BE - 0x2EF6F0, &codes, sizeof(codes));

		//SKSE::SafeWrite16(RelocAddr<uintptr_t>(0x8B13A5), 0x9090); // fix race menu... 51488	8b1360
		//SKSE::SafeWrite16(RelocAddr<uintptr_t>(0x863199), 0x9090); // fix enchanting menu...
		//SKSE::SafeWrite16(RelocAddr<uintptr_t>(0x86DED1), 0x9090); // fix race menu...
		//SKSE::SafeWrite16(RelocAddr<uintptr_t>(0x8B0733), 0x9090); // fix race menu...

		SKSE::SafeWrite16(REL::ID(51488).GetAddress() + 0x8B13A5 - 0x8b1360, 0x9090); // fix race menu...51488	8b1360
		SKSE::SafeWrite16(REL::ID(50274).GetAddress() + 0x863199 - 0x862c80, 0x9090); // fix enchanting menu...50274	862c80
		SKSE::SafeWrite16(REL::ID(50470).GetAddress() + 0x86DED1 - 0x86de10, 0x9090); // fix race menu...50470	86de10
		SKSE::SafeWrite16(REL::ID(51483).GetAddress() + 0x8B0733 - 0x8b0200, 0x9090); // fix race menu... 51483	8b0200

		//D3D11::InstallHooks();
		//MenuControlsEx::InitHooks();
		//BSWin32KeyboardDeviceEx::InitHooks();
	}
}

