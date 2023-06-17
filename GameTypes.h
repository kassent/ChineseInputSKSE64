#pragma once
#include <skse64_common/Utilities.h>
#include "skse64/GameAPI.h"

namespace rime {


	class TESForm;

	// 08
	struct BSIntrusiveRefCounted
	{
	public:
		volatile UInt32	m_refCount;	// 00
		UInt32			unk04;		// 04
	};

	// 04
	template <typename T>
	class BSTSmartPointer
	{
	public:
		// refcounted
		T	* ptr;
	};

	// 08
	class SimpleLock
	{
		enum
		{
			kFastSpinThreshold = 10000
		};

		volatile SInt32	threadID;	// 00
		volatile UInt32	lockCount;	// 04

	public:
		SimpleLock() : threadID(0), lockCount(0) {}

		void Lock(UInt32 pauseAttempts = 0);
		void Release(void);
	};
	STATIC_ASSERT(sizeof(SimpleLock) == 0x8);

	class SimpleLocker
	{
	public:
		SimpleLocker(SimpleLock * dataHolder) { m_lock = dataHolder; m_lock->Lock(); }
		~SimpleLocker() { m_lock->Release(); }

	protected:
		SimpleLock	* m_lock;
	};

	// 08
	class BSReadWriteLock
	{
		enum
		{
			kFastSpinThreshold = 10000,
			kLockWrite = 0x80000000,
			kLockCountMask = 0xFFFFFFF
		};

		volatile SInt32	threadID;	// 00
		volatile UInt32	lockValue;	// 04

	public:
		BSReadWriteLock() : threadID(0), lockValue(0) {}

		//void LockForRead();
		//void LockForWrite();
		MEMBER_FN_PREFIX(BSReadWriteLock);
		DEFINE_MEMBER_FN(LockForRead, void, 0x00C072D0);
		DEFINE_MEMBER_FN(LockForWrite, void, 0x00C07350);
		DEFINE_MEMBER_FN(UnlockRead, void, 0x00C07590);
		DEFINE_MEMBER_FN(UnlockWrite, void, 0x00C075A0);
		DEFINE_MEMBER_FN(LockForReadAndWrite, void, 0x00C07450);
		DEFINE_MEMBER_FN(TryLockForWrite, bool, 0x00C07540);
	};
	STATIC_ASSERT(sizeof(BSReadWriteLock) == 0x8);

	class BSReadLocker
	{
	public:
		BSReadLocker(BSReadWriteLock * lock) { m_lock = lock; CALL_MEMBER_FN(m_lock, LockForRead)(); }
		~BSReadLocker() { CALL_MEMBER_FN(m_lock, UnlockRead)(); }

	protected:
		BSReadWriteLock    * m_lock;
	};

	class BSWriteLocker
	{
	public:
		BSWriteLocker(BSReadWriteLock * lock) { m_lock = lock; CALL_MEMBER_FN(m_lock, LockForWrite)(); }
		~BSWriteLocker() { CALL_MEMBER_FN(m_lock, UnlockWrite)(); }

	protected:
		BSReadWriteLock    * m_lock;
	};

	class BSReadAndWriteLocker
	{
	public:
		BSReadAndWriteLocker(BSReadWriteLock * lock) { m_lock = lock; CALL_MEMBER_FN(m_lock, LockForReadAndWrite)(); }
		~BSReadAndWriteLocker() { CALL_MEMBER_FN(m_lock, UnlockWrite)(); }

	protected:
		BSReadWriteLock    * m_lock;
	};

	// 80808
	class StringCache
	{
	public:
		struct Ref
		{
			const char	* data;

			MEMBER_FN_PREFIX(Ref);
			DEFINE_MEMBER_FN(ctor, Ref *, 0x00C28BF0, const char * buf);
			// E728381B6B25FD30DF9845889144E86E5CC35A25+38
			DEFINE_MEMBER_FN(ctor_ref, Ref *, 0x00C28C80, const Ref & rhs);
			DEFINE_MEMBER_FN(Set, Ref *, 0x00C28D60, const char * buf);
			// F3F05A02DE2034133B5965D694745B6369FC557D+F3
			DEFINE_MEMBER_FN(Set_ref, Ref *, 0x00C28E20, const Ref & rhs);
			// 77D2390F6DC57138CF0E5266EB5BBB0ACABDFBE3+A0
			DEFINE_MEMBER_FN(Release, void, 0x00C28D40);

