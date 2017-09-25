/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file rpc_seal_server.h
 * @brief 密封RPC服务器端
 * @author LiJianfeng
 * @date 2017年06月12日
 */

#pragma once

#include <libweb3jsonrpc/SessionManager.h>
#include "../libesiconsensus/consenter.h"
#include "rpc_seal_server_face.h"

class RPCSealServer : public RPCSealServerFace
{
public:
    RPCSealServer(Consenter& cons, dev::rpc::SessionManager& sm);

    bool doMine(const std::string& session) override;
    bool stopMining(const std::string& session) override;
private:
    Consenter& cons_; ///< 共识接口
    dev::rpc::SessionManager& session_manager_;///< 会话管理器
};
