/**
 * @file main.cpp
 * @brief 服务器端主程序
 * @author LiJianfeng
 * @date 2017年06月07日
 */

#include <iostream>
#include <unistd.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <libdevcore/FileSystem.h>
#include <libdevcore/Common.h>
#include <libdevcore/CommonIO.h>
#include <libethcore/BasicAuthority.h>
#include <libethcore/KeyManager.h>
#include <libp2p/Host.h>
#include <libp2p/Network.h>
#include <jsonrpccpp/server/connectors/httpserver.h>
#include <libweb3jsonrpc/AccountHolder.h>
#include <libweb3jsonrpc/Eth.h>
#include <libweb3jsonrpc/SafeHttpServer.h>
#include <libweb3jsonrpc/ModularServer.h>
#include <libweb3jsonrpc/IpcServer.h>
#include <libweb3jsonrpc/LevelDB.h>
//#include <libweb3jsonrpc/Whisper.h>
#include <libweb3jsonrpc/Net.h>
#include <libweb3jsonrpc/Web3.h>
#include <libweb3jsonrpc/AdminNet.h>
#include <libweb3jsonrpc/AdminEth.h>
#include <libweb3jsonrpc/AdminUtils.h>
#include <libweb3jsonrpc/Personal.h>
#include <libweb3jsonrpc/Debug.h>
//#include <libesirpcserver/rpc_seal_server.h>
//#include <libesirpcserver/rpc_net_server.h>
//#include <libesirpcserver/rpc_core_server.h>
#include <libesipbftseal/pbft_client.h>
#include <libesiconsensus/consenter.h>
#include <libesiwebthree/web_three.h>
#include "genesis_info.h"

using namespace std;
using namespace dev;
using namespace dev::eth;
using namespace dev::p2p;
using namespace dev::rpc;
using namespace p2p;
using namespace rpc;
using namespace genesis;
using namespace pbft;
namespace fs = boost::filesystem;

class ExitHandler: public SystemManager
{
public:
    void exit() { exitHandler(0); }
    static void exitHandler(int) {should_exit = true; }
    bool shouldExit() const { return should_exit; }

private:
    static bool should_exit;
};

bool ExitHandler::should_exit = false;

const string ver = "ESI BlockChain V0.1";

int version()
{
    cout << EthRed << ver << " dev. via Eth." << EthReset << endl;
    return 1;
}

int help()
{
    cout << EthRed
        << "--verbosity <0-21>         设置日志等级，默认：8" << endl
        << "--test                     开启本地测试模式" << endl
        << "--n <整数>                 设置共识节点数量，默认：4" << endl
        << "--c <字符串>               指定本共识节点ID" << endl
        << "-p/--peerset <公钥@IP地址:端口号>" << endl
        << "                           连接共识节点" << endl
        << "--public-ip <IP地址>           设置公共网络地址，默认：自动获取" << endl
        << "--listen-ip <IP地址>           监听网络连接地址，默认：0.0.0.0" << endl
        << "--listen-port <端口号>     监听网络连接端口，默认：30303" << endl
        << "--no-upnp                  关闭UPNP" << endl
        << "--rpc-port <端口号>        指定RPC端口，默认：8548" << endl
        << "--rpccorsdomain <域名>     跨域访问" << endl
        << "-j/--json-rpc              启动RPC服务器，提供基础RPC服务" << endl
        << "--util-rpc                 提供工具类RPC服务" << endl
        << "--admin-rpc                提供管理员权限类RPC服务" << endl
        << "--kill-blockchain          删除区块链数据库" << endl
        << "--rebuild-blockchain       重构/恢复区块链数据库" << endl
        << "--rescue-blockchain        修复区块链数据库" << endl
        << "-d/--db-path <目录名>      指定数据库路径" << endl
        << "--config <文件名>          读取JSON格式配置文件，包括创世块配置" << endl
        << "--genesis-config <文件名>  读取JSON格式创世块文件" << endl
        << "-v/--version               查看版本" << endl
        << "-h/--help                  查看帮助" << endl
        << EthReset;
    return 1;
}

