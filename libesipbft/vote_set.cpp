/**
 * @file vote_set.cpp
 * @brief
 * @author Lilongpu
 * @date 2017年06月23日
 */
#include "vote_set.h"
#include "utils.h"
void VoteSet::clear()
{
    prevote_.clear();
    precommit_.clear();
    validators_.clear();
    round_prevote_count_.clear();
    round_hash_prevote_count_.clear();
    round_precommit_count_.clear();
    round_hash_precommit_count_.clear();
    prevote_any32_.clear();
    prevote_maj32_.clear();
    precommit_any32_.clear();
    precommit_maj32_.clear();
}
void VoteSet::addVote(Vote vote)
{
    if(isExistVote(vote))
    {
        printf("[Trace] vote is exist\n");
        return;
    }
    if(PREVOTE_STEP == vote.vote_type)
    {
        prevote_.push_back(vote);
        updateVote(prevote_.back());
    }
    else if(PRECOMMIT_STEP == vote.vote_type)
    {
        precommit_.push_back(vote);
        updateVote(precommit_.back());
    }
}
void VoteSet::addVote(long h, long r, PBFT_STATE vote_type, const PubKey& pub_key, const bytes& data_hash, const bytes& sign)
{
    addVote(Vote(h, r, vote_type, pub_key, data_hash, sign));
}
bool VoteSet::hasTwoThirdsMajority(PBFT_STATE vote_type)
{
    if(PREVOTE_STEP == vote_type)
    {
        return (-1 != prevote_maj32_.round);
    }
    else if(PRECOMMIT_STEP == vote_type)
    {
        return (-1 != precommit_maj32_.round);
    }
    return false;
}
bool VoteSet::hasTwoThirdsAny(PBFT_STATE vote_type)
{
    if(PREVOTE_STEP == vote_type)
    {
        return (-1 != prevote_any32_.round);
    }
    else if(PRECOMMIT_STEP == vote_type)
    {
        return (-1 != precommit_any32_.round);
    }
    return false;
}
MAJ32 VoteSet::getTwoThirdsMajority(PBFT_STATE vote_type)
{
    if(PREVOTE_STEP == vote_type)
    {
        return prevote_maj32_;
    }
    else if(PRECOMMIT_STEP == vote_type)
    {
        return precommit_maj32_;
    }
    return MAJ32();
}
ANY32 VoteSet::getTwoThirdsAny(PBFT_STATE vote_type)
{
    if(PREVOTE_STEP == vote_type)
    {
        return prevote_any32_;
    }
    else if(PRECOMMIT_STEP == vote_type)
    {
        return precommit_any32_;
    }
    return ANY32();
}
void VoteSet::addValidatorPubKey(const PubKey& pub_key)
{
    validators_.insert(pub_key);
}
bool VoteSet::isExistVote(Vote vote)
{
    if(vote.vote_type == PREVOTE_STEP)
    {
        std::list<Vote>::iterator it = prevote_.begin();
        std::list<Vote>::iterator end = prevote_.end();
        for( ; it != end; ++it)
        {
            if(vote == *it)
                return true;
        }
        return false;
    }
    else if(vote.vote_type == PRECOMMIT_STEP)
    {
        std::list<Vote>::iterator it = precommit_.begin();
        std::list<Vote>::iterator end = precommit_.end();
        for( ; it != end; ++it)
        {
            if(*it == vote)
                return true;
        }
        return false;
    }
    return false;
}
void VoteSet::updateVote(const Vote& vote)
{
    long count = 0;
    long threshold = (validators_.size()*2)/3 + 1;
    printf("threshold:%ld\n", threshold);
    if(vote.vote_type == PREVOTE_STEP)
    {
        count = ++round_prevote_count_[vote.round].count;
        printf("prevote any count:%ld,vote.height:%ld,vote.round:%ld\n", count, vote.height, vote.round);
        if((size_t)count >= (validators_.size()*2)/3 + 1)
        {
            prevote_any32_.round = vote.round;
            printf("got any32!\n");
        }
        count = ++round_hash_prevote_count_[vote.round][vote.voted_proposal_hash].count;
        printf("prevote maj count:%ld,vote.height:%ld,vote.round:%ld\n", count, vote.height, vote.round);
        if((size_t)count >= (validators_.size()*2)/3 + 1)
        {
            printf("got maj32!\n");
            prevote_maj32_.round = vote.round;
            prevote_maj32_.voted_proposal_hash = vote.voted_proposal_hash;
        }
    }
    else if(vote.vote_type == PRECOMMIT_STEP)
    {
        count = ++round_precommit_count_[vote.round].count;
        printf("precommit any count:%ld,vote.height:%ld,vote.round:%ld\n", count, vote.height, vote.round);
        if((size_t)count >= (validators_.size()*2)/3 + 1)
        {
            precommit_any32_.round = vote.round;
            printf("got any32!\n");
        }
        count = ++round_hash_precommit_count_[vote.round][vote.voted_proposal_hash].count;
        printf("precommit maj count:%ld,vote.height:%ld,vote.round:%ld\n", count, vote.height, vote.round);
        if((size_t)count >= (validators_.size()*2)/3 + 1)
        {
            printf("got maj32!\n");
            precommit_maj32_.round = vote.round;
            precommit_maj32_.voted_proposal_hash = vote.voted_proposal_hash;
        }
    }
}
