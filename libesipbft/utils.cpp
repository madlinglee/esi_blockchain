/**
 * @file utils.cpp
 * @brief
 * @author Lilongpu
 * @date 2017年06月13日
 */
#include "utils.h"
#include "pbft_common.h"
std::string long2String(long num)
{
    char temp_buf[32];
    snprintf(temp_buf, 31, "%ld", num);
    return temp_buf;
}
void displayRLP(const RLP& r, int for_i, int item_i)
{
    if(r.isData())
    {
        printf("item[%d:%d]:%s\n", for_i++, item_i, r.toString().c_str());
    }
    else if(r.isInt())
    {
        printf("int[]:%d\n",r.toInt());
    }
    else if(r.isList())
    {
        RLP::iterator it = r.begin();
        RLP::iterator end = r.end();
        for_i++;
        for( int item_i = 0; it != end; ++it, ++item_i)
        {
            displayRLP(*it, for_i, item_i);
        }
    }
    else
    {
    }
}
void displayRLP(RLPStream& rlp, int for_i, int item_i)
{
    RLP r(rlp.out());
    displayRLP(r, for_i, item_i);
}
const char* step2str(long step)
{
    switch(step)
    {
        case NEW_HEIGHT_STEP:
            return "NEW_HEIGHT_STEP";
            break;
        case PROPOSAL_STEP:
            return "PROPOSAL_STEP";
            break;
        case PREVOTE_STEP:
            return "PREVOTE_STEP";
            break;
        case PREVOTEWAIT_STEP:
            return "PREVOTEWAIT_STEP";
            break;
        case PRECOMMIT_STEP:
            return "PRECOMMIT_STEP";
            break;
        case PRECOMMITWAIT_STEP:
            return "PRECOMMITWAIT_STEP";
            break;
        default:
            return "UNKNOWN STEP";
    }
}
h256 createVoteMsgHash(long h, long r, PBFT_STATE vote_type, const bytes& msg)
{
        RLPStream rlp;
        rlp.appendList(4);
        rlp.append(long2String(h));
        rlp.append(long2String(r));
        rlp.append(long2String(vote_type));
        rlp.append(msg);
        bytes data = rlp.out();
        return sha256(&data);//哈希
}
PBFT_STATE long2PBFT_STATE(long type)
{
    if(UNVALID_STATE<type && type<MAX_STEP)
        return (PBFT_STATE)type;
    else
        return UNVALID_STATE;
}
bytes str2bytes(const std::string& str)
{
    bytes data;
    data.resize(str.size());
    data.assign(str.begin(), str.end());
    return data;
}
std::string bytes2str(const bytes& data)
{
    std::string str;
    str.assign(data.begin(), data.end());
    return str;
}
