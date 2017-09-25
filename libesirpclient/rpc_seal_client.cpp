/**
 * @file rpc_seal_client.cpp
 * @brief 密封RPC客户端
 * @author LiJianfeng
 * @date 2017年06月27日
 */

#include "rpc_seal_client.h"

bool RPCSealClient::doMine() throw (jsonrpc::JsonRpcException)
{
    Json::Value p = Json::nullValue;
    Json::Value result;
    try
    {
        result = this->CallMethod("doMine", p);
    }
    catch(jsonrpc::JsonRpcException e)
    {
        throw e;
    }
    if (result.isBool())
        return result.asBool();
    else
        throw jsonrpc::JsonRpcException(
                jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE,
                result.toStyledString());
}
