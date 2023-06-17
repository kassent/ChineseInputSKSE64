#pragma once
#include <thread>
#include <memory>
#include <shared_mutex>
#include <atomic>
#include <vector>
#include <condition_variable>
#include <queue>
#include <mutex>
#include <utility>
#include <boost/dll/import.hpp>
#include <boost/circular_buffer.hpp>
#include "GameTypes.h"
//
class CommonMessageQueue;



enum class MessageType {
	kMessageType_Char,
	kMessageType_Option,
	kMessageType_Command,
	kMessageType_Invalid
};
class AbstractMessage
{
public:
	AbstractMessage(MessageType a_messageType) : messageType_(a_messageType) { }
	virtual ~AbstractMessage() = default;
	MessageType type() const { return messageType_; }
	//operator MessageType() { return messageType_; }
private:
	MessageType     messageType_ = MessageType::kMessageType_Invalid;
};

class RimeCharMessage : public AbstractMessage
{
public:
	enum {
		kInvalidChar = -1
	};
	RimeCharMessage(UInt32 a_asciiCode) : AbstractMessage(MessageType::kMessageType_Char), asciiCode(a_asciiCode) { }
	virtual ~RimeCharMessage() = default;
public:
	UInt32			 asciiCode;
};

class RimeCommandMessage : public AbstractMessage {
public:
	enum {
		kCommandType_Invalid,
		kCommandType_ClearComposition,
		kCommandType_CommitCompositon,
		kCommandType_SwitchAsciiMode,
		kCommandType_SwitchFullShape,
		kCommandType_SwitchSimplification,
		kCommandType_ExitLoop
	};
	RimeCommandMessage(UInt32 a_command) : AbstractMessage(MessageType::kMessageType_Command), command(a_command) { }
	virtual ~RimeCommandMessage() = default;
public:
	UInt32			command;
};

class CommonMessageQueue
{
public:
	typedef std::unique_ptr<AbstractMessage> ContainerEntryType;

	CommonMessageQueue();

	~CommonMessageQueue() = default;

	void Push(ContainerEntryType&& entry);

	ContainerEntryType Pop();

private:

	static constexpr size_t MAX_QUEUE_SIZE = 128;
	// Queue for the Msgs
	std::queue<ContainerEntryType> queue_;

	// Mutex to protect access to the queue
	std::mutex  queueMutex_;

	// Condition variable to wait for when getting Msgs from the queue
	std::condition_variable queueCond_;
};

using RE::GFxCharEvent;
class RimeManager
{
public:
	struct RimeIndicator {
		enum {
			kFlag_Disabled = 1,
			kFlag_Composing = 1 << 1,
			kFlag_Ascii = 1 << 2,
			kFlag_FullShape = 1 << 3,
			kFlag_Simplified = 1 << 4
		};
		inline void Clear() {
			curStatus = kFlag_Disabled;
			composition.clear();
			commit.clear();
			schemaName.clear();
			candidateList.clear();
			numPages = curPageIndex = curHighlightIndex = 0;
		}
		inline bool IsComposing() const { return !(curStatus & kFlag_Disabled) && (curStatus & kFlag_Composing); }
		inline bool IsAsciiMode() const { return curStatus & kFlag_Ascii; };
		inline bool IsFullShape() const { return curStatus & kFlag_FullShape; };
		inline bool IsSimplification() const { return curStatus & kFlag_Simplified; };

		UInt8						curStatus = kFlag_Disabled;
		std::string					schemaName;
		std::string					composition;
		std::string					commit;
		std::vector<std::string>	candidateList;
		UInt32						numPages = 0;
		UInt32						curPageIndex = 0;
		UInt32						curHighlightIndex = 0;
	};

	RimeManager();
	void		StartRimeService();
	void		QueryIndicator(RimeIndicator & indicator);
	bool		IsRimeEnabled() const;
	//inline bool IsRimeComposing() { return m_composing_flag; }
	bool		IsRimeComposing();
	void		ClearComposition();


	static RimeManager& GetSingleton();
	//************************************
	// 将部分用于控制输入法的键盘DXScanCode转为输入法能够识别的unicode控制码。
	// Method:    TranslateKeycode
	// FullName:  RimeManager::TranslateKeycode
	// Access:    public static 
	// Returns:   UInt32
	// Qualifier:
	// Parameter: UInt32 keyCode
	//************************************
	static UInt32	TranslateKeycode(UInt32 keyCode);

	template <class T, class ...Args>
	void PostCommand(Args... args) {
		m_command_queue.Push(std::make_unique<T>(std::forward<Args>(args)...));
	}

private:
	void RunRimeService();

	void StopRimeService();

	void SendUnicodeMessage(UInt32 charCode);
private:
	static constexpr size_t UNICODE_BUFFER_SIZE = 0x20;
	//GFxCharEvent
	typedef boost::circular_buffer<GFxCharEvent> circular_buffer_t;

	mutable std::shared_mutex		m_indicator_mutex;
	RimeIndicator					m_indicator;
	CommonMessageQueue				m_command_queue;
	std::unique_ptr<std::thread>	m_rime_thread;
	//std::atomic_bool				m_composing_flag;
	std::atomic_bool				m_is_ready;
	boost::dll::shared_library		m_rime_library;
	circular_buffer_t				m_unicode_buf;
};

