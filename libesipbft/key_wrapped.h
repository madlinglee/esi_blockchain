/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file key_create_interface.h
 * @brief
 * @author Lilongpu
 * @date 2017年06月19日
 */
#ifndef KEY_WRAPPED_H_
#define KEY_WRAPPED_H_
#include "libdevcrypto/Common.h"
#include "libethcore/KeyManager.h"
#include "pbft_common.h"
/**
 * @brief 密钥管理
 */
class KeyWrapped
{
public:
    /**
     * @brief 根据种子，生成公钥对
     *
     * @param seed 种子
     */
    void createKeyFromSeed(const std::string& seed);
    /**
     * @brief 根据私钥，创建公私钥对
     *
     * @paramsec
     */
    void createKeyFromSec(const SecKey& sec);

    /**
     * @brief 获取私钥
     *
     * @return 私钥
     */
    const SecKey& sec();
    /**
     * @brief 获取公钥
     *
     * @return 公钥
     */
    const PubKey& pub();
private:
    SecKey sec_key_;
    PubKey pub_key_;
};
#endif //KEY_WRAPPED_H_
