/**
 * @file p2p_host.cpp
 * @brief P2P和客户端包装器
 * @author LiJianfeng
 * @date 2017年07月11日
 */

#include <libethcore/BasicAuthority.h>
#include <libethereum/Defaults.h>
#include "p2p_host.h"

using namespace std;
using namespace dev;
using namespace p2p;
using namespace pbft;

P2PHost::P2PHost(const string& client_version,
    const string& db,
    const ChainParams& cp,
    WithExisting we,
    const NetworkPreferences& np,
    bytesConstRef network
):host_(client_version, np, network)
{
    if(db.size())
        Defaults::setDBPath(db);
    BasicAuthority::init();
    client_.reset(new PBFTClient(cp, (int)cp.u256Param("networkID"), &host_,
        shared_ptr<GasPricer>(), getDataDir(), we));
    host_.start();
}

P2PHost::~P2PHost()
{
    host_.stop();
    client_.reset();
}

void P2PHost::requirePeer(const NodeID& id, const bi::tcp::endpoint& endpoint)
{
    host_.requirePeer(id, NodeIPEndpoint(endpoint.address(),
        endpoint.port(), endpoint.port()));
}

void P2PHost::requirePeer(const NodeID& id, const string& ip_port)
{
    requirePeer(id, Network::resolveHost(ip_port));
}

string P2PHost::enode() const
{
    return host_.enode();
}
