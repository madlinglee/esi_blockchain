/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file common.h
 * @brief 公共类
 * @author SongYuanjie
 * @date 2017年06月28日
 */
#pragma once
#include <libesipbft/validator_set.h>

/**
 * @brief 获取验证者
 *
 * @param seed 种子
 *
 * @return
 */
static Validator getValidator(const std::string &seed)
{
    //1, 生成私钥
    //2, 生成公私钥对
    //3, 生成公钥
    dev::Secret sec = dev::eth::KeyManager::brain(seed);
    dev::KeyPair key_pair(sec);
    Validator validator;
    validator.pub_key = key_pair.pub();

    validator.name = seed;
    return validator;
}
