/**
 * @file rpc_core_server.cpp
 * @brief 核心RPC服务器端
 * @author LiJianfeng
 * @date 2017年06月27日
 */

#include <jsonrpccpp/common/exception.h>
#include "libdevcore/CommonJS.h"
#include "libdevcore/FixedHash.h"
#include "libethcore/CommonJS.h"
#include "libethcore/ICAP.h"
#include "libweb3jsonrpc/JsonHelper.h"
#include "rpc_core_server.h"

using namespace std;
using namespace dev;
using namespace dev::eth;
using namespace dev::rpc;
using namespace jsonrpc;

RPCCoreServer::RPCCoreServer(Interface& i, KeyManager& km, SessionManager& sm) :
    client_(i), key_manager_(km), session_manager_(sm)
{}

Json::Value RPCCoreServer::newAccount(const Json::Value& acc,
    const string& session)
{
    if (!session_manager_.hasPrivilegeLevel(session, Privilege::Admin))
        throw jsonrpc::JsonRpcException("Invalid privileges");
    if (!acc.isMember("name"))
        throw jsonrpc::JsonRpcException("No member found: name");
    string name = acc["name"].asString();
    auto s = ICAP::createDirect();
    h128 uuid;
    if (acc.isMember("password"))
    {
        string password = acc["password"].asString();
        string hint = acc["passhint"].asString();
        uuid = key_manager_.import(s, name, password, hint);
    }
    else
        uuid = key_manager_.import(s, name);
    Json::Value ret;
    ret["account"] = toJS(toAddress(s));
    ret["uuid"] = toUUID(uuid);
    return ret;
}

Json::Value RPCCoreServer::listAccountAddresses(const string& session)
{
    if (!session_manager_.hasPrivilegeLevel(session, Privilege::Admin))
        throw jsonrpc::JsonRpcException("Invalid privileges");
    return toJson(key_manager_.accounts());
}

string RPCCoreServer::getBalance(const string& address, const string& session)
{
    if (!session_manager_.hasPrivilegeLevel(session, Privilege::Admin))
        throw jsonrpc::JsonRpcException("Invalid privileges");
    try
    {
        return toJS(client_.balanceAt(jsToAddress(address), client_.number()));
    }
    catch (...)
    {
        BOOST_THROW_EXCEPTION(JsonRpcException(Errors::ERROR_RPC_INVALID_PARAMS));
    }
}

string RPCCoreServer::submitTransaction(const Json::Value& tx,
    const string& pass)
{
    TransactionSkeleton t;
    try
    {
        t = toTransactionSkeleton(tx);
    }
    catch (...)
    {
        BOOST_THROW_EXCEPTION(JsonRpcException(Errors::ERROR_RPC_INVALID_PARAMS));
        return string();
    }
    //Secret s = Secret("acf545ba38f0d5ba6851a1a209765fabc17a6c5f40e8c97e859dff7feede8bfd");
    if (auto s = key_manager_.secret(t.from, [&](){ return pass; }, false))
    {
        return toJS(client_.submitTransaction(t, s).first);
    }
    else
        BOOST_THROW_EXCEPTION(JsonRpcException("Invalid password or account."));
    return string();
}
