/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file proposal_info.h      
 * @brief 
 * @author Lilongpu
 * @date 2017年06月14日
 */
#ifndef PROPOSAL_INFO_H_
#define PROPOSAL_INFO_H_
#include "libdevcore/Common.h"
#include "libdevcore/RLP.h"
using namespace dev;
/**
 * @brief 提案信息
 */
typedef struct ProposalInfo_tag
{
	bytes 	proposal_hash; //hash(proposal_data)
	bytes 	proposal_data; //rlp格式的真实数据
	bytes   sign; 	       //sign(propsal_hash)
	bool    is_valid;       //提案信息是否有效
	ProposalInfo_tag():is_valid(false){}
	void clear()
	{
		proposal_hash.clear();
		proposal_data.clear();
		sign.clear();
		is_valid=false;
	}
	bytes toBytes()
	{
		RLPStream rlp;
		rlp.appendList(3);
		rlp.append(proposal_hash);
		rlp.append(proposal_data);
		rlp.append(sign);
		return rlp.out();
	}
}ProposalInfo;
#endif //PROPOSAL_INFO_H_
