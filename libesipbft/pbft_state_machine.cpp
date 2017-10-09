/**
 * @file pbft_state_machine.cpp
 * @brief
 * @author Lilongpu
 * @date 2017年06月19日
 */
#include <iostream>
#include "pbft_state_machine.h"
#include "libdevcore/RLP.h"
#include "libdevcrypto/Hash.h"

using namespace dev;
using namespace std;

void PBFTStateMachine::injectPBFTI(PBFTInterface* pbft_interface)
{
    pbft_interface_ = pbft_interface;
}
void PBFTStateMachine::start()
{
    init();
    Worker::startWorking(); //启动消息处理线程
    time_out_ticker_.start(); //启动超时事件产生器
}
void PBFTStateMachine::registerMsgCollector(MsgCollectorI* msg_collector)
{
    state_msg_collector_ = msg_collector;
}
void PBFTStateMachine::insertValidator(const Validator& validator)
{
    validator_set_.insertValidator(validator);
}
void PBFTStateMachine::resetKeyPairFromSeed(const std::string& seed)
{
    key_pair_.createKeyFromSeed(seed);
}
void PBFTStateMachine::resetKeyPairFromSec(const SecKey& sec)
{
    key_pair_.createKeyFromSec(sec);
}
void PBFTStateMachine::doWork() //重写
{
    if(MsgCollectorI::queueSize() > 0)
        handleMsg(MsgCollectorI::popMsg());
}
void PBFTStateMachine::handleMsg(PBFTMsg msg)
{
    //TODO 消息处理
    try{
        switch(msg.msg_type_)
        {
            case PROPOSAL:
                processProposalMsg(msg.msg_content_);
                break;
            case VOTE:
                processVoteMsg(msg.msg_content_);
                break;
            case TIME_OUT:
                processTimeOutMsg(msg.msg_content_);
                break;
            default:
                break;
        }
    }catch(...)
    {
        clog(PBFTError) << "Handle" << msg.msg_type_ << "msg error";
    }
    return;
}
void PBFTStateMachine::processProposalMsg(const bytes& msg)
{
    //long h, r;
    RLP rlp(msg);
    try
    {
        proposal_info_.proposal_hash = rlp[0].toBytes();
        proposal_info_.proposal_data = rlp[1].toBytes();
        proposal_info_.sign = rlp[2].toBytes();
        //h = atol(rlp[1][0].toString().c_str());
        //r = atol(rlp[1][1].toString().c_str());
    }
    catch(...)
    {
        proposal_info_.clear();
        clog(PBFTError) << "Recv unvalid proposal msg: rlp error";
        return;
    }
    if(isProposer() || verifyProposalInfo())
    {
        proposal_info_.is_valid = true;
        enterPrevote(round_state_.height, round_state_.round);
    }
        
}
bool PBFTStateMachine::verifyProposalInfo()
{
    bytes data;
    long h = -1;
    long r = -1;
    try
    {
        RLP rlp(proposal_info_.proposal_data);
        h = atol(rlp[0].toString().c_str());
        r = atol(rlp[1].toString().c_str());
        data = rlp[2].toBytes();
        if(h != round_state_.height || r != round_state_.round) //验证数据的高度和轮次
        {
            proposal_info_.clear();
            clog(PBFTWarn) << "Recv unvalid proposal msg: h r" << "(" << h << "," << r << ") wrong";
            return false;
        }
    }
    catch(...)
    {
        proposal_info_.clear();
        clog(PBFTError) << "Recv unvalid proposal msg: rlp error";
        return false;
    }

    if(!pbft_interface_->verify(data)) //协商的共识数据没有通过外部接口的确认
    {
        proposal_info_.clear();
        clog(PBFTWarn) << "Recv unvalid proposal msg: data failed" << "(" << h << "," << r << ")";
        return false;
    }

    h256 hash = sha256(&(proposal_info_.proposal_data));//哈希
    if(!(hash.asBytes() == proposal_info_.proposal_hash)) //验哈希
    {
        proposal_info_.clear();
        clog(PBFTWarn) << "Recv unvalid proposal msg: hash failed" << "(" << h << "," << r << ")";
        return false;
    }
    if(!dev::verify(proposal_validator_.pub_key, dev::Signature(proposal_info_.sign), hash)) //验签
    {
        proposal_info_.clear();
        clog(PBFTWarn) << "Recv unvalid proposal msg: sign failed" << "(" << h << "," << r << ")";
        return false;
    }
    clog(PBFTTrace) << "Recv a valid proposal" << "(" << h << "," << r << ")";
    proposal_info_.is_valid = true;
    return true;
}
void PBFTStateMachine::processVoteMsg(const bytes& msg)
{
    long height, round, validator_index = 0;
    PBFT_STATE vote_type;
    bytes hash, signature;
    try
    {
        RLP rlp(msg);
        height                 = atol(rlp[0].toString().c_str());
        round                  = atol(rlp[1].toString().c_str());
        vote_type             = long2PBFT_STATE(atol(rlp[2].toString().c_str()));  //投票类型
        validator_index      = atol(rlp[3].toString().c_str());//验证者的索引
        hash                 = rlp[4].toBytes();               //哈希
        signature             = rlp[5].toBytes();               //签名
    }
    catch(...)
    {
        clog(PBFTError) << "Recv unvalid vote msg: rlp error";
        return;
    }

    if(height != round_state_.height)
    {
        clog(PBFTWarn) << "Recv unvalid vote msg: current height:" << round_state_.height << ", vote height:" << height;
        return;
    }
    if(validator_index >= validator_set_.size())
    {
        clog(PBFTWarn) << "Recv unvalid vote msg: validator index:" << validator_index << ", validator set size:" << validator_set_.size();
        return;
    }
    if(dev::verify(validator_set_.getValidator(validator_index).pub_key, dev::Signature(signature), h256(bytesConstRef(&hash)) )) //验签
    {
        clog(PBFTTrace) << "Recv" << step2str(vote_type) << "vote msg(" << height << ","<< round << "), current(" << round_state_.height << "," << round_state_.round << "), validator[" << validator_index << "]";
        vote_set_.addVote(height, round, vote_type,
                validator_set_.getValidator(validator_index).pub_key, hash, signature);
        judgeVoteSet(vote_type);
        return;
    }
    else
    {
        clog(PBFTWarn) << "Recv sign failed" << step2str(vote_type) << "vote msg(" << height << ","<< round << ")";
        return;
    }
}
void PBFTStateMachine::judgePrevoteSet()
{
    PBFT_STATE vote_type = PREVOTE_STEP;
    if(vote_set_.hasTwoThirdsMajority(vote_type))
    {
        MAJ32 maj32 = vote_set_.getTwoThirdsMajority(vote_type);
        if(maj32.round < round_state_.round) //以前的轮次，只更新锁定信息
        {
            clog(PBFTTrace) << "Recv later prevote consensus, major32 round:" << maj32.round << ", current round:" << round_state_.round;
            vote_set_.clearPrevoteMaj32(); //历史轮次的一致信息，立马清空
            //TODO update lock info
        }
        else if(maj32.round >= round_state_.round && round_state_.step < PREVOTE_STEP) //还没投过票
        {
            enterPrevote(round_state_.height, maj32.round);    //进入新的轮次，并投票
            enterPrevotewait(round_state_.height, maj32.round);
            enterPrecommit(round_state_.height, maj32.round);
        }
        else if(maj32.round >= round_state_.round && round_state_.step < PRECOMMIT_STEP) //已经投过票了
        {
            // 已收集了三分之二多数的一致，不用经过entrePrevotewait了，直接enterPrecommit
            enterPrecommit(round_state_.height, maj32.round);
        }
    }
    else if(vote_set_.hasTwoThirdsAny(vote_type))
    {
        ANY32 any32 = vote_set_.getTwoThirdsAny(vote_type);
        if(any32.round < round_state_.round)
        {
            //do nothing
            clog(PBFTTrace) << "Recv prevote any32, any32 round:" << any32.round << ", current round:" << round_state_.round;
        }
        else if(any32.round >= round_state_.round && round_state_.step <= PREVOTEWAIT_STEP)
        {
            enterPrevotewait(round_state_.height, round_state_.round);
        }
    }
    else
    {
    }
}
void PBFTStateMachine::tryCommit(long h, long r)
{
    clog(PBFTTrace) << "Try commit";
    if(!vote_set_.hasTwoThirdsMajority(PRECOMMIT_STEP))
    {
        clog(PBFTWarn) << "No maj32...";
        return;
    }
    MAJ32 maj32 = vote_set_.getTwoThirdsMajority(PRECOMMIT_STEP);
    if(maj32.voted_proposal_hash == createVoteMsgHash(h, r, PRECOMMIT_STEP, proposal_info_.proposal_hash).asBytes())
    {
        finallyCommit(proposal_info_.proposal_data);
    }
    else if(maj32.voted_proposal_hash == createVoteMsgHash(h, r, PRECOMMIT_STEP, lock_info_.lock_data.proposal_hash).asBytes())
    {
        finallyCommit(lock_info_.lock_data.proposal_hash);
    }
    else
    {
        clog(PBFTTrace) << "Enter next round for null consensus data";
        enterNewRound(h, r+1);
    }
}
void PBFTStateMachine::finallyCommit(const bytes& data)
{
    clog(PBFTTrace) << "Finally commit";
    try
    {
        if(pbft_interface_->commit(RLP(data)[2].toBytes()))
        {
            enterNewHeight(round_state_.height + 1, 0);
        }else
        {
            enterNewRound(round_state_.height, round_state_.round + 1); //如果此处发生异常，那么会在catch中再进入一次enterNewRound
        }
    }catch(...)
    {
        clog(PBFTError) << "Commit unvalid data: rlp error";
        enterNewRound(round_state_.height, round_state_.round + 1);
    }
}
void PBFTStateMachine::judgePrecommitSet()
{
    PBFT_STATE vote_type = PRECOMMIT_STEP;
    if(vote_set_.hasTwoThirdsMajority(vote_type))
    {
        MAJ32 maj32 = vote_set_.getTwoThirdsMajority(vote_type);
        if(maj32.round < round_state_.round)
        {
            //do nothing
            clog(PBFTTrace) << "Recv precommit maj32 for history round";
            vote_set_.clearPrecommitMaj32();
        }
        else if(maj32.round >= round_state_.round)
        {
            tryCommit(round_state_.height, maj32.round); //不再投票了，立刻尝试提交
        }
    }
    else if(vote_set_.hasTwoThirdsAny(vote_type))
    {
        ANY32 any32 = vote_set_.getTwoThirdsAny(vote_type);
        if(any32.round < round_state_.round)
        {
            //do nothing
            clog(PBFTTrace) << "Recv precommit any32, any32 round:" << any32.round << ", current round:" << round_state_.round;
        }
        else if(any32.round >= round_state_.round && round_state_.step <= PRECOMMITWAIT_STEP)
        {
            //enterPrecommit(round_state_.height, round_state_.round); //我要投票
            enterPrecommitwait(round_state_.height, round_state_.round);
        }

    }
    else
    {
        //do nothing
    }
}
void PBFTStateMachine::enterPrecommitwait(long h, long r)
{
    if(h != round_state_.height || r < round_state_.round || (r == round_state_.round && PRECOMMITWAIT_STEP <= round_state_.step))
    {
        clog(PBFTWarn) << "Enter invalid PrecommitWait step: args(" << h << "," << r << "), current(" << round_state_.height << "," << round_state_.round << ")";
        return;
    }
    clog(PBFTTrace) << "Enter PrecommitWait step";
    round_state_.update(r, PRECOMMITWAIT_STEP);
    timeOut(PRECOMMITWAIT_STEP);
}
void PBFTStateMachine::judgeVoteSet(PBFT_STATE vote_type) //一定跟在addVote动作之后
{
    if(vote_type == PREVOTE_STEP)
    {
        judgePrevoteSet();
    }
    else if(vote_type == PRECOMMIT_STEP)
    {
        judgePrecommitSet();
    }
}
void PBFTStateMachine::enterPrevotewait(long h, long r)
{
    if(h != round_state_.height || r < round_state_.round || (r == round_state_.round && PREVOTEWAIT_STEP <= round_state_.step))
    {
        clog(PBFTWarn) << "Enter invalid PrevoteWait step: args(" << h << "," << r << "), current(" << round_state_.height << "," << round_state_.round << ")";
        return;
    }
    clog(PBFTTrace) << "Enter PrevoteWait step";
    round_state_.update(r, PREVOTEWAIT_STEP);
    timeOut(PREVOTEWAIT_STEP);

}
void PBFTStateMachine::enterPrecommit(long h, long r)
{
    if(h != round_state_.height || r < round_state_.round || (r == round_state_.round && PRECOMMIT_STEP <= round_state_.step))
    {
        clog(PBFTWarn) << "Enter invalid Precommit step: args(" << h << "," << r << "), current(" << round_state_.height << "," << round_state_.round << ")";
        return;
    }
    clog(PBFTTrace) << "Enter Precommit step";
    time_out_ticker_.pause();
    round_state_.update(r, PRECOMMIT_STEP);
    if(vote_set_.hasTwoThirdsMajority(PREVOTE_STEP)) //达成了一致
    {
        MAJ32 maj32 = vote_set_.getTwoThirdsMajority(PREVOTE_STEP);
        h256 prevote_hash = createVoteMsgHash(round_state_.height, round_state_.round, PREVOTE_STEP, proposal_info_.proposal_hash);
        h256 lock_prevote_hash = createVoteMsgHash(round_state_.height, round_state_.round, PREVOTE_STEP, lock_info_.lock_data.proposal_hash);
        if(maj32.voted_proposal_hash == prevote_hash.asBytes()) //对提案信息达成了一致
        {
            clog(PBFTTrace) << "Do precommit for proposal";
            lock_info_.update(h, r, proposal_info_); //更新锁定信息
            sendMsg(VOTE, createVoteMsg(proposal_info_.proposal_hash, PRECOMMIT_STEP));

        }
        else if(maj32.voted_proposal_hash == lock_prevote_hash.asBytes()) //对锁定信息达成了一致
        {
            clog(PBFTTrace) << "Do precommit for lock";
            sendMsg(VOTE, createVoteMsg(lock_info_.lock_data.proposal_hash, PRECOMMIT_STEP));
        }
        else //其它情况，投空票
        {
            clog(PBFTTrace) << "Do precommit for null";
            lock_info_.clear(); //解锁
            sendMsg(VOTE, createVoteMsg("NULL", PRECOMMIT_STEP));
        }
    }
    else if(vote_set_.hasTwoThirdsAny(PREVOTE_STEP)) //超时
    {
            clog(PBFTTrace) << "Do precommit for null/timeout";
            sendMsg(VOTE, createVoteMsg("NULL", PRECOMMIT_STEP));
    }
}
void PBFTStateMachine::processTimeOutMsg(const bytes msg)
{
    long height, round, round_step = 0;
    RLP rlp(msg);
    try
    {
        height = atol(rlp[0].toString().c_str());
        round  = atol(rlp[1].toString().c_str());
        round_step  = atol(rlp[2].toString().c_str());
    }
    catch(...)
    {
        clog(PBFTError) << "Recv unvalid timeout msg: rlp error";
        return;
    }

    if(height != round_state_.height || round != round_state_.round || (round == round_state_.round && round_step < round_state_.step))
    {
        clog(PBFTWarn) << "Recv unvalid timeout msg";
        return;
    }
    clog(PBFTTrace) << "Recv timeout msg(" << height << "," <<round << "," << step2str(round_step) << ")";
    switch(round_step)
    {
        case NEW_HEIGHT_STEP:
            enterNewRound(round_state_.height, round_state_.round);
            break;
        case PROPOSAL_STEP:
            enterPrevote(round_state_.height, round_state_.round);
            break;
        case PREVOTEWAIT_STEP:
            enterPrecommit(round_state_.height, round_state_.round);
            break;
        case PRECOMMITWAIT_STEP:
            enterNewRound(round_state_.height, round_state_.round + 1);
            break;
        default:
            clog(PBFTError) << "Recv unvalid type timeout msg";
    }
}
void PBFTStateMachine::timeOut(PBFT_STATE step)
{
    int time_out_ms = 0;
    switch(step)
    {
        //TODO 硬编码
        case NEW_HEIGHT_STEP:
            time_out_ms = 5;
            break;
        case PROPOSAL_STEP:
            time_out_ms = 30000;
            break;
        case PREVOTEWAIT_STEP:
            time_out_ms = 50;
            break;
        case PRECOMMITWAIT_STEP:
            time_out_ms = 50;
            break;
        default:
            clog(PBFTError) << "Timeout unknown type" << step;
            break;
    }
    time_out_ticker_.pause();
    time_out_ticker_.setTimeInfo(round_state_.height, round_state_.round, step, time_out_ms);
}
void PBFTStateMachine::enterNewHeight(long h, long r)
{
    round_state_.height = h;
    round_state_.update(r, NEW_HEIGHT_STEP);

    proposal_info_.clear(); //开始协商新区块，旧的提案信息可以清空了
    vote_set_.clear(); //开始协商新区块，旧的投票信息可以清空了
    validator_set_.update(round_state_);
    updateVoteSetValidator(); //将validator_set_中的验证者更新到vote_set_中
    proposal_validator_ = validator_set_.getProposalVaildtor(round_state_);

    timeOut(NEW_HEIGHT_STEP);
    return;
}
void PBFTStateMachine::updateVoteSetValidator()
{
    int size = validator_set_.size();
    for(int i = 0; i < size; ++i)
    {
        vote_set_.addValidatorPubKey(validator_set_.getValidator(i).pub_key);
    }
}
void PBFTStateMachine::enterNewRound(long h, long r)
{
    if(round_state_.height != h || round_state_.round > r || (round_state_.round == r && round_state_.step != NEW_HEIGHT_STEP))
    {
        clog(PBFTWarn) << "Enter invalid NewRound step: args(" << h << "," << r <<"), current(" << round_state_.height << "," << round_state_.round << "," << step2str(round_state_.step) << ")";
        return;
    }
    clog(PBFTTrace) << "Enter NewRound step(" << h << "," << r <<")";
    //在enterNewHeight中更新了验证者集合和提案者信息
    //在这里只更新了提案者信息，如果上一轮没有协商成功，那么下一轮必须更换提案者
    //而验证者集合，只有在协商的第0轮，才有可能更新
    proposal_validator_ = validator_set_.getProposalVaildtor(round_state_);
    vote_set_.clearPrecommitMaj32();
    vote_set_.clearPrecommitAny32();
    vote_set_.clearPrevoteMaj32();
    vote_set_.clearPrevoteAny32();
    round_state_.update(r, NEW_ROUND_STEP);
    //TODO 如果内存中没有交易，则不调用enterPropose阶段，而是产生NewHeightStep超时事件，重新进入enterNewRound
    //if(!pbft_interface_->getTxNum())
    //    timeOut(NEW_HEIGHT_STEP);
    //else
        enterPropose(h, r);
}
void PBFTStateMachine::enterPropose(long h, long r)
{
    if(round_state_.height != h || round_state_.round > r || (round_state_.round == r && PROPOSAL_STEP <= round_state_.step))
    {
        clog(PBFTWarn) << "Enter invalid Propose step: args(" << h << "," << r <<"), current(" << round_state_.height << "," << round_state_.round << "," << step2str(round_state_.step) << ")";
        return;
    }
    clog(PBFTTrace) << "Enter Propose step(" << h << "," << r <<")";
    if(isProposer()) //我是提案者
    {
        clog(PBFTDetail) << "As a proposer";
        createProposalInfo();
        sendProposal();
        round_state_.update(r, PROPOSAL_STEP);
    }
    else             //我不是提案者
    {
        round_state_.update(r, PROPOSAL_STEP);
        timeOut(PROPOSAL_STEP);
        clog(PBFTDetail) << "As not a proposer";
    }
}

