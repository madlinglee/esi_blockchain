/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file pbft_interface.h      
 * @brief 
 * @author Lilongpu
 * @date 2017年06月14日
 */
#ifndef PBFT_INTERFACE_H_
#define PBFT_INTERFACE_H_
#include "pbft_common.h"
using namespace dev;
class PBFTInterface
{
public:
	/**
	 * @brief 创建共识信息
	 *
	 * @return
	 */
	virtual bytes createConsensusData()
	{
		bytes data;
		std::string str("commit test data");
		data.resize(str.size());
		data.assign(str.begin(), str.end());
		return data;
	}; 

	/**
	 * @brief 提交数据
	 *
	 * @param data 数据
	 *
	 * @return 
	 */
	virtual bool commit(const bytes& data) 
	{
		std::string str;
		str.assign(data.begin(), data.end());
		printf("commit:%s\n", str.c_str());
		return false;
	};    

	/**
	 * @brief  验证数据的有效性
	 *
	 * @param data 待验证数据
	 *
	 * @return
	 */
	virtual bool verify(const bytes& data) {return true;};     //验证一个数据的有效性
	/**
	 * @brief 获取当前的高度
	 *
	 * @return 0
	 */
	virtual long getHeight(){return 0;}
	/**
	 * @brief 获取交易数量
	 *
	 * @return 0
	 */
	virtual long getTxNum(){return 0;}
};
#endif //PBFT_INTERFACE_H_
