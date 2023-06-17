#pragma once

#include <skse64_common/Utilities.h>
#include "skse64/GameAPI.h"
#include "GameTypes.h"
#include "GFxEvent.h"
namespace rime {

	class  BSWin32KeyboardDevice : public BSKeyboardDevice
	{
	public:
		virtual	void	Initialize(void) override;		// 00 00A6B060
		virtual	void	Process(float arg) override;	// 01 00A6B110 { MapVirtualKey, ToUnicode, ... }

		bool IsPressed(UInt32 keyCode) const
		{
			return (keyCode < sizeof(curState)) && ((curState[keyCode] & 0x80) != 0);
		}

		// @members
		//void		* inputDevice;					// 070 - IDirectInputDevice8 *
		UInt64		pad02C[(0x168 - 0x8) >> 3];	// 
		UInt8		curState[0x100];				// 168
		UInt8		prevState[0x100];				// 268

		DEF_MEMBER_FN(ProcessKeyboardInput, void, 0x0C190F0, UInt32 keyCode, float delta, bool curPressState, bool prevPressState);
	public:
		static void InitHooks();
		//************************************
		// Access:    public static 
		// Returns:   void
		// Parameter: BSWin32KeyboardDevice * pDevice
		// Parameter: UInt32 keyCode
		// Parameter: float delta
		// Parameter: bool curPressState
		// Parameter: bool prevPressState
		//************************************
		static void ProcessKeyboardInput_Hook(BSWin32KeyboardDevice * pDevice, UInt32 keyCode, float delta, bool curPressState, bool prevPressState);
	};
	STATIC_ASSERT(offsetof(BSWin32KeyboardDevice, curState) == 0x168);


	// 90
	class MenuControls
	{
	public:
		using FnReceiveEvent = EventResult (__thiscall MenuControls::*)(InputEvent **, void *);

		virtual			~MenuControls();
		virtual UInt32	Unk_01();

		//	void			** _vtbl;		// 00
		BSTEventSink<MenuModeChangeEvent> menuModeEventSink; // 08
		UInt64			unk10;			// 10
		tArray<void*>	arr18;			// 18
		UInt64			unk30[3];		// 30

		MenuEventHandler* clickHandler;	// 48
		MenuEventHandler* directionHandler;	// 50
		MenuEventHandler* consoleOpenHandler;	// 58
		MenuEventHandler* quickSaveLoadHandler;	// 60
		MenuEventHandler* menuOpenHandler;	// 68
		MenuEventHandler* favoritesHandler;	// 70
		MenuEventHandler* screenshotHandler;	// 78

		UInt8					unk80;			// 80
		UInt8					unk81;			// 81
		bool					remapMode;		// 82
		UInt8					unk83;			// 83
		UInt8					pad84[0x90 - 0x84];	// 84
		static FnReceiveEvent	ReceiveEvent_RealFunc;

		DEF_MEMBER_FN(ProcessCommonInputEvent, void, 0x8A85C0, InputEvent *);
	public:
		//************************************
		// Method:    ReceiveEvent_Hook
		// FullName:  rime::MenuControls::ReceiveEvent_Hook
		// Access:    public 
		// Returns:   EventResult
		// Qualifier:
		// Parameter: InputEvent * * evns
		// Parameter: void * dispatcher
		//************************************
		EventResult ReceiveEvent_Hook(InputEvent ** evns, void * dispatcher);
		//************************************
		// Method:    ProcessCommonInputEvent_Hook
		// FullName:  rime::MenuControls::ProcessCommonInputEvent_Hook
		// Access:    public static 
		// Returns:   void
		// Qualifier:
		// Parameter: MenuControls * control
		// Parameter: InputEvent * e
		//************************************
		static void ProcessCommonInputEvent_Hook(MenuControls * control, InputEvent * e);
		//************************************
		// Method:    GetSingleton
		// FullName:  rime::MenuControls::GetSingleton
		// Access:    public static 
		// Returns:   rime::MenuControls *
		// Qualifier:
		// Parameter: void
		//************************************
		static MenuControls *	GetSingleton(void);
		//************************************
		// Method:    InitHooks
		// FullName:  rime::MenuControls::InitHooks
		// Access:    public static 
		// Returns:   void
		// Qualifier:
		//************************************
		static void InitHooks();
	};

	STATIC_ASSERT(offsetof(MenuControls, remapMode) == 0x082);
	// 258

