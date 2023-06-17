#pragma once
#include <RE/BSWin32KeyboardDevice.h>
#include <RE/MenuControls.h>
#include <RE/InterfaceStrings.h>
#include "GameTypes.h"

using RE::InputEvent;
using RE::BSWin32KeyboardDevice;
using RE::MenuControls;
using RE::BSFixedString;
using RE::GFxEvent;
// 20
class BSWin32KeyboardDeviceEx : public BSWin32KeyboardDevice
{
private:
	void ProcessKeyboardInput(UInt32 a_keyCode, float a_delta, bool a_curPressState, bool a_prevPressState);
	void ProcessKeyboardInput_Hook(UInt32 a_keyCode, float a_delta, bool a_curPressState, bool a_prevPressState);
public:
	static void InitHooks();
};

class MenuControlsEx : public MenuControls
{
private:
	void ProcessCommonInputEvent_Hook(InputEvent* a_event);
public:
	void ProcessCommonInputEvent(InputEvent* a_event);
public:
	static void InitHooks();
	static MenuControlsEx* GetSingleton();
};

namespace UI
{
	void SendBSUIScaleformData(const BSFixedString& a_name, GFxEvent* a_event);
}
