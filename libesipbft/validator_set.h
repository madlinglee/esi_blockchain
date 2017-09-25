/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file validator_set.h
 * @brief
 * @author Lilongpu
 * @date 2017年06月14日
 */

#ifndef VALIDATOR_SET_H_
#define VALIDATOR_SET_H_
#include "pbft_common.h"
#include "libethcore/KeyManager.h"
typedef struct Validator_tag
{
    std::string name;  //验证者的名字
    PubKey         pub_key;//公钥
}Validator;
//TODO not thread safe
class ValidatorSet
{
public:
    /**
     * @brief 插入验证者
     *
     * @param validator 验证者信息
     */
    void insertValidator(const Validator& validator);
    /**
     * @brief 验证者数量
     *
     * @return 数量
     */
    int validatorNum();
    /**
     * @brief 验证者集合大小
     *
     * @return 返回大小
     */
    int size();
    /**
     * @brief 选取验证者
     *
     * @param[in] index 验证者索引
     *
     * @return  验证者
     */
    Validator getValidator(int index);
    /**
     * @brief 验证者是否存在
     *
     * @param[in] pub_key 公钥
     *
     * @return true 存在，false 不存在
     */
    int isExistValidator(const PubKey& pub_key);
    /**
     * @brief 验证者索引
     *
     * @param[in] pub_key 公钥
     *
     * @return 验证者
     */
    int getValidatorIndex(const PubKey& pub_key);
    //TODO 更新验证者信息
    /**
     * @brief 更新当前状态信息
     *
     * @param[in] rs 状态
     */
    void update(RoundState rs);
	/**
	 * @brief 获取共识的提案者
	 *
	 * @param[in] rs 状态
	 *
	 * @return 提案者
	 */
    Validator getProposalVaildtor(RoundState rs); //TODO 选举算法
    /**
     * @brief 初始化信息
     */
    void init(); //TODO do something?
private:
    std::vector<Validator>       validators_;
};
#endif //VALIDATOR_SET_H_
