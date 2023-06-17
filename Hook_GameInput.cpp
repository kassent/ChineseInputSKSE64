#include "Hook_GameInput.h"

#include <RE/ButtonEvent.h>
#include <SKSE/API.h>

#include "DXScanCode.h"
#include "RimeManager.h"

using namespace RE;

void BSWin32KeyboardDeviceEx::ProcessKeyboardInput(UInt32 a_keyCode, float a_delta, bool a_curPressState, bool a_prevPressState)
{
	using func_t = decltype(&BSWin32KeyboardDeviceEx::ProcessKeyboardInput);
	static REL::Offset<func_t> func = REL::ID(67441);//0x0C190F0
	return func(this, a_keyCode, a_delta, a_curPressState, a_prevPressState);
}

void BSWin32KeyboardDeviceEx::ProcessKeyboardInput_Hook(UInt32 a_keyCode, float a_delta, bool a_curPressState, bool a_prevPressState)
{
	auto& rime = RimeManager::GetSingleton();
	if (rime.IsRimeEnabled()) {
		if (rime.IsRimeComposing()) {
			UInt32 cmdKeyCode = RimeManager::TranslateKeycode(a_keyCode);
			if ((a_keyCode ^ cmdKeyCode)) {
				if (a_curPressState && !a_prevPressState)
					rime.PostCommand<RimeCharMessage>(cmdKeyCode); //一些特殊的控制字符。
				return;
			}
		}
		else if (a_keyCode == KeyCode::F3) { //唯一特殊按键F4，用来切换及配置输入法shema。ps: F4可能用于Alt+F4操作，暂改为F3.
			if (a_curPressState && !a_prevPressState)
				rime.PostCommand<RimeCharMessage>(ibus::F4); //一些特殊的控制字符。
			return;
		}
	}
	ProcessKeyboardInput(a_keyCode, a_delta, a_curPressState, a_prevPressState);
}


void BSWin32KeyboardDeviceEx::InitHooks()
{
	//67472	c1a130
	//g_branchTrampoline.Write5Call(RelocAddr<uintptr_t>(0xC1A28D), (uintptr_t)ProcessKeyboardInput_Hook);
	SKSE::GetTrampoline()->Write5Call(REL::ID(67472).GetAddress() + 0xC1A28D - 0xC1A130, unrestricted_cast<uintptr_t>(&BSWin32KeyboardDeviceEx::ProcessKeyboardInput_Hook));
}

void MenuControlsEx::ProcessCommonInputEvent(InputEvent* a_event)
{
	using func_t = decltype(&MenuControlsEx::ProcessCommonInputEvent);
	static REL::Offset<func_t> func = REL::ID(51370);//0x8A85C0
	return func(this, a_event);
}

void MenuControlsEx::ProcessCommonInputEvent_Hook(InputEvent* a_event)
{
	if (a_event) {
		auto& rime = RimeManager::GetSingleton();
		switch (a_event->eventType)
		{
		case INPUT_EVENT_TYPE::kButton:
		{
			if (rime.IsRimeEnabled()) {
				if (rime.IsRimeComposing()) //在输入法组字的时候屏蔽UI的键位输入。
					return;
				auto* buttonEvent = (ButtonEvent*)a_event;
				if (buttonEvent->device == INPUT_DEVICE::kKeyboard && buttonEvent->IsDown()) {
					UInt32 idCode = buttonEvent->idCode;
					switch (idCode)
					{
					case KeyCode::LeftShift:
						rime.PostCommand<RimeCommandMessage>(RimeCommandMessage::kCommandType_SwitchAsciiMode);
						break;
					case KeyCode::RightAlt:
						rime.PostCommand<RimeCommandMessage>(RimeCommandMessage::kCommandType_SwitchFullShape);
						break;
					case KeyCode::RightControl:
						rime.PostCommand<RimeCommandMessage>(RimeCommandMessage::kCommandType_SwitchSimplification);
						break;
					default:
						break;
					}
				}
			}
		}
		break;
		case INPUT_EVENT_TYPE::kMouseMove:
			break;
		case INPUT_EVENT_TYPE::kChar:
		{
			CharEvent* charEvent = (CharEvent*)a_event;
			if (rime.IsRimeEnabled() && (charEvent->unicode != 0x60 /*|| rime.IsRimeComposing()*/)) {//~key is for console open/close...IsConsoleMode doesn't work as intended...
				rime.PostCommand<RimeCharMessage>(charEvent->unicode);
				return;
			}
		}
		break;
		default:
			break;
		}
	}
	ProcessCommonInputEvent(a_event);
}

MenuControlsEx* MenuControlsEx::GetSingleton()
{
	return static_cast<MenuControlsEx*>(MenuControls::GetSingleton());
}

void MenuControlsEx::InitHooks()
{
	/*
	51356	8a7ca0
	51357	8a7f00
	*/
	//g_branchTrampoline.Write5Call(RelocAddr<uintptr_t>(0x8A7DEB), (uintptr_t)ProcessCommonInputEvent_Hook);
	//g_branchTrampoline.Write5Call(RelocAddr<uintptr_t>(0x8A7E22), (uintptr_t)ProcessCommonInputEvent_Hook);
	//ReceiveEvent_RealFunc = HookUtil::SafeWrite64(RelocAddr<uintptr_t>(0x16B8308) + 0x8 * 1, &MenuControls::ReceiveEvent_Hook);

	SKSE::GetTrampoline()->Write5Call(REL::ID(51356).GetAddress() + 0x8A7DEB - 0x8A7CA0, unrestricted_cast<uintptr_t>(&MenuControlsEx::ProcessCommonInputEvent_Hook));
	SKSE::GetTrampoline()->Write5Call(REL::ID(51356).GetAddress() + 0x8A7E22 - 0x8A7CA0, unrestricted_cast<uintptr_t>(&MenuControlsEx::ProcessCommonInputEvent_Hook));
}

void UI::SendBSUIScaleformData(const BSFixedString& a_name, GFxEvent* a_event)
{
	using func_t = decltype(SendBSUIScaleformData);
	static REL::Offset<func_t*> func = REL::ID(80079);//0xEC3ED0 80079	ec3ed0
	return func(a_name, a_event);
}




