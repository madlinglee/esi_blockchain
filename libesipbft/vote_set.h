/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file vote_set.h
 * @brief
 * @author Lilongpu
 * @date 2017年06月20日
 */
#ifndef VOTE_SET_H_
#define VOTE_SET_H_
#include "pbft_common.h"
#include "validator_set.h"
#include <map>
using namespace dev;
/**
 * @brief 定义投票信息
 */
typedef struct Vote_tag
{
    Vote_tag(){}
    Vote_tag(long h, long r, PBFT_STATE type, const PubKey& key, const bytes& data_hash, const bytes& sig)
        :height(h), round(r), vote_type(type), pub_key(key), voted_proposal_hash(data_hash), sign(sig)
    {
    }
    /**
     * @brief 投票信息的高度
     */
    long         height;
    /**
     * @brief 投票的轮次
     */
    long        round;
    /**
     * @brief 投票类型
     */
    PBFT_STATE  vote_type;
    /**
    * @brief 投票人
    */
    PubKey      pub_key;
    /**
     * @brief 被投票信息的哈希
     */
    bytes        voted_proposal_hash;
    /**
    * @brief 签名
    */
    bytes        sign;
    bool operator==(const Vote_tag& vote)
    {
        return
            (height == vote.height)&&
            (round == vote.round)&&
            (vote_type == vote.vote_type)&&
            (pub_key == vote.pub_key)&&
            (voted_proposal_hash == vote.voted_proposal_hash);
//            (sign == vote.sign); //每次签名的信息都不同，因此签名信息不参与是否相等的判断
    }
}Vote;
typedef struct Long_tag
{
    Long_tag():count(0){};
    long count;
}Long;
/**
 * @brief 定义达到任意三分之二多数票的轮次
 */
typedef struct ANY32_tag
{
    ANY32_tag():round(-1){};
    long round;
    void clear(){round = -1;}
}ANY32;
/**
 * @brief 定义达到三分之二多数票的轮次和被投票信息
 */
typedef struct MAJ32_tag
{
    MAJ32_tag():round(-1){};
    long round;
    bytes voted_proposal_hash;
    void clear(){round = -1; voted_proposal_hash.clear();}
}MAJ32;
/**
 * @brief 投票的集合
 */
class VoteSet
{
public:

    /**
     * @brief 清空投票集合
     */
    void clear();
    /**
     * @brief 增加投票
     *
     * @param[in] vote 投票
     */
    void addVote(Vote vote);
    /**
     * @brief 增加投票
     *
     * @param[in] h 高度
     * @param[in] r 轮次
     * @param[in] vote_type 投票的类型
     * @param[in] pub_key 公钥
     * @param[in] data_hash 数据hash
     * @param[in] sign 签名信息
     */
    void addVote(long h, long r, PBFT_STATE vote_type, const PubKey& pub_key, 
			const bytes& data_hash, const bytes& sign);
    /**
     * @brief 判断某一状态下，投票总数超过2/3
     *
     * @param[in] vote_type 投票类型
     *
     * @return ture 超过， false未超过
     */
    bool hasTwoThirdsMajority(PBFT_STATE vote_type);
    /**
     * @brief 任意投票数超过2/3
     *
     * @param[in] vote_type 投票类型
     *
     * @return true 成功, false,失败
     */
    bool hasTwoThirdsAny(PBFT_STATE vote_type);
    /**
     * @brief  投票超过2/3
     *
     * @param vote_type 投票信息
     *
     * @return
     */
    MAJ32 getTwoThirdsMajority(PBFT_STATE vote_type);
    /**
     * @brief 获取任意超过2/3的投票
     *
     * @param[in] vote_type 投票类型
     *
     * @return 任意2/3投票
     */
    ANY32 getTwoThirdsAny(PBFT_STATE vote_type);
    /**
     * @brief 增加验证者
     *
     * @param[in] pub_key 公钥
     */
    void addValidatorPubKey(const PubKey& pub_key);
	void clearPrevoteMaj32(){prevote_maj32_.clear();}
	void clearPrecommitMaj32(){precommit_maj32_.clear();}
	void clearPrevoteAny32(){prevote_any32_.clear();}
	void clearPrecommitAny32(){precommit_any32_.clear();}
private:
    /**
     * @brief 是否存在投票
     *
     * @param[in] vote 投票
     *
     * @return true, 存在，false，不存在
     */
    bool isExistVote(Vote vote);
    /**
     * @brief 更新投票信息
     *
     * @paramp[in] vote 投票
     */
    void updateVote(const Vote& vote);
private:
    std::list<Vote>     prevote_;
    std::list<Vote>     precommit_;
    std::set<PubKey>     validators_;
    std::map<long, Long> round_prevote_count_;
    std::map<long, std::map<bytes, Long> > round_hash_prevote_count_;
    std::map<long, Long> round_precommit_count_;
    std::map<long, std::map<bytes, Long> > round_hash_precommit_count_;
    ANY32              prevote_any32_;
    MAJ32              prevote_maj32_;
    ANY32              precommit_any32_;
    MAJ32              precommit_maj32_;
};
#endif //VOTE_SET_H_
