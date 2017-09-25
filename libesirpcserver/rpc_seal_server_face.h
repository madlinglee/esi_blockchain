/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file rpc_seal_server_face.h
 * @brief 密封RPC服务器端接口
 * @author LiJianfeng
 * @date 2017年06月12日
 */

#pragma once

#include <libweb3jsonrpc/ModularServer.h>

class RPCSealServerFace : public ServerInterface<RPCSealServerFace>
{
public:
    RPCSealServerFace()
    {
        this->bindAndAddMethod(jsonrpc::Procedure("doMine",
            jsonrpc::PARAMS_BY_POSITION,
            jsonrpc::JSON_BOOLEAN,
            "session",
            jsonrpc::JSON_STRING,
            NULL),
            &RPCSealServerFace::doMine);
        this->bindAndAddMethod(jsonrpc::Procedure("stopMining",
            jsonrpc::PARAMS_BY_POSITION,
            jsonrpc::JSON_BOOLEAN,
            "session",
            jsonrpc::JSON_STRING,
            NULL),
            &RPCSealServerFace::stopMining);
    }

    virtual void doMine(const Json::Value& request,
        Json::Value& response)
    {
        response = this->doMine(request[0].asString());
    }

    virtual void stopMining(const Json::Value& request,
        Json::Value& response)
    {
        response = this->stopMining(request[0].asString());
    }

    virtual RPCModules implementedModules() const override
    {
        return RPCModules{RPCModule{"seal", "1.0"}};
    }

    /**
     * @brief 开启密封/挖矿
     *
     * @param[in] session 会话密钥
     *
     * @return true 开启成功
     */
    virtual bool doMine(const std::string& session) = 0;

    /**
     * @brief 停止密封/挖矿
     *
     * @param[in] session 会话密钥
     *
     * @return true 停止成功
     */
    virtual bool stopMining(const std::string& session) = 0;
};