			Ref();
			Ref(const char * buf);

			void Release() { CALL_MEMBER_FN(this, Release)(); }

			bool operator==(const Ref& lhs) const { return data == lhs.data; }
			bool operator<(const Ref& lhs) const { return data < lhs.data; }

			const char * c_str() const { return operator const char *(); }
			const char * Get() const { return c_str(); }
			operator const char *() const { return data ? data : ""; }
		};

		// 10
		struct Lock
		{
			UInt32	unk00;	// 00 - set to 80000000 when locked
			UInt32	pad04;	// 04
			UInt64	pad08;	// 08
		};

		void	* lut[0x10000];	// 00000
		Lock	lock[0x80];		// 80000
		UInt8	isInit;			// 80800
	};

	typedef StringCache::Ref BSFixedString;

	//class BSAutoFixedString : public BSFixedString
	//{
	//public:
	//	BSAutoFixedString() : BSFixedString() { }
	//	BSAutoFixedString(const char * buf) : BSFixedString(buf) { }
	//
	//	~BSAutoFixedString()
	//	{
	//		Release();
	//	}
	//};

	// 10
	class BSString
	{
	public:
		BSString() :m_data(NULL), m_dataLen(0), m_bufLen(0) { }
		~BSString();

		const char *	Get(void) const;

		MEMBER_FN_PREFIX(BSString);
		DEFINE_MEMBER_FN(Set, bool, 0x000F9E90, const char * str, UInt32 len);	// len default 0

	private:
		char	* m_data;	// 00
		UInt16	m_dataLen;	// 08
		UInt16	m_bufLen;	// 0A
		UInt32	pad0C;		// 0C
	};


	// Container types

	// 18
	template <class T, int nGrow = 10, int nShrink = 10>
	class tArray
	{
	public:
		T* entries;			// 00
		UInt32 capacity;	// 08
		UInt32 pad0C;		// 0C
		UInt32 count;		// 10
		UInt32 pad14;		// 14

		tArray() : entries(NULL), capacity(0), count(0), pad0C(0), pad14(0) { }

		T& operator[](UInt64 index)
		{
			return entries[index];
		}

		void Clear()
		{
			Heap_Free(entries);
			entries = NULL;
			capacity = 0;
			count = 0;
		}

		bool Allocate(UInt32 numEntries)
		{
			entries = (T *)Heap_Allocate(sizeof(T) * numEntries);
			if (!entries) return false;

			for (UInt32 i = 0; i < numEntries; i++)
				new (&entries[i]) T;

			capacity = numEntries;
			count = numEntries;

			return true;
		}

		bool CopyFrom(const tArray<T> * rhs)
		{
			if (rhs->count == 0) return false;
			if (!rhs->entries) return false;

			if (entries)
				Clear();

			if (!Allocate(rhs->count)) return false;
			memcpy(entries, rhs->entries, sizeof(T) * count);
			return true;
		}



		bool Resize(UInt32 numEntries)
		{
			if (numEntries == capacity)
				return false;

			if (!entries) {
				Allocate(numEntries);
				return true;
			}
			if (numEntries < capacity) {
				// Delete the truncated entries
				for (UInt32 i = numEntries; i < capacity; i++)
					delete &entries[i];
			}

			T * newBlock = (T *)Heap_Allocate(sizeof(T) * numEntries);						// Create a new block
			memmove_s(newBlock, sizeof(T) * numEntries, entries, sizeof(T) * numEntries);	// Move the old memory to the new block
			if (numEntries > capacity) {														// Fill in new remaining entries
				for (UInt32 i = capacity; i < numEntries; i++)
					new (&entries[i]) T;
			}
			Heap_Free(entries);																// Free the old block
			entries = newBlock;																// Assign the new block
			capacity = numEntries;															// Capacity is now the number of total entries in the block
			count = min(capacity, count);													// Count stays the same, or is truncated to capacity
			return true;
		}