void PBFTStateMachine::enterPrevote(long h, long r)
{
    if(round_state_.height != h || round_state_.round > r || (round_state_.round == r && PREVOTE_STEP <= round_state_.step))
    {
        clog(PBFTWarn) << "Enter invalid Prevote step: args(" << h << "," << r <<"), current(" << round_state_.height << "," << round_state_.round << "," << step2str(round_state_.step) << ")";
        return;
    }
    clog(PBFTTrace) << "Enter Prevote step(" << h << "," << r <<")";
    round_state_.update(r, PREVOTE_STEP);
    time_out_ticker_.pause();
    doPrevote(h, r);
}

bytes PBFTStateMachine::createVoteMsg(const bytes& msg, PBFT_STATE type)
{
    RLPStream rlp;
    try
    {
        h256 hash = createVoteMsgHash(round_state_.height, round_state_.round, type, msg);//哈希
        h520 signature = sign(key_pair_.sec(), hash); //签名

        rlp.clear();
        rlp.appendList(6);
        rlp.append(long2String(round_state_.height));
        rlp.append(long2String(round_state_.round));
        rlp.append(long2String(type));
        rlp.append(long2String(validator_set_.getValidatorIndex(key_pair_.pub())));
        rlp.append(hash.asBytes());
        rlp.append(signature.asBytes());
    }catch(...)
    {
        clog(PBFTError) << "Create vote msg failed";
    }
    return rlp.out();
}
bytes PBFTStateMachine::createVoteMsg(const std::string& msg, PBFT_STATE type)
{
    RLPStream rlp;
    return createVoteMsg(rlp.append(msg).out(), type);
}
void PBFTStateMachine::doPrevote(long h, long r)
{
    if(!isValidator()) return; //oh, 我不是验证者不能投票

    if(proposal_info_.is_valid) //提案信息有效
    {
        clog(PBFTTrace) << "Do prevote for proposal";
        sendMsg(VOTE, createVoteMsg(proposal_info_.proposal_hash, PREVOTE_STEP));
    }
    /*
       else if() //锁定信息有效
       {
    //TODO 投锁定信息
    }
    */
    else //空
    {
        clog(PBFTTrace) << "Do prevote for null";
        sendMsg(VOTE, createVoteMsg("NULL", PREVOTE_STEP));
    }
    //TODO preovte类型投票
}
void PBFTStateMachine::createProposalInfo()
{
    bytes consensus_data = pbft_interface_->createConsensusData();

    RLPStream rlp_proposal_data;
    rlp_proposal_data.appendList(3);
    rlp_proposal_data.append(long2String(round_state_.height));
    rlp_proposal_data.append(long2String(round_state_.round));
    rlp_proposal_data.append(consensus_data);

    proposal_info_.proposal_data = rlp_proposal_data.out();

    h256 hash = sha256(&(proposal_info_.proposal_data));//哈希
    proposal_info_.proposal_hash = hash.asBytes();

    h520 signature = sign(key_pair_.sec(), hash); //签名
    proposal_info_.sign = signature.asBytes();

}
void PBFTStateMachine::sendProposal() //将提案消息，扔到广播队列中
{
    sendMsg(PROPOSAL, proposal_info_.toBytes());
}
void PBFTStateMachine::sendMsg(PBFT_MSG_TYPE state, const bytes& msg)
{
    state_msg_collector_->pushMsg(PBFTMsg(state, msg)); //广播给其它节点
    this->pushMsg(PBFTMsg(state, msg)); //扔给自己
}
RoundState PBFTStateMachine::getRoundState()
{
    return round_state_;
}
bool PBFTStateMachine::isProposer()
{
    return key_pair_.pub() == proposal_validator_.pub_key;
}
