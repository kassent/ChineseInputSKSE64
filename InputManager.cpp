#include "skse64_common/BranchTrampoline.h"
#include "skse64/PluginAPI.h"
#include "skse64/InputMap.h"
#include "InputManager.h"
#include "RimeManager.h"
#include "KeyCode.h"
#include "HookUtil.h"


namespace rime {

	RelocAddr<_SendBSUIScaleformData> SendBSUIScaleformData(0xEC3ED0);

	void BSWin32KeyboardDevice::ProcessKeyboardInput_Hook(BSWin32KeyboardDevice * pDevice, UInt32 keyCode, float delta, bool curPressState, bool prevPressState)
	{
		auto & rime = RimeManager::GetSingleton();
		if (rime.IsRimeEnabled()) {
			if (rime.IsRimeComposing()) {
				UInt32 cmdKeyCode = RimeManager::TranslateKeycode(keyCode);
				if ((keyCode ^ cmdKeyCode)) {
					if (curPressState && !prevPressState)
						rime.PostCommand(RimeCharMessage(cmdKeyCode)); //一些特殊的控制字符。
					return;
				}
			}
			else if (keyCode == KeyCode::F3) { //唯一特殊按键F4，用来切换及配置输入法shema。F4当前另有用途，暂改为F3.
				if (curPressState && !prevPressState)
					rime.PostCommand(RimeCharMessage(ibus::F4)); //一些特殊的控制字符。
				return;
			}
		}
		pDevice->ProcessKeyboardInput(keyCode, delta, curPressState, prevPressState);
	}
	void BSWin32KeyboardDevice::InitHooks()
	{
		g_branchTrampoline.Write5Call(RelocAddr<uintptr_t>(0xC1A28D), (uintptr_t)ProcessKeyboardInput_Hook);
	}




	InputManager * InputManager::GetSingleton(void)
	{
		// 61FAE6E8975F0FA7B3DD4D5A410A240E86A58F7B+E
		static RelocPtr<InputManager*> g_inputManager(0x02EC5BD0);
		return *g_inputManager;
	}

	MenuControls * MenuControls::GetSingleton(void)
	{
		// DC378767BEB0312EBDE098BC7E0CE53FCC296377+D9
		static RelocPtr<MenuControls*> g_menuControls(0x02F003F8);
		return *g_menuControls;
	}

	void MenuControls::ProcessCommonInputEvent_Hook(MenuControls * control, InputEvent * e) //native...
	{
		if (e) {
			auto & rime = RimeManager::GetSingleton();
			switch (e->eventType)
			{
			case InputEvent::kEventType_Button:
				{
				if (rime.IsRimeEnabled()) {
					if (rime.IsRimeComposing()) //在输入法组字的时候屏蔽UI的键位输入。
						return;
					auto * buttonEvent = (ButtonEvent*)e;
					if (buttonEvent->deviceType == kDeviceType_Keyboard && buttonEvent->IsDown()) {
						UInt32 keyMask = buttonEvent->keyMask;
						switch (keyMask)
						{
						case KeyCode::LeftShift:
							rime.PostCommand(RimeCommandMessage(RimeCommandMessage::kCommandType_SwitchAsciiMode));
							break;
						case KeyCode::RightAlt:
							rime.PostCommand(RimeCommandMessage(RimeCommandMessage::kCommandType_SwitchFullShape));
							break;
						case KeyCode::RightControl:
							rime.PostCommand(RimeCommandMessage(RimeCommandMessage::kCommandType_SwitchSimplification));
							break;
						default:
							break;
						}
					}
				}
				}
				break;
			case InputEvent::kEventType_MouseMove:
				break;
			case InputEvent::kEventType_Char:
				{
				//auto * menuMgr = MenuManager::GetSingleton();
				//auto * uiStringHolder = InputStringHolder::GetSingleton();
				CharEvent * charEvent = (CharEvent*)e;
				if (rime.IsRimeEnabled() && (charEvent->keyCode != 0x60 /*|| rime.IsRimeComposing()*/)) {//~key is for console open/close...IsConsoleMode doesn't work as intended...
					rime.PostCommand(RimeCharMessage(charEvent->keyCode));
					return;
				}
				}
				break;
			default:
				break;
			}
		}
		control->ProcessCommonInputEvent(e);
	}

	EventResult MenuControls::ReceiveEvent_Hook(InputEvent ** evns, void * dispatcher)
	{
		for (InputEvent * e = *evns; e; e = e->next) {
			auto & rime = RimeManager::GetSingleton();
			switch (e->eventType)
			{
			case InputEvent::kEventType_Button:
			{
				if (rime.IsRimeEnabled()) {
					auto * buttonEvent = (ButtonEvent*)e;
					if (buttonEvent->deviceType == kDeviceType_Keyboard && buttonEvent->IsDown()) {
						UInt32 keyMask = buttonEvent->keyMask;
						UInt32 keyCode = RimeManager::TranslateKeycode(keyMask);
						_MESSAGE("KEYCODE: %04X", keyMask);
					}
				}
				break;
			case InputEvent::kEventType_MouseMove:
				break;
			case InputEvent::kEventType_Char:
			{

			}
			break;
			default:
				break;
			}
			}
		}
		return (this->*ReceiveEvent_RealFunc)(evns, dispatcher);
	}

	void MenuControls::InitHooks()
	{
		g_branchTrampoline.Write5Call(RelocAddr<uintptr_t>(0x8A7DEB), (uintptr_t)ProcessCommonInputEvent_Hook);
		g_branchTrampoline.Write5Call(RelocAddr<uintptr_t>(0x8A7E22), (uintptr_t)ProcessCommonInputEvent_Hook);
		//ReceiveEvent_RealFunc = HookUtil::SafeWrite64(RelocAddr<uintptr_t>(0x16B8308) + 0x8 * 1, &MenuControls::ReceiveEvent_Hook);
	}

	MenuControls::FnReceiveEvent MenuControls::ReceiveEvent_RealFunc = nullptr;
}


