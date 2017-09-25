/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file msg_collector.h
 * @brief
 * @author Lilongpu
 * @date 2017年06月13日
 */
#ifndef MSG_COLLECTOR_I_H_
#define MSG_COLLECTOR_I_H_

#include "libdevcore/Guards.h"
#include "libdevcore/Common.h"
#include "pbft_common.h"

using namespace dev;

class MsgCollectorI
{
    public:
        /**
         * @brief 接受信息
         *
         * @param[in] msg 信息
         */
        virtual void pushMsg(PBFTMsg msg);

        virtual void pushMsgBytes(const bytes& data);

        /**
         * @brief 取出广播信息
         *
         * @return PBFTMsg
         */
        virtual PBFTMsg popMsg();

        virtual bytes popMsgBytes();

        /**
         * @brief 获取对列的大小
         *
         * @return 队列大小
         */
        virtual size_t queueSize();
        /**
         * @brief 获取信息
         *
         * @return
         */
        std::list<PBFTMsg> getMsgQueue();

        /**
         * @brief 删除信息
         */
        void delMsgQueue();
    private:
        mutable dev::Mutex lock_;
        std::list<PBFTMsg>     msg_queue_; //消息队列

};
#endif //MSG_COLLECTOR_I_H_
