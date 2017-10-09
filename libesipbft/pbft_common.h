/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file pbft_msg.h
 * @brief
 * @author Lilongpu
 * @date 2017年06月10日
 */
#ifndef PBFT_COMM_H_
#define PBFT_COMM_H_
#include <libdevcore/Log.h>
#include <libdevcore/Common.h>
#include <libdevcrypto/Common.h>
using namespace dev;
using PubKey = dev::Public; //64字节的公钥
using SecKey = dev::Secret; //32字节的私钥

struct PBFTNote: public LogChannel 
{ 
    static const char* name()
    {
        return EthYellow "❇" EthBlue " ℹ";
    } 
    static const int verbosity = 2; 
};
struct PBFTChat: public LogChannel 
{ 
    static const char* name()
    {
        return EthYellow "❇" EthWhite " ◌";
    } 
    static const int verbosity = 4;
};
struct PBFTTrace: public LogChannel
{ 
    static const char* name()
    {
        return EthYellow "❇" EthGray " ◎";
    } 
    static const int verbosity = 7; 
};
struct PBFTDetail: public LogChannel 
{
    static const char* name()
    {
        return EthYellow "❇" EthNavy " ●";
    } 
    static const int verbosity = 8; 
};
struct PBFTWarn: public LogChannel 
{ 
    static const char* name()
    {
        return EthYellow "❇" EthOrange " ◇";
    }
    static const int verbosity = 9;
};
struct PBFTError: public LogChannel 
{
    static const char* name()
    {
        return EthYellow "❇" EthRed " ✘";
    }
    static const int verbosity = 10;
};

/**
 * @brief 消息类型
 */
enum PBFT_MSG_TYPE
{
    UNVALID_TYPE = 0x0,
    /**
     * @brief 提案消息
     */
    PROPOSAL,

    /**
     * @brief 投票消息
     */
    VOTE,

    /**
     * @brief 超时消息
     */
    TIME_OUT,
};

/**
 * @brief 状态机可能的状态
 */
enum PBFT_STATE
{
    UNVALID_STATE =0x0,
    NEW_HEIGHT_STEP,
    NEW_ROUND_STEP,
    PROPOSAL_STEP,
    PREVOTE_STEP,
    PREVOTEWAIT_STEP,
    PRECOMMIT_STEP,
    PRECOMMITWAIT_STEP,
    COMMI_STEP,
    MAX_STEP,
};
/**
 * @brief pbft信息
 */
typedef struct PBFTMsg_tag
{
    PBFTMsg_tag(PBFT_MSG_TYPE msg_type = UNVALID_TYPE, dev::bytes msg_content=dev::bytes())
        :msg_type_(msg_type),msg_content_(msg_content)
    {
    }
    PBFT_MSG_TYPE msg_type_;
    dev::bytes msg_content_;
} PBFTMsg;
/**
 * @brief 轮次状态
 */
typedef struct RoundState_tag
{
    long         height;
    long         round;
    PBFT_STATE     step;
    void update(long r, PBFT_STATE s){round = r; step = s;}
    void update(long h, long r, PBFT_STATE s){height = h; round = r; step = s;}
}RoundState;
#endif //PBFT_COMMON_H_
