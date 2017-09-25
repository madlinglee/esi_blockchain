/**
 * @file rpc_seal_server.cpp
 * @brief 密封RPC服务器端
 * @author LiJianfeng
 * @date 2017年06月27日
 */

#include <jsonrpccpp/common/exception.h>
#include "rpc_seal_server.h"

using namespace std;
using namespace dev::eth;
using namespace dev::rpc;
using namespace jsonrpc;

RPCSealServer::RPCSealServer(Consenter& cons, SessionManager& sm) :
    cons_(cons),session_manager_(sm)
{}

bool RPCSealServer::doMine(const string& session)
{
    if (!session_manager_.hasPrivilegeLevel(session, Privilege::Admin))
        throw jsonrpc::JsonRpcException("Invalid privileges");
    cons_.startPBFT();
    return true;
}

bool RPCSealServer::stopMining(const string& session)
{
    if (!session_manager_.hasPrivilegeLevel(session, Privilege::Admin))
        throw jsonrpc::JsonRpcException("Invalid privileges");
    cons_.stopPBFT();
    return true;
}
