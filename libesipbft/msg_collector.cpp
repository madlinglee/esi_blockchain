/**
 * @file msg_collector.cpp
 * @brief
 * @author Lilongpu
 * @date 2017年06月13日
 */

#include <libdevcore/RLP.h>
#include "msg_collector.h"
#include "utils.h"

void MsgCollectorI::pushMsg(PBFTMsg msg)
{
    DEV_GUARDED(lock_)
    {
        msg_queue_.push_back(msg);
    }
}
void MsgCollectorI::pushMsgBytes(const bytes& data)
{
    PBFTMsg message;
    RLP rlp(data);
    message.msg_type_ = (PBFT_MSG_TYPE)atol(rlp[0].toString().c_str());
    message.msg_content_ = rlp[1].toBytes();
    pushMsg(message);
}
PBFTMsg MsgCollectorI::popMsg()
{
    PBFTMsg msg;
    DEV_GUARDED(lock_)
    {
        if(msg_queue_.size() != 0)
        {
            msg = msg_queue_.front();
            msg_queue_.pop_front();
        }
    }
    return msg;
}
bytes MsgCollectorI::popMsgBytes()
{
    PBFTMsg message = popMsg();
    RLPStream s;
    s.appendList(2);
    s.append(long2String(message.msg_type_));
    s.append(message.msg_content_);
    return s.out();
}
size_t MsgCollectorI::queueSize()
{
    size_t size = 0;
    DEV_GUARDED(lock_)
        size = msg_queue_.size();
    return size;
}
std::list<PBFTMsg> MsgCollectorI::getMsgQueue()
{
    std::list<PBFTMsg> retMsgs;
    DEV_GUARDED(lock_)
    {
        retMsgs = msg_queue_;
        delMsgQueue();
    }
    return retMsgs;
}
void MsgCollectorI::delMsgQueue()
{
    if (msg_queue_.size())
        msg_queue_.clear();
    return;
}
