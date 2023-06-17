#include "RimeManager.h"

#include <string>
#include <codecvt>
#include <thread>
#include <ShlObj.h>
#include <boost/format.hpp>
#include <RE/ControlMap.h>

#include "Hook_GameInput.h"
#include "rime_api.h"
#include "DXScanCode.h"
#include "Utilities.h"

using namespace rime;
using RE::CharEvent;
//using RE::GFxCharEvent;

decltype(rime_get_api) * dll_rime_get_api = nullptr;

using RimeIndicator = RimeManager::RimeIndicator;

static std::wstring utf8string2wstring(const std::string& str)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	if (!len)
		return std::wstring();
	std::unique_ptr<wchar_t[]> wchar_buf(new wchar_t[len]);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wchar_buf.get(), len);
	wchar_buf[len - 1] = L'\0';
	return wchar_buf.get();
}

static void query_status(RimeStatus *status, RimeIndicator & indicator) {
	//_MESSAGE("schema: %s / %s", status->schema_id, status->schema_name);
	indicator.schemaName = status->schema_name;
	/*
	_MESSAGE("status: ");
	if (status->is_disabled) {
		_MESSAGE("disabled ");
	}
	if (status->is_composing) {
		_MESSAGE("composing ");
	}
	if (status->is_ascii_mode) {
		_MESSAGE("ascii ");
	}
	if (status->is_full_shape) {
		_MESSAGE("full_shape ");
	}
	if (status->is_simplified) {
		_MESSAGE("simplified ");
	}*/
	auto & curStatus = indicator.curStatus;
	status->is_disabled ? curStatus |= RimeIndicator::kFlag_Disabled : curStatus &= ~RimeIndicator::kFlag_Disabled;
	status->is_composing ? curStatus |= RimeIndicator::kFlag_Composing : curStatus &= ~RimeIndicator::kFlag_Composing;
	status->is_ascii_mode ? curStatus |= RimeIndicator::kFlag_Ascii : curStatus &= ~RimeIndicator::kFlag_Ascii;
	status->is_full_shape ? curStatus |= RimeIndicator::kFlag_FullShape : curStatus &= ~RimeIndicator::kFlag_FullShape;
	status->is_simplified ? curStatus |= RimeIndicator::kFlag_Simplified : curStatus &= ~RimeIndicator::kFlag_Simplified;
}

static void query_composition(RimeComposition *composition, RimeIndicator & indicator) {
	const char *preedit = composition->preedit;
	if (!preedit) return;
	auto & label = indicator.composition;
	label.clear();
	size_t len = strlen(preedit);
	size_t start = composition->sel_start;
	size_t end = composition->sel_end;
	size_t cursor = composition->cursor_pos;
	for (size_t i = 0; i <= len; ++i) {
		//if (start < end) {
		//	if (i == start) {
		//		label.append("[");
		//	}
		//	else if (i == end) {
		//		label.append("]");
		//	}
		//}
		if (i == cursor) label.append("|");
		if (i < len)
			label.append(1, preedit[i]);
	}
}

static void query_menu(RimeMenu *menu, RimeIndicator & indicator) {

	auto & candidateList = indicator.candidateList;
	candidateList.clear();
	indicator.curPageIndex = indicator.numPages = indicator.curHighlightIndex = 0;
	if (menu->num_candidates == 0) return;

	indicator.curPageIndex = menu->page_no + 1;
	indicator.numPages = menu->page_size;
	indicator.curHighlightIndex = menu->highlighted_candidate_index;

	for (int i = 0; i < menu->num_candidates; ++i) {
		bool highlighted = i == menu->highlighted_candidate_index;
		candidateList.push_back((boost::format("%1%.  %2%") % (i + 1) % (menu->candidates[i].text)).str());
	}
}

static void query_context(RimeContext *context, RimeIndicator & indicator) {
	if (context->composition.length > 0) {
		query_composition(&context->composition, indicator);
		query_menu(&context->menu, indicator);
	}
}

