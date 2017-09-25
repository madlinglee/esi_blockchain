/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file pbft_state_machine.h
 * @brief
 * @author Lilongpu
 * @date 2017年06月14日
 */
#ifndef PBFT_STATE_MACHINE_H_
#define PBFT_STATE_MACHINE_H_
#include "time_out_ticker.h"
#include "pbft_common.h"
#include "validator_set.h"
#include "proposal_info.h"
#include "utils.h"
#include "key_wrapped.h"
#include "pbft_interface.h"
#include "vote_set.h"
#include "lock_info.h"
using namespace dev;
/**
 * @brief PBFT的状态机，根据接收到的消息进行状态变换，并能够产生消息
 */
class PBFTStateMachine : public MsgCollectorI , public Worker
{
public:
    /**
     * @brief 构造函数
     */
    PBFTStateMachine() : Worker("pbft")
    {
        injectPBFTI(new PBFTInterface());
        round_state_.update(0,0,UNVALID_STATE);
        time_out_ticker_.registerMsgCollector(this);
    }
    /**
     * @brief 注入依赖：外部的产生数据和提交数据的接口
     *
     * @param pbft_interface: 接口的实例
     */
    void injectPBFTI(PBFTInterface* pbft_interface);
    /**
     * @brief 开始工作，开始工作前必须调用：
     *         1. injectPBFTI；产生数据和提交数据的接口
     *         2. insertValidator；初始化验证者集合
     *         3. resetKeyPairFromSeed；初始化当前节点的密钥对
     *         4. registerMsgCollector；当前节点产生的消息输出到哪
     */
    void start();

    /**
     * @brief 停止工作线程
     */
    void stop()
    {
        time_out_ticker_.stop();
        stopWorking();
    }

