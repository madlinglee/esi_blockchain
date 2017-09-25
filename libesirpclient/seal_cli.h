/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file seal_cli.h
 * @brief 密封RPC命令行界面
 * @author LiJianfeng
 * @date 2017年06月12日
 */

#include <iostream>
#include <vector>
#include <jsonrpccpp/client/connectors/httpclient.h>
#include "libdevcore/Terminal.h"
#include "rpc_seal_client.h"

using namespace std;

/**
 * @brief 解析用户输入的命令行参数，并根据模式匹配调用相应远程方法
 */
class SealCLI
{
public:
    enum class OperationMode
    {
        None,
        DoMine,
        SubmitTransaction
    };

    SealCLI(jsonrpc::HttpClient& http_client,
        OperationMode om = OperationMode::None):
        rpc_seal_client_(http_client),
        operation_mode_(om){}

    static void help()
    {
        cout << EthRed << "    doMine" << EthReset
            << " : 开始挖矿" << endl;
        /*
        cout << EthRed << "    submitTransaction" << EthReset
            << " : 提交交易" << endl;
        */
    }

    bool interpretOption(int &i, int argc, char **argv);
    void execute();
    void doMine();
private:
    /**
     * @brief 处理并返回RPC调用时产生的异常
     */
    string handleError(jsonrpc::JsonRpcException e) const
    {
        string error;
        if(e.GetCode() == -32003)//error_ret_CLIENT_CONNECTOR = -32003
        {
            if(e.GetMessage()[39] == '7')//CURLcode == 7
            {
                error = "错误代码：-32003。连接失败。";
            }
            else if(e.GetMessage()[39] == '2' &&
                    e.GetMessage()[40] == '8')//CURLcode == 28
            {
                error = "错误代码：-32003。服务超时。";
            }
        }
        else
        {
            error = "错误代码：" + to_string(e.GetCode());
        }
        return error;
    }

    RPCSealClient rpc_seal_client_;
    OperationMode operation_mode_;
};
