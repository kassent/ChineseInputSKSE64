#pragma once

enum CharacterSet{
	kCharacterSet_Default,
	kCharacterSet_Chinese,
	kCharacterSet_Korean,
	KCharacterSet_Japanese,
	kCharacterSet_Cyrillic,
	kCharacterSet_Thai,
	kCharacterSet_Vietnamese,
	kMaxCharacterSet
};

class ConfigLoader
{
	ConfigLoader();
public:
	static ConfigLoader& GetSingleton();

	UInt32			uiOffsetX = 50;
	UInt32			uiOffsetY = 50;
	UInt32			characterSet = kCharacterSet_Chinese;
};