static void query_rime(RimeSessionId session_id, RimeIndicator & indicator) {
	RimeApi* rime = dll_rime_get_api();

	RIME_STRUCT(RimeCommit, commit);
	RIME_STRUCT(RimeStatus, status);
	RIME_STRUCT(RimeContext, context);

	if (rime->get_status(session_id, &status)) {
		query_status(&status, indicator);
		rime->free_status(&status);
	}

	if (rime->get_commit(session_id, &commit)) {
		//_MESSAGE("commit: %s", commit.text);
		indicator.commit = commit.text;
		rime->free_commit(&commit);
	}

	if (rime->get_context(session_id, &context)) {
		query_context(&context, indicator);
		rime->free_context(&context);
	}
}

static void on_message(void* context_object,
	RimeSessionId session_id,
	const char* message_type,
	const char* message_value) {
	_MESSAGE("message: [%08X] [%s] %s", session_id, message_type, message_value);
}


CommonMessageQueue::CommonMessageQueue()
	: queue_(), queueMutex_(), queueCond_()
{

}

void CommonMessageQueue::Push(ContainerEntryType && entry)
{
	std::lock_guard<std::mutex> lock(queueMutex_);
	if (queue_.size() < MAX_QUEUE_SIZE)
		queue_.push(std::forward<ContainerEntryType>(entry));
	queueCond_.notify_one();
}

CommonMessageQueue::ContainerEntryType CommonMessageQueue::Pop()
{
	std::unique_lock<std::mutex> lock(queueMutex_);
	queueCond_.wait(lock, [this] {return !queue_.empty(); });
	auto entry = std::move(queue_.front());
	queue_.pop();
	return entry;
}


RimeManager::RimeManager() : m_is_ready(false), m_unicode_buf(UNICODE_BUFFER_SIZE)
{

}


void RimeManager::StartRimeService()
{
	try {
		auto profile_directory = GetProfileDirectory();
		m_rime_library.load(profile_directory + "rime.dll");
		if (m_rime_library.is_loaded()) {
			if (m_rime_library.has("rime_get_api")) {
				dll_rime_get_api = m_rime_library.get<decltype(rime_get_api)>("rime_get_api");
				m_rime_thread.reset(new std::thread(std::bind(&RimeManager::RunRimeService, this)));
			}
		}
	}
	catch (std::bad_alloc & ex) {
		_MESSAGE(ex.what());
	}
	catch (boost::system::system_error & err)
	{
		_MESSAGE(err.what());
	}	
}

void RimeManager::StopRimeService()
{
	//FIXME
	if (m_rime_thread && m_rime_thread->joinable()) {
		m_rime_thread->join();
		m_rime_thread.reset(nullptr);
	}
}


void RimeManager::SendUnicodeMessage(UInt32 charCode)
{
	if (IsRimeEnabled()) {
		m_unicode_buf.push_front(charCode);
		UI::SendBSUIScaleformData(RE::InterfaceStrings::GetSingleton()->topMenu, &m_unicode_buf[0]); //UI线程读取消息，不会获取GFxEvent指针所有权。
	}
}

