/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file rpc_core_server_face.h
 * @brief 核心RPC服务器端接口
 * @author LiJianfeng
 * @date 2017年06月12日
 */

#pragma once

#include <libethcore/Transaction.h>
#include <libethereum/All.h>
#include <libweb3jsonrpc/ModularServer.h>

class RPCCoreServerFace : public ServerInterface<RPCCoreServerFace>
{
public:
    RPCCoreServerFace()
    {
        this->bindAndAddMethod(jsonrpc::Procedure("newAccount",
            jsonrpc::PARAMS_BY_POSITION,
            jsonrpc::JSON_OBJECT,
            "account",
            jsonrpc::JSON_OBJECT,
            "session",
            jsonrpc::JSON_STRING,
            NULL),
            &RPCCoreServerFace::newAccount);
        this->bindAndAddMethod(jsonrpc::Procedure("listAccountAddresses",
            jsonrpc::PARAMS_BY_POSITION,
            jsonrpc::JSON_OBJECT,
            "session",
            jsonrpc::JSON_STRING,
            NULL),
            &RPCCoreServerFace::listAccountAddresses);
        this->bindAndAddMethod(jsonrpc::Procedure("getBalance",
            jsonrpc::PARAMS_BY_POSITION,
            jsonrpc::JSON_STRING,
            "address",
            jsonrpc::JSON_STRING,
            "session",
            jsonrpc::JSON_STRING,
            NULL),
            &RPCCoreServerFace::getBalance);
        this->bindAndAddMethod(jsonrpc::Procedure("submitTransaction",
            jsonrpc::PARAMS_BY_POSITION,
            jsonrpc::JSON_STRING,
            "transcation",
            jsonrpc::JSON_OBJECT,
            "password",
            jsonrpc::JSON_STRING,
            NULL),
            &RPCCoreServerFace::submitTransaction);
    }

    virtual void newAccount(const Json::Value& request,
        Json::Value& response)
    {
        response = this->newAccount(request[0], request[1].asString());
    }

    virtual void listAccountAddresses(const Json::Value& request,
        Json::Value& response)
    {
        response = this->listAccountAddresses(request[0].asString());
    }

    virtual void getBalance(const Json::Value& request,
        Json::Value& response)
    {
        response = this->getBalance(request[0].asString(),
            request[1].asString());
    }

    virtual void submitTransaction(const Json::Value& request,
        Json::Value& response)
    {
        response = this->submitTransaction(request[0], request[1].asString());
    }

    virtual RPCModules implementedModules() const override
    {
        return RPCModules{RPCModule{"seal", "1.0"}};
    }

    /**
     * @brief 创建账户
     *
     * @param[in] acc 账户
     * @param[in] session 会话密钥
     *
     * @return 交易哈希字符串
     */
    virtual Json::Value newAccount(const Json::Value& acc,
        const std::string& session) = 0;

    /**
     * @brief 获取所有账户地址
     *
     * @param[in] session 会话密钥
     *
     * @return 账户地址
     */
    virtual Json::Value listAccountAddresses(const std::string& session) = 0;

    /**
     * @brief 获取指定账户余额
     *
     * @param[in] address 账户地址
     *
     * @param[in] session 会话密钥
     *
     * @return 余额
     */
    virtual std::string getBalance(const std::string& address,
        const std::string& session) = 0;

    /**
     * @brief 构造并提交交易
     *
     * @param[in] tx JS格式的交易骨架
     * @param[in] pass 私钥密码
     *
     * @return 交易哈希字符串
     */
    virtual std::string submitTransaction(const Json::Value& tx,
        const std::string& pass) = 0;
};
