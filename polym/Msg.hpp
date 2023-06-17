#ifndef POLYM_MSG_HPP
#define POLYM_MSG_HPP

#include <memory>
#include <utility>

namespace PolyM {

/** Type for Msg unique identifiers */
using MsgUID = unsigned long long;

/**
 * Msg represents a simple message that doesn't have any payload data.
 * Msg ID identifies the type of the message. Msg ID can be queried with getMsgId().
 */
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

class RimeCharMessage : public AbstractMessage
{
public:
    enum {
        kInvalidChar = -1
    };
    RimeCharMessage(UInt32 a_asciiCode) : AbstractMessage(MessageType::kMessageType_Char), asciiCode(a_asciiCode) { }
    virtual ~RimeCharMessage() = default;
    RimeCharMessage(const RimeCharMessage&) = delete;
    RimeCharMessage& operator=(const RimeCharMessage&) = delete;
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
    RimeCommandMessage(const RimeCommandMessage&) = delete;
    RimeCommandMessage& operator=(const RimeCommandMessage&) = delete;
public:
    UInt32			command;
};


class RimeCommandMessage : public RimeMessage {
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
    RimeCommandMessage(UInt32 command_) : RimeMessage(kMessageType_Command), command(command_) { }

    virtual ~RimeCommandMessage() = default;
    RimeCommandMessage(const RimeCommandMessage&) = delete;
    RimeCommandMessage& operator=(const RimeCommandMessage&) = delete;

    virtual std::unique_ptr<Msg> move() override {
        return std::unique_ptr<Msg>(new RimeCommandMessage(std::exchange(command, kCommandType_Invalid)));
    }
private:
    RimeCommandMessage(RimeCommandMessage&&) = default;
    RimeCommandMessage& operator=(RimeCommandMessage&&) = default;
public:
    UInt32				command;
};

class Msg
{
public:
    /**
     * Construct a Msg.
     *
     * @param msgId Msg ID of this Msg.
     */
    Msg(int msgId = 0);

    virtual ~Msg() = default;
    Msg(const Msg&) = delete;
    Msg& operator=(const Msg&) = delete;

    /** "Virtual move constructor" */
    virtual std::unique_ptr<Msg> move();

    /**
     * Get Msg ID.
     * Msg ID identifies message type.
     * Multiple Msg instances can have the same Msg ID.
     */
    int getMsgId() const;

    /**
     * Get Msg UID.
     * Msg UID is the unique ID associated with this message.
     * All Msg instances have a unique Msg UID.
     */
    MsgUID getUniqueId() const;

protected:
    Msg(Msg&&) = default;
    Msg& operator=(Msg&&) = default;

private:
    int msgId_;
    MsgUID uniqueId_;
};

/**
 * DataMsg<PayloadType> is a Msg with payload of type PayloadType.
 * Payload is constructed when DataMsg is created and the DataMsg instance owns the payload data.
 */
template <typename PayloadType>
class DataMsg : public Msg
{
public:
    /**
     * Construct DataMsg
     * @param msgId Msg ID
     * @param args Arguments for PayloadType ctor
     */
    template <typename ... Args>
    DataMsg(int msgId, Args&& ... args)
      : Msg(msgId),
        pl_(new PayloadType(std::forward<Args>(args) ...))
    {
    }

    virtual ~DataMsg() = default;
    DataMsg(const DataMsg&) = delete;
    DataMsg& operator=(const DataMsg&) = delete;

    /** "Virtual move constructor" */
    virtual std::unique_ptr<Msg> move() override
    {
        return std::unique_ptr<Msg>(new DataMsg<PayloadType>(std::move(*this)));
    }

    /** Get the payload data */
    PayloadType& getPayload() const
    {
        return *pl_;
    }

protected:
    DataMsg(DataMsg&&) = default;
    DataMsg& operator=(DataMsg&&) = default;

private:
    std::unique_ptr<PayloadType> pl_;
};

}

#endif