int main(int argc, char** argv)
{
    //日志等级
    g_logVerbosity = 8;
    
    bool test_mode = false;
    
    unsigned int node_quantity = 4;//TODO 
    string consensus_id = "";
    map<NodeID, NodeIPEndpoint> nodes;

    string public_ip;
    string listen_ip;
    unsigned short listen_port = 30303;
    bool upnp = true;

    int rpc_port = 8548;
    string rpc_cors_domain = "";
    bool rpc_curl = false;
    bool rpc_util = false;
    bool rpc_admin = false;

    WithExisting we = WithExisting::Trust;
    ChainParams cp(genesis_info);
    string json_config;
    string json_genesis;
    
    for(int i = 1; i < argc; ++i)
    {
        string arg = argv[i];
        if(arg == "--verbosity" && i + 1 < argc)
        {
            g_logVerbosity = atoi(argv[++i]);
        }
        else if(arg == "--test")
        {
            test_mode = true;
        }
        else if (arg == "--n" && i + 1 < argc)
        {
            node_quantity = atoi(argv[++i]);
        }
        else if (arg == "--c" && i + 1 < argc)
        {
            consensus_id = argv[++i];
        }
        else if((arg == "-p" || arg == "--peerset") && i + 1 < argc)
        {
            string pub;
            string host;
            unsigned short port = c_defaultListenPort;
            //key@ip[:port]
            vector<string> key_host_port;
            boost::split(key_host_port, argv[++i], boost::is_any_of(":"));
            if(key_host_port.size()!=2)
            {
                cerr << "参数[" << arg << ": " << argv[i] << "]错误" << endl;
                cerr << "正确示例：\n-p 公钥@IP地址:端口号" << endl;
                return -1;
            }
            port = (uint16_t)atoi(key_host_port[1].c_str());
            vector<string> key_host;
            boost::split(key_host, key_host_port[0], boost::is_any_of("@"));
            if(key_host.size()!=2)
            {
                cerr << "参数[" << arg << ": " << argv[i] << "]错误" << endl;
                cerr << "正确示例：\n-p 公钥@IP地址:端口号" << endl;
                return -1;
            }
            pub = key_host[0];
            if(pub.size() != 128)
            {
                cerr << "参数[" << arg << ": " << argv[i] << "]错误" << endl;
                cerr << "正确示例：\n-p 公钥@IP地址:端口号" << endl;
                return -1;
            }
            host = key_host[1];
            Public public_key(fromHex(pub));
            try
            {
                nodes[public_key] = NodeIPEndpoint(bi::address::from_string(host), port, port);
            }
            catch(...)
            {
                cerr << "参数[" << arg << ": " << argv[i] << "]错误" << endl;
                cerr << "正确示例：\n-p 公钥@IP地址:端口号" << endl;
                return -1; 
            }
        }
        else if(arg == "--public-ip" && i + 1 < argc)
        {
            public_ip = argv[++i];
        }
        else if(arg == "--listen-ip" && i + 1 < argc)
        {
            listen_ip = argv[++i];
        }
        else if(arg == "--listen-port" && i + 1 < argc)
        {
            listen_port = (short)atoi(argv[++i]);
        }
        else if(arg == "--no-upnp")
        {
            upnp = false;
        }
        else if(arg == "--rpc-port" && i + 1 < argc)
        {
            rpc_port = atoi(argv[++i]);
        }
        else if(arg == "--rpccorsdomain" && i + 1 < argc)
        {
            rpc_cors_domain = atoi(argv[++i]);
        }
        else if((arg == "-j" || arg == "--json-rpc"))
        {
            rpc_curl = true;
        }
        else if(arg == "--util-rpc")
        {
            rpc_util = true;
        }
        else if(arg == "--admin-rpc")
        {
            rpc_admin = true;
        }
        else if(arg == "--kill-blockchain")
        {
            we = WithExisting::Kill;
        }
        else if(arg == "--rebuild-blockchain")
        {
            we = WithExisting::Verify;
        }
        else if(arg == "--rescue-blockchain")
        {
            we = WithExisting::Rescue;
        }
        else if((arg == "-d" || arg == "--db-path") && i + 1 < argc)
        {
            setDataDir(argv[++i]);
        }
        else if(arg == "--config" && i + 1 < argc)
        {
            try
            {
                json_config = contentsString(argv[++i]);
            }
            catch(...)
            {
                cerr << "参数[" << arg << ": " << argv[i] << "]错误" << endl;
                return -1;
            }
        }
        else if(arg == "--genesis-config" && i + 1 < argc)
        {
            try
            {
                json_genesis = contentsString(argv[++i]);
            }
            catch(...)
            {
                cerr << "参数[" << arg << ": " << argv[i] << "]错误" << endl;
                return -1;
            }
        }
        else if (arg == "-v" || arg == "--version")
        {
            return version();
        }
        else if (arg == "-h" || arg == "--help")
        {
            version();
            return help();
        }
        else
        {
            cout << "搓搓搓，输入-h查询~_~" << endl;
            return -1;
        }
    }
    if(!test_mode)
    {
        if(consensus_id.empty())
        {
            cout<<"请输入本节点标识(e.g.--c 71)"<<endl;
            return -1;
        }
    }

    //向密封引擎注册商注册
    NoProof::init();
    BasicAuthority::init();

    //重置链操作参数
    if(!json_config.empty())
    {
        try
        {
            cp = cp.loadConfig(json_config);
        }
        catch(...)
        {
            cerr << "指定文件的JSON格式错误" << endl;
            cerr << "正确示例: " << endl << genesis_info_ljf << endl;
            return -1;
        }
    }
    if(!json_genesis.empty())
    {
        try
        {
            cp = cp.loadGenesis(json_genesis);
        }
        catch(...)
        {
            cerr << "指定文件的JSON格式错误" << endl;
            string sample =
            R"E(
            {
                "nonce": "0x0000000000000042",
                "difficulty": "0x400000000",
                "mixHash": "0x0000000000000000000000000000000000000000000000000000000000000000",
                "author": "0x0000000000000000000000000000000000000000",
                "timestamp": "0x00",
                "parentHash": "0x0000000000000000000000000000000000000000000000000000000000000000",
                "extraData": "0x11bbe8db4e347b4e8c937c1c8370e4b5ed33adb3db69cbdb7a38e1e50b1b82fa",
                "gasLimit": "0x1388"
            }
            )E";
            cerr << "正确示例: " << endl << sample << endl;
            return -1;
        }
    }

    //允许未来区块
    cp.allowFutureBlocks = true;

    if(test_mode)
        cp.chainID = -1;

    auto net_prefs = public_ip.empty()? 
        NetworkPreferences(public_ip, listen_ip, listen_port, upnp):
        NetworkPreferences(listen_ip, listen_port, upnp);
    net_prefs.discovery = false;
    net_prefs.pin = true;

    auto hosts_state = contents(getDataDir()/fs::path( "network.rlp"));

    //构造Host、Client、Consenter
    WebThreeConsensus wt(consensus_id, ver, cp, net_prefs, &hosts_state, getDataDir(), we);

    u256 ask_price = DefaultGasPrice;
    u256 bid_price = DefaultGasPrice;
    shared_ptr<eth::TrivialGasPricer> gp = make_shared<eth::TrivialGasPricer>(ask_price, bid_price);
    
    //创建密钥管理器
    fs::path secrets_path;
    if (test_mode)
        secrets_path = (boost::filesystem::path(getDataDir()) / "keystore");
    else
        secrets_path = SecretStore::defaultPath();
    KeyManager km(KeyManager::defaultPath(), secrets_path);
    
    auto get_password = [&](const string& prompt)
    {
        string ret = dev::getPassword(prompt);
        return ret;
    };
    function<string(const Address&)> get_account_password = [&](const Address& a)
    {
        return get_password("请输入账户" + km.accountName(a)
        + "(账户地址: " + a.abridged() + ")的密码(密码提示: " + km.passwordHint(a) + "): ");
    };
    function<bool(const TransactionSkeleton&, bool)> authenticator
        = [](const TransactionSkeleton&, bool) -> bool { return true; };

    //创建账户管理器
    unique_ptr<SimpleAccountHolder> ah(new SimpleAccountHolder(
        [&](){return wt.client();}, get_account_password, km, authenticator));

    unique_ptr<ExitHandler> eh(new ExitHandler());

    //配置RPC服务器端
    unique_ptr<ModularServer<>> rpc_server;
    unique_ptr<SessionManager> sm;
    if(rpc_curl)
    {
        auto *hs = new SafeHttpServer(rpc_port, "", "", SensibleHttpThreads);
        hs->setAllowedOrigin(rpc_cors_domain);

        EthFace* eth = new Eth(*wt.client(), *ah.get());
        NetFace* net = new Net(wt);
        DBFace* db = nullptr;
        Web3Face* w3 = nullptr;
        DebugFace* dbg = nullptr;
        //WhisperFace* wsp = nullptr;
        PersonalFace* per = nullptr;
        AdminEthFace* adm_eth = nullptr;
        AdminNetFace* adm_net = nullptr;
        AdminUtilsFace* adm_utl = nullptr;

        if(test_mode)
            per = new Personal(km, *ah, *wt.client());
        if(rpc_util)
        {
            db = new LevelDB();
            w3 = new Web3(wt.clientVersion());
            dbg = new Debug(*wt.client());
        }
        if(rpc_admin)
        {
            sm.reset(new SessionManager());
            adm_eth = new AdminEth(*wt.client(), *gp.get(), km, *sm.get());
            adm_net = new AdminNet(wt, *sm.get());
            adm_utl = new AdminUtils(*sm.get(), eh.get());
            string session_key;
            session_key = sm->newSession(SessionPermissions{{Privilege::Admin}});
            cout << "@会话密钥：" << session_key << endl;
        }

        rpc_server.reset(new ModularServer<EthFace, NetFace, DBFace, Web3Face,
            DebugFace, PersonalFace, AdminEthFace, AdminNetFace, AdminUtilsFace>
            (eth, net, db, w3, dbg, per, adm_eth, adm_net, adm_utl));
        rpc_server->addConnector(hs);
        rpc_server->StartListening();
        cout << "@RPC服务器端开启监听成功。" << endl;
    }

    //获取PBFT客户端
    PBFTClient* pclient = static_cast<PBFTClient*>(wt.client());
    
    cout << "@区块链高度：" << pclient->getHeight() << endl;
    
    if(test_mode)
    {
        while(!eh->shouldExit())
            pclient->testSealing();
    }
    else
    {
       //开启节点
        wt.startNetwork();
        //本节点的识别地址
        cout << wt.enode() << endl;
 
        for (auto const& p: nodes)
            wt.requirePeer(p.first, p.second);
        
        wt.insertValidator("72");
        wt.insertValidator("71");
 
        //开启共识
        while((wt.peerCount() < node_quantity-1) && !eh->shouldExit())//四个节点启动再开启
            ;
        if(!eh->shouldExit())
        {
            cout << "@开启PBFT" <<endl;
            wt.startPBFT();
        }
        
        while(!eh->shouldExit())
            sleep(1);
    }
    
    if(rpc_server.get())
        rpc_server->StopListening();

    bytes net_data;
    net_data = wt.saveNetwork();
    if (!net_data.empty())
        writeFile(getDataDir()/fs::path("/network.rlp"), net_data);
    
    return 0;
}
