/**
 * @file rpc_net_server.cpp
 * @brief 密封RPC服务器端
 * @author LiJianfeng
 * @date 2017年06月27日
 */

#include <jsonrpccpp/common/exception.h>
#include "libdevcore/CommonJS.h"
#include "libweb3jsonrpc/JsonHelper.h"
#include "rpc_net_server.h"

using namespace std;
using namespace dev;
using namespace dev::p2p;
using namespace dev::rpc;
using namespace jsonrpc;

RPCNetServer::RPCNetServer(Host& h, SessionManager& sm) :
    host_(h), session_manager_(sm)
{}

string RPCNetServer::peerCount()
{
    return toJS(host_.peerCount());
}

bool RPCNetServer::addPeer(const string& enode,
    const string& session, bool required)
{
    if (!session_manager_.hasPrivilegeLevel(session, Privilege::Admin))
        throw jsonrpc::JsonRpcException("Invalid privileges");
    required ? host_.addPeer(NodeSpec(enode), PeerType::Required) :
        host_.addPeer(NodeSpec(enode), PeerType::Optional);
    return true;
}
