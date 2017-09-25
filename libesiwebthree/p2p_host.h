/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file p2p_host.h
 * @brief P2P和客户端包装器
 * @author LiJianfeng
 * @date 2017年07月11日
 */

#pragma once

#include <libdevcore/Common.h>
#include <libdevcore/CommonIO.h>
#include <libdevcore/Exceptions.h>
#include <libp2p/Host.h>
#include <libethereum/ChainParams.h>
#include "pbft_client.h"

class P2PHost
{
public:
    P2PHost(const std::string& client_version,
        const std::string& db,
        const eth::ChainParams& cp,
        WithExisting we = WithExisting::Trust,
        const p2p::NetworkPreferences& np = p2p::NetworkPreferences(),
        bytesConstRef network = bytesConstRef()
    );
    ~P2PHost();

    pbft::PBFTClient* client()
    {
        return client_.get();
    }

    dev::eth::Client* ethClient()
    {
        return static_cast<dev::eth::Client*>(client_.get());
    }

    void requirePeer(const p2p::NodeID& id, const bi::tcp::endpoint& endpoint);
    void requirePeer(const p2p::NodeID& id, const std::string& ip_port);
    std::string enode() const;
private:
    p2p::Host host_;
    std::unique_ptr<pbft::PBFTClient> client_;
};