		bool Push(const T & entry)
		{
			if (!entries || count + 1 > capacity) {
				if (!Grow(nGrow))
					return false;
			}

			new (&entries[count]) T(entry);
			count++;
			return true;
		};

		bool Insert(UInt32 index, const T & entry)
		{
			if (!entries || index < count)
				return false;

			entries[index] = entry;
			return true;
		};

		bool Remove(UInt32 index)
		{
			if (!entries || index >= count)
				return false;

			// This might not be right for pointer types...
			(&entries[index])->~T();

			if (index + 1 < count) {
				UInt32 remaining = count - index;
				memmove_s(&entries[index + 1], sizeof(T) * remaining, &entries[index], sizeof(T) * remaining); // Move the rest up
			}
			count--;

			if (capacity > count + nShrink)
				Shrink();

			return true;
		}

		bool Shrink()
		{
			if (!entries || count == capacity) return false;

			try {
				UInt32 newSize = count;
				T * oldArray = entries;
				T * newArray = (T *)Heap_Allocate(sizeof(T) * newSize); // Allocate new block
				memmove_s(newArray, sizeof(T) * newSize, entries, sizeof(T) * newSize); // Move the old block
				entries = newArray;
				capacity = count;
				Heap_Free(oldArray); // Free the old block
				return true;
			}
			catch (...) {
				return false;
			}

			return false;
		}

		bool Grow(UInt32 numEntries)
		{
			if (!entries) {
				entries = (T *)Heap_Allocate(sizeof(T) * numEntries);
				count = 0;
				capacity = numEntries;
				return true;
			}

			try {
				UInt32 oldSize = capacity;
				UInt32 newSize = oldSize + numEntries;
				T * oldArray = entries;
				T * newArray = (T *)Heap_Allocate(sizeof(T) * newSize); // Allocate new block
				if (oldArray)
					memmove_s(newArray, sizeof(T) * newSize, entries, sizeof(T) * capacity); // Move the old block
				entries = newArray;
				capacity = newSize;

				if (oldArray)
					Heap_Free(oldArray); // Free the old block

				for (UInt32 i = oldSize; i < newSize; i++) // Allocate the rest of the free blocks
					new (&entries[i]) T;

				return true;
			}
			catch (...) {
				return false;
			}

			return false;
		}

		bool GetNthItem(UInt64 index, T& pT) const
		{
			if (index < count) {
				pT = entries[index];
				return true;
			}
			return false;
		}

		SInt64 GetItemIndex(T & pFind) const
		{
			for (UInt64 n = 0; n < count; n++) {
				T& pT = entries[n];
				if (pT == pFind)
					return n;
			}
			return -1;
		}

		DEFINE_STATIC_HEAP(Heap_Allocate, Heap_Free)
	};

	//template<class T>
	//class tMutexArray : public tArray<T>
	//{
	//public:
	//	SimpleLock lock;	// 18
	//};

	typedef tArray<UInt64> UnkArray;
	typedef tArray<TESForm*> UnkFormArray;

	// Returns if/where the element was found, otherwise indexOut can be used as insert position
	template <typename T>
	bool GetSortIndex(tArray<T> & arr, T & elem, SInt32 & indexOut)
	{
		UInt32 count = arr.count;
		if (count == 0)
		{
			indexOut = 0;
			return false;
		}

		SInt32 leftIdx = 0;
		SInt32 rightIdx = count - 1;

		while (true)
		{
			UInt32 pivotIdx = leftIdx + ((rightIdx - leftIdx) / 2);

			T & p = arr[pivotIdx];

			if (elem == p)
			{
				indexOut = pivotIdx;
				return true;
			}
			else if (elem > p)
			{
				leftIdx = pivotIdx + 1;
			}
			else
			{
				rightIdx = pivotIdx - 1;
			}

			if (leftIdx > rightIdx)
			{
				indexOut = leftIdx;
				return false;
			}
		}
	}