void RimeManager::RunRimeService()
{
	RimeApi* rime = dll_rime_get_api();
	if (!rime) return;
	RIME_STRUCT(RimeTraits, traits);
	auto profile_path = GetProfileDirectory();
	traits.app_name = "rime.skyrimIME";
	traits.shared_data_dir = (profile_path + "SharedData").c_str();

	//char	myDocumentsPath[MAX_PATH];
	//ASSERT(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, myDocumentsPath)));
	std::string userProfileDir = GetUserDataDirectory();
	traits.user_data_dir = userProfileDir.c_str();
	rime->setup(&traits);

	rime->set_notification_handler(&on_message, NULL);

	_MESSAGE("initializing...");
	rime->initialize(NULL);
	Bool full_check = True;
	if (rime->start_maintenance(full_check))
		rime->join_maintenance_thread();
	_MESSAGE("ready.");

	RimeSessionId session_id = rime->create_session();
	if (!session_id) {
		_ERROR("Error creating rime session.");
		return;
	}
	//rime->set_option(session_id, "zh_simp", True);
	m_is_ready = true;

	for (auto message_ptr = m_command_queue.Pop(); message_ptr; message_ptr = m_command_queue.Pop()) { //bad cast exception
		auto * rime_message_ptr = dynamic_cast<AbstractMessage*>(message_ptr.get());
		if (rime_message_ptr) {
			switch (rime_message_ptr->type())
			{
			case MessageType::kMessageType_Char:
			{
				auto * char_message_ptr = dynamic_cast<RimeCharMessage*>(rime_message_ptr);
				if (char_message_ptr) {
					if (rime->process_key(session_id, char_message_ptr->asciiCode, 0)) {
						std::unique_lock<std::shared_mutex> lock(m_indicator_mutex);
						m_indicator.Clear();
						query_rime(session_id, m_indicator);
						auto & commitStr = m_indicator.commit;
						if (commitStr.size()) {
							std::wstring commitWStr = utf8string2wstring(commitStr);
							for (wchar_t unicode : commitWStr) {
								SendUnicodeMessage(unicode);
								//m_unicode_buffer.push_front(unicode);
								//SendBSUIScaleformData(UIStringHolder::GetSingleton()->topMenu, &m_unicode_buffer[0]);
								//CharEvent charEvent(unicode);
								//MenuControls::GetSingleton()->ProcessCommonInputEvent(&charEvent);
							}
						}
						//m_composing_flag = m_indicator.IsComposing();
					}
					else {
						UInt32 charCode = char_message_ptr->asciiCode & 0xFF; //去除部分特殊控制字符的mask.
						if (!IsRimeComposing() || charCode != 0x8) { //Backspace, 当组字时退格键不会删除输入框中已输入内容。
							//CharEvent charEvent(charCode);
							//MenuControls::GetSingleton()->ProcessCommonInputEvent(&charEvent); //未处理消息发送回引擎的charEvent队列中。
							SendUnicodeMessage(charCode);
							//m_unicode_buffer.push_front(charCode);
							//SendBSUIScaleformData(UIStringHolder::GetSingleton()->topMenu, &m_unicode_buffer[0]);
						}
					}
				}
			}
			break;
			case MessageType::kMessageType_Command:
			{
				auto * cmd_message_ptr = dynamic_cast<RimeCommandMessage*>(rime_message_ptr);
				if (cmd_message_ptr) {
					switch (cmd_message_ptr->command)
					{
					case RimeCommandMessage::kCommandType_ClearComposition:
					{
						rime->clear_composition(session_id);
						std::unique_lock<std::shared_mutex> lock(m_indicator_mutex);
						m_indicator.Clear();
						query_rime(session_id, m_indicator);
						//m_composing_flag = m_indicator.IsComposing();
					}
					break;
					case RimeCommandMessage::kCommandType_CommitCompositon:
					{
						//bool result = rime->commit_composition(session_id);
						//std::unique_lock<std::shared_mutex> lock(m_indicator_mutex);
						//m_indicator.Clear();
						//if (result) {
						//	query_rime(session_id, m_indicator);
						//	auto & commitStr = m_indicator.commit;
						//	if (commitStr.size()) {
						//		std::wstring commitWStr = utf8string2wstring(commitStr);
						//		for (wchar_t unicode : commitWStr) {
						//			CharEvent charEvent(unicode);
						//			MenuControls::GetSingleton()->ProcessCommonInputEvent(&charEvent);
						//		}
						//	}
						//}
						//m_composing_flag = m_indicator.IsComposing();		
						std::string commitStr = rime->get_input(session_id);
						rime->clear_composition(session_id);
						std::unique_lock<std::shared_mutex> lock(m_indicator_mutex);
						m_indicator.Clear();
						query_rime(session_id, m_indicator);
						//m_composing_flag = m_indicator.IsComposing();
						if (commitStr.size()) {
							std::wstring commitWStr = utf8string2wstring(commitStr);
							for (wchar_t unicode : commitWStr) {
								/*CharEvent charEvent(unicode);
								MenuControlsEx::GetSingleton()->ProcessCommonInputEvent(&charEvent);*/
								SendUnicodeMessage(unicode);
							}
						}		
					 }
					break;
					case RimeCommandMessage::kCommandType_SwitchAsciiMode:
					{
						RimeApi* rime = dll_rime_get_api();
						RIME_STRUCT(RimeStatus, status);
						if (rime->get_status(session_id, &status)) {
							rime->set_option(session_id, "ascii_mode", !status.is_ascii_mode);
							rime->free_status(&status);
						}
					}
					break;
					case RimeCommandMessage::kCommandType_SwitchFullShape:
					{
						RimeApi* rime = dll_rime_get_api();
						RIME_STRUCT(RimeStatus, status);
						if (rime->get_status(session_id, &status)) {
							rime->set_option(session_id, "full_shape", !status.is_full_shape);
							rime->free_status(&status);
						}
					}
					break;
					case RimeCommandMessage::kCommandType_SwitchSimplification:
					{
						RimeApi* rime = dll_rime_get_api();
						RIME_STRUCT(RimeStatus, status);
						if (rime->get_status(session_id, &status)) {
							if (status.is_simplified) {
								rime->set_option(session_id, "zh_simp", False);
								rime->set_option(session_id, "zh_tw", False);
								rime->set_option(session_id, "zh_trad", True);
								rime->set_option(session_id, "simplification", False);
							}
							else {
								rime->set_option(session_id, "zh_simp", True);
								rime->set_option(session_id, "zh_tw", False);
								rime->set_option(session_id, "zh_trad", False);
								rime->set_option(session_id, "simplification", True);
							}
							rime->free_status(&status);
						}
					}
					break;
					default:
						break;
					}
				}
			}
			break;
			case MessageType::kMessageType_Option:
				break;
			default:
				break;
			}
		}
	}
	rime->destroy_session(session_id);
	rime->finalize();
}


