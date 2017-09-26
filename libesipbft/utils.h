/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file utils.h
 * @brief
 * @author Lilongpu
 * @date 2017年06月13日
 */
#ifndef UTILS_H_
#define UTILS_H_
#include <string>
#include "libdevcore/RLP.h"
#include "libdevcrypto/Hash.h"
#include "pbft_common.h"
using namespace dev;
/**
 * @brief long转为string
 *
 * @param[in] num 长整型数
 *
 * @return string
 */
std::string long2String(long num);
/**
 * @brief 打印RLP格式数据
 *
 * @param[in] r
 * @param[in] for_i
 * @param[in] item_i
 */
void displayRLP(const RLP& r, int for_i = 0, int item_i = 0);
/**
 * @brief 打印RLPStream
 *
 * @param[in] rlp
 * @param[in] for_i
 * @param[in] item_i
 */
void displayRLP(RLPStream& rlp, int for_i = 0, int item_i = 0);
const char* step2str(long step);
/**
 * @brief 创建voteMsgHash
 *
 * @param[in] h
 * @param[in] r
 * @param[in] vote_type
 * @param[in] msg
 *
 * @return h256
 */
h256 createVoteMsgHash(long h, long r, PBFT_STATE vote_type, const bytes& msg);
/**
 * @brief long转换为PBFT_STATE
 *
 * @param type 类型
 *
 * @return PBFT_STATE
 */
PBFT_STATE long2PBFT_STATE(long type);
/**
 * @brief 字节数据转为string
 *
 * @param data 二进制数据
 *
 * @return string
 */
std::string bytes2str(const bytes& data);
/**
 * @brief string转换为字节bytes
 *
 * @param str 字符串
 *
 * @return bytes
 */
bytes str2bytes(const std::string& str);
#endif //UTILS_H_
