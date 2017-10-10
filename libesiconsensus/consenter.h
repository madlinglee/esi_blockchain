/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file consenter.h
 * @brief 共识节点
 * @author SongYuanjie
 * @date 2017年06月26日
 */
#pragma once

#include <iostream>
#include <libesipbftseal/pbft_client.h>
#include <libesipbft/utils.h>
#include <libesipbft/validator_set.h>
#include <libesipbft/msg_collector.h>
#include <libesipbft/pbft_state_machine.h>
#include "common.h"

using namespace pbft;

/**
 * @brief 定义共识者
 */
class Consenter: public MsgCollectorI, public Worker
{
public:
    Consenter(const std::string& consenter_name, const eth::ChainParams& params, p2p::Host* host, const boost::filesystem::path& db_path, WithExisting we = WithExisting::Trust) : Worker("con"),
    client_name_(consenter_name)
    {
        client_ptr_.reset(new PBFTClient(params, (int)params.networkID, host, std::shared_ptr<GasPricer>(), db_path, we));
        //产生自身的公钥
        pbft_instance_.resetKeyPairFromSeed(client_name_);
    }
    ~Consenter()
    {
        client_ptr_.reset();
    }

    /**
     * @brief 插入验证者
     *
     * @param client_name_ 节点名称
     */
    void insertValidator(const std::string& name)
    {
        Validator validator = getValidator(name);
        pbft_instance_.insertValidator(validator);
    }

    /**
     * @brief 开启pbft线程
     */
    void startPBFT()
    {
        
        clog(PBFTNote) << "Starting PBFT consensus";
        //注册广播消息队列
        pbft_instance_.registerMsgCollector(this);
    //注册数据验证和提交接口
    pbft_instance_.injectPBFTI(client_ptr_.get());
        //启动pbft线程
        pbft_instance_.start();
        Worker::startWorking();
    }

    /**
     * @brief 重写Worker的工作线程
     */
    virtual void doWork()
    {
        excuteConsensus();
    }

    void stopPBFT()
    {
        clog(PBFTNote) << "Stopping PBFT consensus";
        pbft_instance_.stop();
        Worker::stopWorking();
    }
    Client* client() const{return static_cast<Client*>(client_ptr_.get());}
private:
    /**
     * @brief PBFT消息的注入P2Pnode
     */
    void injectP2P()
    {
         //PBFTMsg
        std::list<PBFTMsg> pbftMsgs = this->getMsgQueue();
     
        while (!pbftMsgs.empty())
        {
            PBFTMsg message = pbftMsgs.front();
            pbftMsgs.pop_front();
            //构造RLPstream
            RLPStream rlpStream;
            rlpStream.appendList(2);
            rlpStream.append(long2String(message.msg_type_));
            rlpStream.append(message.msg_content_);
            client_ptr_->broadcastPBFTMsgs(rlpStream.out());
        }

    }

    /**
     * @brief P2Pnode消息注入pbft状态机
     */
    void injectPBFT()
    {
        //P2Pnode消息
        std::vector<bytes> P2PMsgs = client_ptr_->getRecvMsgs();
        for (size_t i = 0; i < P2PMsgs.size(); i++)
        {
            PBFTMsg message;
            RLP rlp(P2PMsgs[i]);
            message.msg_type_ = (PBFT_MSG_TYPE)atol(rlp[0].toString().c_str());
            message.msg_content_ = rlp[1].toBytes();
            pbft_instance_.pushMsg(message);
        }
        return;
    }
    //开始共识
    /**
     * @brief 执行共识
     */
    void excuteConsensus()
    {
            injectP2P();
            injectPBFT();
    }
    //P2P网络
    std::unique_ptr<PBFTClient> client_ptr_;
    std::string client_name_;
    //pbft
    PBFTStateMachine pbft_instance_;
};
