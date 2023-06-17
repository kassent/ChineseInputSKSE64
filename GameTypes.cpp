#include "GameTypes.h"
#include <RE/InputEvent.h>

namespace RE 
{
	InputEvent::~InputEvent() {

	}

	bool InputEvent::HasIDCode() const {
		return false;
	}

	const BSFixedString& InputEvent::QUserEvent() const {
		static BSFixedString nullStr;
		return nullStr;
	}
}
