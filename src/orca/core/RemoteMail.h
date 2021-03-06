#ifndef  ORCA_CORE_REMOTE_MAIL_H
#define  ORCA_CORE_REMOTE_MAIL_H

#include <memory>
#include "Address.h"
#include "MessagePack.h"

namespace orca
{ 
namespace core
{

struct RemoteActorName
{
    uint32_t frameworkId;
    std::string actorName;
};

template<typename MessageType>
class RemoteMail
{
public:
    enum IndexMode
    {
        ByAddress = 1,
        ByName
    };
    RemoteMail();
    RemoteMail(struct Address& from, struct Address& to, std::shared_ptr<MessageType> message);
    RemoteMail(struct Address& from, uint32_t framework ,std::string& name, std::shared_ptr<MessageType> message);
    uint32_t getDestinationId();
    int unpack(const char* data, int size);
    int pack(char* data, int size);
    int size();
    int getIndexMode();

    struct Address& getFromAddress();
    struct Address& getDestinationAddress();
    std::string& getDestinationActor();
    std::shared_ptr<MessageType> getMessage();

private:
    IndexMode indexMode_;
    struct Address from_;
    struct Address to_;
    RemoteActorName remoteActor_;
    std::shared_ptr<MessageType> message_;

    int packAddress(struct Address& addr,char* data);
    int unPackAddress(struct Address& addr, const char* data,int size);
    int packString(std::string& name, char* data);
    int unPackString(std::string& name, const char* data,int size);

    int extendSize();
};


template<typename MessageType>
inline RemoteMail<MessageType>::RemoteMail()
{
}

template<typename MessageType>
inline RemoteMail<MessageType>::RemoteMail(struct Address& from, struct Address& to, std::shared_ptr<MessageType> message)
    : indexMode_(ByAddress),
    from_(from),
    to_(to),
    remoteActor_({ 0,"" }),
    message_(message)
{
}

template<typename MessageType>
inline RemoteMail<MessageType>::RemoteMail(struct Address& from, uint32_t framework, std::string& name, std::shared_ptr<MessageType> message)
    : indexMode_(ByName),
    from_(from),
    remoteActor_({framework,name}),
    message_(message)
{
}

template<typename MessageType>
inline uint32_t RemoteMail<MessageType>::getDestinationId()
{
    uint32_t id = (indexMode_ == ByAddress) ? to_.framework : remoteActor_.frameworkId;
    return id;
}

template<typename MessageType>
inline int RemoteMail<MessageType>::unpack(const char* data, int size)
{
    auto index = 0;
    indexMode_ = static_cast<IndexMode>(data[index++]);

    index += unPackAddress(from_, &data[index], (size - index));
    if (indexMode_ == ByAddress)
    {
        index += unPackAddress(to_, &data[index], (size - index));
    }
    else
    {
        index += unPackString(remoteActor_.actorName, &data[index], (size - index));
    }
    message_ = std::make_shared<MessageType>(const_cast<char*>(&data[index]), size - index);
    return 0;
}

template<typename MessageType>
inline int RemoteMail<MessageType>::pack(char* data, int size)
{
    if (size < this->size())
    {
        return -1;
    }
    int index = 0;
    data[index++] = indexMode_;
    index += packAddress(from_,&data[index]);
    if (indexMode_ == ByAddress)
    {
        index += packAddress(to_, &data[index]);
    }
    else
    {
        index += packString(remoteActor_.actorName, &data[index]);
    }
    
    if (message_)
    {
        MessagePack<MessageType> messagepack(message_);
        int dataSize = messagepack.size();
        const char* ptr = (const char*)(messagepack.enter());
        std::copy(ptr, ptr + dataSize, data+index);
    }
    return 0;
}

template<typename MessageType>
inline int RemoteMail<MessageType>::size()
{
    return extendSize() + (int)message_->size();
}

template<typename MessageType>
inline int RemoteMail<MessageType>::getIndexMode()
{
    return indexMode_;
}

template<typename MessageType>
inline Address& RemoteMail<MessageType>::getFromAddress()
{
    return from_;
}

template<typename MessageType>
inline Address& RemoteMail<MessageType>::getDestinationAddress()
{
    return to_;
}

template<typename MessageType>
inline std::string& RemoteMail<MessageType>::getDestinationActor()
{
    return remoteActor_.actorName;
}

template<typename MessageType>
inline std::shared_ptr<MessageType> RemoteMail<MessageType>::getMessage()
{
    return message_;
}


template<typename MessageType>
inline int RemoteMail<MessageType>::packAddress(struct Address& addr, char* data)
{
    char* p = (char*)(&from_);
    int addrsize = sizeof(from_);
    std::copy(p, p + addrsize, data);
    return addrsize;
}

template<typename MessageType>
inline int RemoteMail<MessageType>::unPackAddress(struct Address& addr, const char * data, int size)
{
    int dataSize = sizeof(addr);
    if (dataSize <= size)
    {
        char* p = (char*)(&addr);
        std::copy(data, data + dataSize, p);
        return dataSize;
    }
    return -1;
}

template<typename MessageType>
inline int RemoteMail<MessageType>::packString(std::string& name, char* data)
{
    int strSize = (int)remoteActor_.actorName.size();
    if (strSize > 255)
    {
        base::ErrorHandle::Instance()->error(base::ErrorInfo::ActorNameTooLong, std::string("actor's name more than 512 bytes:") + remoteActor_.actorName);
        return -1;
    }
    data[0] = strSize;
    const char* p = remoteActor_.actorName.c_str();
    std::copy(p, p + strSize, &data[1]);
    return strSize+1;
}

template<typename MessageType>
inline int RemoteMail<MessageType>::unPackString(std::string& name, const char* data, int size)
{
    name.clear();
    uint8_t strSize = data[0];
    if (strSize + 1 < size)
    {
        data++;
        std::copy(data, data + strSize, back_inserter(name));
        return strSize+1;
    }
    return -1;
}

template<typename MessageType>
inline int RemoteMail<MessageType>::extendSize()
{
    size_t size = 0;
    if (indexMode_ == ByName)
    {
        size = sizeof(char) +sizeof(from_) +sizeof(char) + remoteActor_.actorName.size() ;
    }
    else
    {
        size = sizeof(char) + sizeof(from_) + sizeof(to_) ;
    }
    return (int)size;
}

}
}
#endif // ! ORCA_CORE_REMOTE_MESSAGE_H

