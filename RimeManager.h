#pragma once
#include <thread>
#include <memory>
#include <shared_mutex>
#include <atomic>
#include <vector>
#include <boost/dll/import.hpp>
#include <boost/circular_buffer.hpp>
#include "polym/Queue.hpp"

class GFxCharEvent;

class RimeMessage : public PolyM::Msg {
public:
	enum 
	{
		kMessageType_Char,
		kMessageType_Option,
		kMessageType_Command,
		kMessageType_Quit
	};
	RimeMessage(UInt32 type_) : type(type_) { }
	virtual ~RimeMessage() = default;
	RimeMessage(const RimeMessage&) = delete;
	RimeMessage& operator=(const RimeMessage&) = delete;

	virtual std::unique_ptr<Msg> move() override {
		return std::unique_ptr<Msg>(new RimeMessage(type));
	}
	virtual UInt32 GetType() { return type; };

private:
	RimeMessage(RimeMessage&&) = default;
	RimeMessage& operator=(RimeMessage&&) = default;
	UInt32			type;
};

class RimeCharMessage : public RimeMessage
{
public:
	RimeCharMessage(UInt32 asciiCode_) : RimeMessage(kMessageType_Char), asciiCode(asciiCode_) { }

	virtual ~RimeCharMessage() = default;
	RimeCharMessage(const RimeCharMessage&) = delete;
	RimeCharMessage& operator=(const RimeCharMessage&) = delete;

	virtual std::unique_ptr<Msg> move() override {
		return std::unique_ptr<Msg>(new RimeCharMessage(asciiCode));
	}
private:
	RimeCharMessage(RimeCharMessage&&) = default;
	RimeCharMessage& operator=(RimeCharMessage&&) = default;
public:
	UInt32			asciiCode;
};

class RimeCommandMessage : public RimeMessage {
public:
	enum {
		kCommandType_ClearComposition,
		kCommandType_CommitCompositon,
		kCommandType_SwitchAsciiMode,
		kCommandType_SwitchFullShape,
		kCommandType_SwitchSimplification
	};
	RimeCommandMessage(UInt32 command_) : RimeMessage(kMessageType_Command), command(command_) { }

	virtual ~RimeCommandMessage() = default;
	RimeCommandMessage(const RimeCommandMessage&) = delete;
	RimeCommandMessage& operator=(const RimeCommandMessage&) = delete;

	virtual std::unique_ptr<Msg> move() override {
		return std::unique_ptr<Msg>(new RimeCommandMessage(command));
	}
private:
	RimeCommandMessage(RimeCommandMessage&&) = default;
	RimeCommandMessage& operator=(RimeCommandMessage&&) = default;
public:
	UInt32				command;
};


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
	void StartRimeService();
	void PostCommand(RimeMessage && msg);
	void QueryIndicator(RimeIndicator & indicator);
	bool IsRimeEnabled() const;
	inline bool IsRimeComposing() { return m_composing_flag; }
	void ClearComposition();
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
private:
	void RunRimeService();

	void StopRimeService();

	void SendUnicodeMessage(UInt32 charCode);
private:
	//GFxCharEvent
	typedef boost::circular_buffer<GFxCharEvent> container_type;

	mutable std::shared_mutex		m_rw_mutex;
	RimeIndicator					m_indicator;
	PolyM::Queue					m_command_queue;
	std::unique_ptr<std::thread>	m_rime_thread;
	std::atomic_bool				m_composing_flag;
	boost::dll::shared_library		m_rime_library;
	container_type					m_unicode_buffer;
};