    /**
     * @brief 注册状态机的消息收集器，状态机产生的消息都将被其收集
     *
     * @param msg_collector: 收集状态机的输出消息，这些消息应广播给其它验证者
     */
    void registerMsgCollector(MsgCollectorI* msg_collector);
    /**
     * @brief 插入一个验证者
     *
     * @param validator: 验证者对象
     */
    void insertValidator(const Validator& validator);
    /**
     * @brief 从一个种子初始化当前节点的密钥对
     *
     * @param seed: 种子信息
     */
    void resetKeyPairFromSeed(const std::string& seed);
    /**
     * @brief 从一个私有初始化当前节点的密钥对
     *
     * @param sec: 私钥信息
     */
    void resetKeyPairFromSec(const SecKey& sec);
    /**
     * @brief 获取当前状态机的状态
     *
     * @return 当前状态机的状态H, R, S
     */
    RoundState getRoundState();
    /**
     * @brief 当前节点是否是提案者
     *
     * @return true:是；false:不是
     */
    bool isProposer();
    /**
     * @brief 当前节点是否是验证者
     *
     * @return true:是；false:不是
     */
    bool isValidator()
    {
        return(-1 != validator_set_.isExistValidator(key_pair_.pub()));
    }
private:
    /**
     * @brief 初始化动作
     */
    void init()
    {
        long h = pbft_interface_->getHeight();
        enterNewHeight(h+1, 0);
    }
    /**
     * @brief 重写工作线程的doWork方法
     */
    virtual void doWork(); //重写
    /**
     * @brief 消息处理方法，被不断轮询调用
     *
     * @param msg: 待处理的消息
     */
    void handleMsg(PBFTMsg msg);
    /**
     * @brief 处理投票消息
     *
     * @param msg: 待处理的消息
     */
    void processVoteMsg(const bytes& msg);
    /**
     * @brief 处理提案消息
     *
     * @param msg: 待处理的消息
     */
    void processProposalMsg(const bytes& msg); //目前只接收一个提案，需要接收多个吗
    /**
     * @brief 处理超时消息
     *
     * @param msg: 待处理的消息
     */
    void processTimeOutMsg(const bytes msg);
    /**
     * @brief 更新验证者集合
     */
    void updateVoteSetValidator();     //更新验证者集合，目前没有具体动作
    /**
     * @brief 验证提案信息
     *
     * @return true 成功，false 失败
     */
    bool verifyProposalInfo();    //验证提案信息
    /**
     * @brief 产生超时信息
     *
     * @paramstep
     */
    void timeOut(PBFT_STATE step); //产生超时信息
    /**
     * @brief 进入NewHeight阶段
     *
     * @param[in] h 高度
     * @param[in] r 轮次
     */
    void enterNewHeight(long h, long r); //进入NewHeight阶段
    /**
     * @brief 进入新的轮次
     *
     * @param[in] h 高度
     * @param[in] r 轮次
     */
    void enterNewRound(long h, long r);
    /**
     * @brief 进入提案
     *
     * @param[in] h 高度
     * @param[in] r 轮次
     */
    void enterPropose(long h, long r);
    /**
     * @brief 进入预投票阶段
     *
     * @param[in] h 高度
     * @param[in] r 轮次
     */
    void enterPrevote(long h, long r);
    /**
     * @brief 设置投票等待时间
     *
     * @param[in] h 当前高度
     * @param[in] r 当前轮次
     */
    void enterPrevotewait(long h, long r);
    /**
     * @brief 进入预提交阶段
     *
     * @param[in] h 当前高度
     * @param[in] r 当前的轮次
     */
    void enterPrecommit(long h, long r);
    /**
     * @brief 进入预提交的等待阶段
     *
     * @param[in] h 当前高度
     * @param[in] r 当前轮次
     */
    void enterPrecommitwait(long h, long r);
    /**
     * @brief 进入投票阶段
     *
     * @param[in] h 当前高度
     * @param[in] r 当前轮次
     */
    void doPrevote(long h, long r); //投票
    /**
     * @brief 创建投票信息
     *
     * @param msg 信息
     * @param type 类型
     *
     * @return
     */
    bytes createVoteMsg(const bytes& msg, PBFT_STATE type); //创建投票信息
    /**
     * @brief 创建投票信息
     *
     * @param[in] msg 消息
     * @param[in] type 消息类型
     *
     * @return
     */
    bytes createVoteMsg(const std::string& msg, PBFT_STATE type); //创建投票信息
    /**
     * @brief 创建提案信息
     */
    void createProposalInfo(); //创建提案信息
    /**
     * @brief 发送提案信息
     */
    void sendProposal(); //将提案消息，扔到广播队列中
    /**
     * @brief 发送信息
     *
     * @param[in] state 状态
     * @param[in] msg 信息
     */
    void sendMsg(PBFT_MSG_TYPE state, const bytes& msg);
    /**
     * @brief 尝试提交
     *
     * @param[in] h 高度
     * @param[in] r 轮次
     */
    void tryCommit(long h, long r); //尝试提交
    /**
     * @brief 最终提交
     *
     * @param[in] data 数据
     */
    void finallyCommit(const bytes& data); //最终提交
    /**
     * @brief 判断当前precommit类型的投票
     */
    void judgePrecommitSet();
    /**
     * @brief 判断当前prevote类型的投票
     */
    void judgePrevoteSet();
    /**
     * @brief 判断
     *
     * @paramvote_type
     */
    void judgeVoteSet(PBFT_STATE vote_type);
private:
    MsgCollectorI*  state_msg_collector_;//状态机产生的，需要广播出去的消息
    PBFTInterface*  pbft_interface_;    //PBFT状态机接口
    RoundState         round_state_;          //状态
    TimeOutTicker     time_out_ticker_;     //超时事件产生器
    ValidatorSet    validator_set_;     //验证者集合
    Validator         proposal_validator_;//提案者
    ProposalInfo    proposal_info_;      //提案信息
    KeyWrapped      key_pair_;             //我的密钥
    VoteSet         vote_set_;          //收集的投票信息
    LockInfo        lock_info_;         //锁定信息
};
#endif //PBFT_STATE_MACHINE_H_
