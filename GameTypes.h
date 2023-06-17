#pragma once
#include <RE/InputEvent.h>
#include <RE/GFxEvent.h>

namespace RE
{
	// 20
	class CharEvent : public InputEvent
	{
	public:
		CharEvent(UInt32 code) : unicode(code) {
			device = INPUT_DEVICE::kKeyboard;	// 08
			eventType = INPUT_EVENT_TYPE::kChar;	// 0C
			next = nullptr;		// 10
		}
		UInt32			unicode;		// 18 (ascii code)
	};
	STATIC_ASSERT(sizeof(CharEvent) == 0x20);


	// 0C
	// Confirmed in Skyrim.SE.
	class GFxCharEvent : public GFxEvent
	{
	public:
		GFxCharEvent(UInt32 a_wcharCode, UInt8 a_keyboardIndex = 0) : wcharCode(a_wcharCode), keyboardIndex(a_keyboardIndex)
		{
			type = GFxEvent::EventType::kCharEvent;
		}

		// @members
		UInt32	wcharCode;			// 04
		UInt8	keyboardIndex;		// 08
	};
	STATIC_ASSERT(sizeof(GFxCharEvent) == 0xC);
}


