/**
 * @file validator_set.cpp      
 * @brief 
 * @author Lilongpu
 * @date 2017年06月23日
 */
#include "validator_set.h"
void ValidatorSet::insertValidator(const Validator& validator)
{
	if(isExistValidator(validator.pub_key) == -1) //当前不存在该验证者时，才插入
		validators_.push_back(validator);
}
int ValidatorSet::validatorNum()
{
	return validators_.size();
}
int ValidatorSet::size()
{
	return validatorNum();
}
Validator ValidatorSet::getValidator(int index)
{
	assert((int)(validators_.size()) > index);
	return validators_[index];
}
int ValidatorSet::isExistValidator(const PubKey& pub_key)
{
	int size = validators_.size();
	for(int i = 0; i < size; ++i)
	{
		if(pub_key == validators_[i].pub_key)
			return i;
	}
	return -1;
}
int ValidatorSet::getValidatorIndex(const PubKey& pub_key)
{
	return isExistValidator(pub_key);
}
void ValidatorSet::update(RoundState rs)
{
	//TODO 更新验证者信息
}
Validator ValidatorSet::getProposalVaildtor(RoundState rs)
{
	//TODO 获取一个提案者
	assert(validators_.size() != 0);
	int i = (rs.round)%(validators_.size());
	assert((int)(validators_.size()) > i);
	return validators_[0];
}
void ValidatorSet::init()
{//TODO only for test. read xml file or contract
}