	enum {
		eListCount = -3,
		eListEnd = -2,
		eListInvalid = -1,
	};


	template <typename T> class BSTEventSink;

	enum EventResult
	{
		kEvent_Continue = 0,
		kEvent_Abort
	};

	// 058 
	template <typename EventT, typename EventArgT = EventT>
	class EventDispatcher
	{
		typedef BSTEventSink<EventT> SinkT;

		tArray<SinkT*>		eventSinks;			// 000
		tArray<SinkT*>		addBuffer;			// 018 - schedule for add
		tArray<SinkT*>		removeBuffer;		// 030 - schedule for remove
		SimpleLock			lock;				// 048
		bool				stateFlag;			// 050 - some internal state changed while sending
		char				pad[7];				// 051

		// Note: in SE there are multiple identical copies of all these functions 
		MEMBER_FN_PREFIX(EventDispatcher);
		// 66B1C7AC473D5EA48E4FD620BBFE0A06392C5885+66
		DEFINE_MEMBER_FN(AddEventSink_Internal, void, 0x0056B600, SinkT * eventSink);
		// ??_7BGSProcedureShoutExecState@@6B@ dtor | +43
		DEFINE_MEMBER_FN(RemoveEventSink_Internal, void, 0x00423B70, SinkT * eventSink);
		// D6BA7CEC95B2C2B9C593A9AEE7F0ADFFB2C10E11+456
		DEFINE_MEMBER_FN(SendEvent_Internal, void, 0x00177DC0, EventArgT * evn);

	public:

		EventDispatcher() : stateFlag(false) {}

		void AddEventSink(SinkT * eventSink) { CALL_MEMBER_FN(this, AddEventSink_Internal)(eventSink); }
		void RemoveEventSink(SinkT * eventSink) { CALL_MEMBER_FN(this, RemoveEventSink_Internal)(eventSink); }
		void SendEvent(EventArgT * evn) { CALL_MEMBER_FN(this, SendEvent_Internal)(evn); }
	};
	STATIC_ASSERT(sizeof(EventDispatcher<void*>) == 0x58);


	enum
	{
		kDeviceType_Keyboard = 0,
		kDeviceType_Mouse,
		kDeviceType_Gamepad
	};
	// 18
	class InputEvent
	{
	public:
		enum
		{
			kEventType_Button = 0,
			kEventType_MouseMove,
			kEventType_Char,
			kEventType_Thumbstick,
			kEventType_DeviceConnect,
			kEventType_Kinect
		};

		virtual					~InputEvent() { }
		virtual bool			IsIDEvent() { return false; }
		virtual BSFixedString *	GetControlID() { return nullptr; }

		//	void			** _vtbl;	// 00
		UInt32			deviceType;	// 08
		UInt32			eventType;	// 0C
		InputEvent		* next;		// 10
	};


	class IDEvent
	{
	public:
		BSFixedString	controlID;	// 00
	};

	// 30
	class ButtonEvent : public IDEvent, public InputEvent
	{
	public:
		virtual					~ButtonEvent();
		virtual bool			IsIDEvent();
		virtual BSFixedString *	GetControlID();

		// 18 -> controlID from IDEvent
		UInt32			keyMask;	// 20 (00000038 when ALT is pressed, 0000001D when STRG is pressed)
		UInt32			pad24;		// 24
		float			pressure;	// 28		// 28 - isn't this a float?
		float			timer;		// 2C (hold duration)

		inline bool IsPressed() const {
			return pressure > 0;
		}

		inline bool IsDown() const {
			return (pressure > 0) && (timer == 0.0f);
		}

		inline bool IsUp() const {
			return (pressure == 0) && (timer > 0.0f);
		}
	};

	class MouseMoveEvent : public IDEvent, public InputEvent
	{
	};

	// 20
	class CharEvent : public InputEvent
	{
	public:
		CharEvent(UInt32 unicode) : keyCode(unicode) {
			deviceType = kDeviceType_Keyboard;	// 08
			eventType = kEventType_Char;	// 0C
			next = nullptr;		// 10
		}
		UInt32			keyCode;		// 18 (ascii code)
	};

