/**
 * @file key_wrapped.cpp
 * @brief
 * @author Lilongpu
 * @date 2017年06月23日
 */
#include "key_wrapped.h"
#include "pbft_common.h"
void KeyWrapped::createKeyFromSeed(const std::string& seed)
{
    dev::Secret sec = dev::eth::KeyManager::brain(seed);
    dev::KeyPair keyPair(sec);
    pub_key_=keyPair.pub();
    sec_key_=keyPair.sec();
}
void KeyWrapped::createKeyFromSec(const SecKey& sec)
{
    dev::KeyPair keyPair(sec);
    pub_key_=keyPair.pub();
    sec_key_=keyPair.sec();
}
const SecKey& KeyWrapped::sec()
{
    return sec_key_;
}
const PubKey& KeyWrapped::pub()
{
    return pub_key_;
}
