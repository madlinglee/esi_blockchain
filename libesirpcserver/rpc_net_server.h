/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file rpc_net_server.h
 * @brief 密封RPC服务器端
 * @author LiJianfeng
 * @date 2017年06月12日
 */

#pragma once

#include <libp2p/Host.h>
#include <libweb3jsonrpc/SessionManager.h>
#include "rpc_net_server_face.h"

class RPCNetServer : public RPCNetServerFace
{
public:
    RPCNetServer(dev::p2p::Host& h, dev::rpc::SessionManager& sm);

    std::string peerCount() override;
    bool addPeer(const std::string& enode,
        const std::string& session, bool required = false) override;
private:
    dev::p2p::Host& host_; ///< 网络主机
    dev::rpc::SessionManager& session_manager_;///< 会话管理器
};
