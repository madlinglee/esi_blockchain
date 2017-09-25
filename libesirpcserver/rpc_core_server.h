/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file rpc_core_server.h
 * @brief 核心RPC服务器端
 * @author LiJianfeng
 * @date 2017年06月12日
 */

#pragma once

#include <libethcore/KeyManager.h>
#include <libweb3jsonrpc/SessionManager.h>
#include <libethereum/Interface.h>
#include "rpc_core_server_face.h"

class RPCCoreServer : public RPCCoreServerFace
{
public:
    RPCCoreServer(dev::eth::Interface& i, dev::eth::KeyManager& km,
        dev::rpc::SessionManager& sm);

    Json::Value newAccount(const Json::Value& acc,
        const std::string& session) override;
    Json::Value listAccountAddresses(const std::string& session) override;
    std::string getBalance(const std::string& address,
        const std::string& session) override;
    std::string submitTransaction(const Json::Value& tx,
        const std::string& pass) override;
private:
    dev::eth::Interface& client_; ///< 客户端接口
    dev::eth::KeyManager& key_manager_;///< 密钥管理器
    dev::rpc::SessionManager& session_manager_;///< 会话管理器
};
