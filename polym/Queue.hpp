#ifndef POLYM_QUEUE_HPP
#define POLYM_QUEUE_HPP

#include <memory>

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
    AbstractMessage(const AbstractMessage&) = delete;
    AbstractMessage& operator=(const AbstractMessage&) = delete;
    MessageType type() const { return messageType_; }
    operator MessageType() { return messageType_; }
private:
    MessageType     messageType_ = MessageType::kMessageType_Invalid;
};

typedef std::unique_ptr<AbstractMessage> ContainerEntryType;
class CommonMessageQueue;

#endif