	class UIStringHolder
	{
	public:
		void			* unk00;					// 000
		BSFixedString	faderData;					// 008 "FaderData"
		BSFixedString	hudData;					// 010 "HUDData"
		BSFixedString	hudCamData;					// 018 "HUDCamData"
		BSFixedString	floatingQuestMarkers;		// 020 "FloatingQuestMarkers"
		BSFixedString	consoleData;				// 028 "ConsoleData"
		BSFixedString	quantityData;				// 030 "QuantityData"
		BSFixedString	messageBoxData;				// 038 "MessageBoxData"
		BSFixedString	bsUIScaleformData;			// 040 "BSUIScaleformData"
		BSFixedString	bsUIMessageData;			// 048 "BSUIMessageData"
		BSFixedString	bsUIAnalogData;				// 050 "BSUIAnalogData"
		BSFixedString	inventoryUpdateData;		// 058 "InventoryUpdateData"
		BSFixedString	refHandleUIData;			// 060 "RefHandleUIData"
		BSFixedString	tesFormUIData;				// 068 "TESFormUIData"
		BSFixedString	loadingMenuData;			// 070 "LoadingMenuData"
		BSFixedString	kinectStateData;			// 078 "KinectStateChangeData"
		BSFixedString	kinectUserEventData;		// 080 "KinectUserEventData"
		BSFixedString	inventoryMenu;				// 088 "InventoryMenu"
		BSFixedString	console;					// 090 "Console"
		BSFixedString	dialogueMenu;				// 098 "Dialogue Menu"
		BSFixedString	hudMenu;					// 0A0 "HUD Menu"
		BSFixedString	mainMenu;					// 0A8 "Main Menu"
		BSFixedString	messageBoxMenu;				// 0B0 "MessageBoxMenu"
		BSFixedString	cursorMenu;					// 0B8 "Cursor Menu"
		BSFixedString	faderMenu;					// 0C0 "Fader Menu"
		BSFixedString	magicMenu;					// 0C8 "MagicMenu"
		BSFixedString	topMenu;					// 0D0 "Top Menu"
		BSFixedString	overlayMenu;				// 0D8 "Overlay Menu"
		BSFixedString	overlayInteractionMenu;		// 0E0 "Overlay Interaction Menu"
		BSFixedString	loadingMenu;				// 0E8 "Loading Menu"
		BSFixedString	tweenMenu;					// 0F0 "TweenMenu"
		BSFixedString	barterMenu;					// 0F8 "BarterMenu"
		BSFixedString	giftMenu;					// 100 "GiftMenu"
		BSFixedString	debugTextMenu;				// 108 "Debug Text Menu"
		BSFixedString	mapMenu;					// 110 "MapMenu"
		BSFixedString	lockpickingMenu;			// 118 "Lockpicking Menu"
		BSFixedString	quantityMenu;				// 120 "Quantity Menu"
		BSFixedString	statsMenu;					// 128 "StatsMenu"
		BSFixedString	containerMenu;				// 130 "ContainerMenu"
		BSFixedString	sleepWaitMenu;				// 138 "Sleep/Wait Menu"
		BSFixedString	levelUpMenu;				// 140 "LevelUp Menu"
		BSFixedString	journalMenu;				// 148 "Journal Menu"
		BSFixedString	bookMenu;					// 150 "Book Menu"
		BSFixedString	favoritesMenu;				// 158 "FavoritesMenu"
		BSFixedString	raceSexMenu;				// 160 "RaceSex Menu"
		BSFixedString	craftingMenu;				// 168 "Crafting Menu"
		BSFixedString	trainingMenu;				// 170 "Training Menu"
		BSFixedString	mistMenu;					// 178 "Mist Menu"
		BSFixedString	tutorialMenu;				// 180 "Tutorial Menu"
		BSFixedString	creditsMenu;				// 188 "Credits Menu"
		BSFixedString	modManagerMenu;				// 190 "Mod Manager Menu"		- NEW IN SE
		BSFixedString	titleSequenceMenu;			// 198 "TitleSequence Menu"
		BSFixedString	consoleNativeUIMenu;		// 1A0 "Console Native UI Menu"
		BSFixedString	kinectMenu;					// 1A8 "Kinect Menu"
		BSFixedString	loadWaitSpinner;			// 1B0 "LoadWaitSpinner"		- NEW IN SE
		BSFixedString	streamingInstallMenu;		// 1B8	"StreamingInstallMenu"  - NEW IN SE
		BSFixedString	textWidget;					// 1C0 "TextWidget"
		BSFixedString	buttonBarWidget;			// 1C8 "ButtonBarWidget"
		BSFixedString	graphWidget;				// 1D0 "GraphWidget"
		BSFixedString	textureWidget;				// 1D8 "TextWidget"
		BSFixedString	uiMenuOK;					// 1E0 "UIMenuOK"
		BSFixedString	uiMenuCancel;				// 1E8 "UIMenuCancel"
		BSFixedString	showText;					// 1F0 "Show Text"
		BSFixedString	hideText;					// 1F8 "Hide Text"
		BSFixedString	showList;					// 200 "Show List"
		BSFixedString	voiceReady;					// 208 "Voice Ready"
		BSFixedString	dmfoStr;					// 210 "DMFOStr"
		BSFixedString	showJournal;				// 218 "Show Journal"
		BSFixedString	journalSettingsSaved;		// 220 "Journal Settings Saved"
		BSFixedString	closeMenu;					// 228 "CloseMenu"
		BSFixedString	closingAllMenus;			// 230 "Closing All Menus"
		BSFixedString	refreshMenu;				// 238 "RefreshMenu"
		BSFixedString	cancelLoading;				// 240 "CancelLoading"		- NEW IN SE
		BSFixedString	menuTextureDegradeEvent;	// 248 "Menu Texture Degrade Event"
		BSFixedString	diamondMarker;				// 250 "<img src='DiamondMarker' width='10' height='15' align='baseline' vspace='5'>"

		static UIStringHolder *	GetSingleton(void)
		{
			// 81B349AB8ABC9944E48046819F0345AB0526CDB5+9
			static RelocPtr<UIStringHolder *> g_UIStringHolder(0x01EC0A78);
			return *g_UIStringHolder;
		}
	};

	typedef void * (*_SendBSUIScaleformData)(BSFixedString & name, GFxEvent * e);
	extern RelocAddr<_SendBSUIScaleformData> SendBSUIScaleformData;

}


