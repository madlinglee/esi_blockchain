/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file seal_client.h
 * @brief 密封RPC服务器端
 * @author LiJianfeng
 * @date 2017年06月12日
 */

#pragma once
#include <jsonrpccpp/client.h>

class RPCSealClient : public jsonrpc::Client
{
public:
    RPCSealClient(jsonrpc::IClientConnector &conn,
        jsonrpc::clientVersion_t type = jsonrpc::JSONRPC_CLIENT_V2)
        : jsonrpc::Client(conn, type){}

    bool doMine() throw (jsonrpc::JsonRpcException);
};