	class BSInputDevice
	{
	public:

		// SE: destructor is now at top
		virtual			~BSInputDevice();
		virtual void	Initialize(void);		// pure
		virtual	void	Process(float delta);	// pure
		virtual	void	Unk_03(void);		// pure

		// These 3 ones added in SE. The three of them do some call to CRC functions but I didn't do further research
		virtual	bool	Unk_04(UInt32 unk0, void *unk1);
		virtual bool	Unk_05(void *unk0);
		virtual bool	Unk_06(UInt32 unk0, UInt32 *unk1);

		virtual bool	IsEnabled(void);	// Always 1 for non-gamepad?	
		virtual void	Unk_08(void* unk1);		// pure 	
	};


	class BSKeyboardDevice : public BSInputDevice
	{
	public:
		// @members

	};


	template <typename T>
	class BSTEventSource
	{
	public:

		tArray<void*> unk00; // 08
		UnkArray	unk18;	// 18
		UnkArray	unk30;	// 30
		UInt32		unk48;	// 48
		UInt32		unk4C;	// 4C
		UInt8		unk50;	// 50
		UInt8		pad51[7];	// 51
	};
	STATIC_ASSERT(sizeof(BSTEventSource<void*>) == 0x58);


	// 128 
	class InputManager
	{
	public:
		enum
		{
			kContext_Gameplay = 0,
			kContext_MenuMode,
			kContext_Console,
			kContext_ItemMenu,
			kContext_Inventory,
			kContext_DebugText,
			kContext_Favorites,
			kContext_Map,
			kContext_Stats,
			kContext_Cursor,
			kContext_Book,
			kContext_DebugOverlay,
			kContext_Journal,
			kContext_TFCMode,
			kContext_MapDebug,
			kContext_Lockpicking,
			kContext_Favor,
			kContextCount = 17
		};

		struct InputContext
		{
			// 18
			struct Mapping
			{
				BSFixedString	name;		// 00
				UInt32			buttonID;	// 08
				UInt32			sortIndex;	// 0C
				UInt32			unk10;		// 10
				UInt32			pad14;		// 14
			};

			tArray<Mapping>	keyboardMap;
			tArray<Mapping>	mouseMap;
			tArray<Mapping>	gamepadMap;
		};


		void*			unkPtr000;					// 000
		BSTEventSource<void *>	unk008;				// 008 - TODO: template type
		InputContext	* context[kContextCount];	// 060
		tArray<void*>	unk0E8;						// 0E8
		tArray<void*>	unk100;						// 100
		UInt32			unk118;						// 118 - init'd to 0xFFFFFFFF
		UInt32			unk11C;						// 11C - init'd to 0x80000000
		UInt8			allowTextInput;				// 120
		UInt8			unk121;						// 121
		UInt8			unk122;						// 122
		UInt8			pad[5];						// 123

		static InputManager *	GetSingleton(void);

		inline bool	IsTextInputEnabled() { return allowTextInput != 0; }
	};
	STATIC_ASSERT(sizeof(InputManager) == 0x128);


	// 10 
	struct MenuOpenCloseEvent
	{
		BSFixedString	menuName;	// 00
		bool			opening;	// 08
		char			pad[7];
	};

	// Todo
	struct MenuModeChangeEvent
	{
	};

	// 08 
	template <typename T>
	class BSTEventSink
	{
	public:
		virtual ~BSTEventSink() { };
		virtual	EventResult	ReceiveEvent(T * evn, EventDispatcher<T> * dispatcher) { return kEvent_Continue; }; // pure
	//	void	** _vtbl;	// 00
	};

	class MenuEventHandler : public BSTEventSink <MenuOpenCloseEvent>
	{
	public:
		virtual EventResult		ReceiveEvent(MenuOpenCloseEvent * evn, EventDispatcher<MenuOpenCloseEvent> * dispatcher);
	};

	template <>
	class BSTEventSink <InputEvent>
	{
	public:
		virtual ~BSTEventSink() {}; // todo?
		virtual	EventResult ReceiveEvent(InputEvent ** evn, void * dispatcher) = 0;
	};
}