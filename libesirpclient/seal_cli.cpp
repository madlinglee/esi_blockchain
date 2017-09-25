/**
 * @file seal_cli.cpp
 * @brief 密封RPC命令行界面
 * @author LiJianfeng
 * @date 2017年06月27日
 */

#include "seal_cli.h"

bool SealCLI::interpretOption(int &i, int argc, char **argv)
{
    string arg = argv[i];//第一次传入i==1
    if(arg == "doMine" || arg == "domine" || arg == "dm")
    {
        operation_mode_ = OperationMode::DoMine;
    }
    /*
    else if(arg == "submitTransaction" || arg == "submittransaction" ||
            arg == "st")
    {
        operation_mode_ = OperationMode::SubmitTransaction;
    }
    */
    else
        return false;
    return true;
}

void SealCLI::execute()
{
    switch(operation_mode_){
        case OperationMode::DoMine :
            {
                doMine();
                break;
            }
        /*
        case OperationMode::SubmitTransaction :
            {
                break;
            }
        */
        default:
            break;

    }
}

void SealCLI::doMine()
{
    try
    {
        rpc_seal_client_.doMine();
    }
    catch(jsonrpc::JsonRpcException e)
    {
        cerr << handleError(e) << endl;
    }
}
