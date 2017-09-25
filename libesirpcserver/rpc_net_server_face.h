/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file rpc_net_server_face.h
 * @brief 对等网络RPC服务器端接口
 * @author LiJianfeng
 * @date 2017年06月12日
 */

#pragma once

#include <libethcore/Transaction.h>
#include <libethereum/All.h>
#include <libweb3jsonrpc/ModularServer.h>

class RPCNetServerFace : public ServerInterface<RPCNetServerFace>
{
public:
    RPCNetServerFace()
    {
        this->bindAndAddMethod(jsonrpc::Procedure("peerCount",
            jsonrpc::PARAMS_BY_POSITION,
            jsonrpc::JSON_STRING,
            NULL),
            &RPCNetServerFace::peerCount);
        this->bindAndAddMethod(jsonrpc::Procedure("addPeer",
            jsonrpc::PARAMS_BY_POSITION,
            jsonrpc::JSON_BOOLEAN,
            "enode",
            jsonrpc::JSON_STRING,
            "session",
            jsonrpc::JSON_STRING,
            "required",
            jsonrpc::JSON_BOOLEAN,
            NULL),
            &RPCNetServerFace::addPeer);
    }

    virtual void peerCount(const Json::Value& request,
        Json::Value& response)
    {
        (void)request;
        response = this->peerCount();
    }

    virtual void addPeer(const Json::Value& request,
        Json::Value& response)
    {
        response = this->addPeer(request[0].asString(), request[1].asString(),
            request[2].asBool());
    }

    virtual RPCModules implementedModules() const override
    {
        return RPCModules{RPCModule{"seal", "1.0"}};
    }

    /**
     * @brief 获取节点连接数
     *
     * @return 节点连接数
     */
    virtual std::string peerCount() = 0;
    /**
     * @brief 连接节点
     *
     * @param[in] enode 连接地址，格式：enode://pk@ip:port
     * @param[in] session 会话密钥
     * @param[in] required 是否是信任节点（required）
     *
     * @return true 连接成功
     */
    virtual bool addPeer(const std::string& enode,
        const std::string& session, bool required) = 0;
};