void RimeManager::ClearComposition()
{
	PostCommand<RimeCommandMessage>(RimeCommandMessage::kCommandType_ClearComposition);
}

RimeManager& RimeManager::GetSingleton()
{
	static RimeManager s_inst;
	return s_inst;
}


UInt32 RimeManager::TranslateKeycode(UInt32 keyCode)
{
	switch (keyCode)
	{
	case KeyCode::Escape:
		return ibus::Escape;
	case KeyCode::Backspace:
		return ibus::BackSpace;
	case KeyCode::Tab:
		return ibus::Tab;
	case KeyCode::Enter:
	case KeyCode::KP_Enter:
		return ibus::Return;
	case KeyCode::Up:
		return ibus::Up;
	case KeyCode::Left:
		return ibus::Left;
	case KeyCode::Right:
		return ibus::Right;
	case KeyCode::Down:
		return ibus::Down;
	case KeyCode::PageUp:
		return ibus::Page_Up;
	case KeyCode::PageDown:
		return ibus::Page_Down;
	case KeyCode::F4:
		return ibus::F4;
	default:
		return keyCode;
	}
}

void RimeManager::QueryIndicator(RimeIndicator & indicator)
{
	std::shared_lock<std::shared_mutex> lock(m_indicator_mutex);
	indicator = m_indicator;
}

bool RimeManager::IsRimeEnabled() const
{
	return m_is_ready && RE::ControlMap::GetSingleton()->textEntryCount != 0;
}

bool RimeManager::IsRimeComposing()
{
	std::shared_lock<std::shared_mutex> lock(m_indicator_mutex);
	return m_indicator.IsComposing();
}
