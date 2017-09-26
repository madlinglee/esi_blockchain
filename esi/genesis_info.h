/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file genesis_info.h
 * @brief 创世块等相关的json数据
 * @author LiJianfeng
 * @date 2017年06月07日
 */

#ifndef BLOCKCHAIN_GENESIS_INFO_H_
#define BLOCKCHAIN_GENESIS_INFO_H_
#include <libdevcrypto/Common.h>
#include <json_spirit/JsonSpiritHeaders.h>

using namespace dev;

namespace genesis
{

const Secret secret_ljf("acf545ba38f0d5ba6851a1a209765fabc17a6c5f40e8c97e859dff7feede8bfd");
const KeyPair keypair_ljf(secret_ljf);

std::string const genesis_info_ljf =
R"ETHEREUM(
{
    "sealEngine": "NoProof",
    "params": {
        "accountStartNonce": "0x",
        "maximumExtraDataSize": "0x1000000",
        "blockReward": "0x",
        "registrar": "",
        "networkID" : "0x45"
    },
    "genesis": {
        "author": "0x0000000000000000000000000000000000000000",
        "timestamp": "0x00",
        "parentHash": "0x0000000000000000000000000000000000000000000000000000000000000000",
        "extraData": "0x",
        "gasLimit": "0x1000000000000"
    },
    "accounts": {
        "0000000000000000000000000000000000000001": { "wei": "1", "precompiled": { "name": "ecrecover", "linear": { "base": 3000, "word": 0 } } },
        "0000000000000000000000000000000000000002": { "wei": "1", "precompiled": { "name": "sha256", "linear": { "base": 60, "word": 12 } } },
        "0000000000000000000000000000000000000003": { "wei": "1", "precompiled": { "name": "ripemd160", "linear": { "base": 600, "word": 120 } } },
        "0000000000000000000000000000000000000004": { "wei": "1", "precompiled": { "name": "identity", "linear": { "base": 15, "word": 3 } } },
        "b640dd8113e51af713aed4989f3aa6afcd97c252": { "balance": "880000000000000000000" },
        "35c0774937be8bc371d75382239ae7606e7d082a": { "balance": "880000000000000000000" },
        "125a46f811a8dee28e4aa821fe591445ea9132e3": { "balance": "880000000000000000000" },
        "71a0f33b8b1af0fb4719f9270b9d3d56c8b50f04": { "balance": "880000000000000000000" }
        //"b640dd8113e51af713aed4989f3aa6afcd97c252": { "wei": "1606938044258990275541962092341162602522202993782792835301376" }
    }
}
)ETHEREUM";

std::string const genesis_info_basic_authority_ljf =
R"E(
{
    "sealEngine": "BasicAuthority",
    "params": {
        "accountStartNonce": "0x00",
        "maximumExtraDataSize": "0x20",
        "minGasLimit": "0x1388",
        "maxGasLimit": "0x7fffffffffffffff",
        "gasLimitBoundDivisor": "0x0400",
        "minimumDifficulty": "0x020000",
        "difficultyBoundDivisor": "0x0800",
        "durationLimit": "0x0d",
        "blockReward": "0x4563918244F40000",
        "registrar" : "0xc6d9d2cd449a754c494264e1809c50e34d64562b",
        "networkID" : "0x1"
    },
    "genesis": {
        "nonce": "0x0000000000000042",
        "difficulty": "0x400000000",
        "mixHash": "0x0000000000000000000000000000000000000000000000000000000000000000",
        "author": "0x0000000000000000000000000000000000000000",
        "timestamp": "0x00",
        "parentHash": "0x0000000000000000000000000000000000000000000000000000000000000000",
        "extraData": "0x11bbe8db4e347b4e8c937c1c8370e4b5ed33adb3db69cbdb7a38e1e50b1b82fa",
		"gasLimit": "0x9999999999"
    },
    "accounts": {
        "0000000000000000000000000000000000000001": { "wei": "1", "precompiled": { "name": "ecrecover", "linear": { "base": 3000, "word": 0 } } },
        "0000000000000000000000000000000000000002": { "wei": "1", "precompiled": { "name": "sha256", "linear": { "base": 60, "word": 12 } } },
        "0000000000000000000000000000000000000003": { "wei": "1", "precompiled": { "name": "ripemd160", "linear": { "base": 600, "word": 120 } } },
        "0000000000000000000000000000000000000004": { "wei": "1", "precompiled": { "name": "identity", "linear": { "base": 15, "word": 3 } } },
        "b640dd8113e51af713aed4989f3aa6afcd97c252": { "balance": "880000000000000000000" },
        "71a0f33b8b1af0fb4719f9270b9d3d56c8b50f04": { "balance": "880000000000000000000" }
        //"b640dd8113e51af713aed4989f3aa6afcd97c252": { "wei": "1606938044258990275541962092341162602522202993782792835301376" }
    }
}
)E";

}

#endif
