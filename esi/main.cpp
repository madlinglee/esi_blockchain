/**
 * @file main.cpp
 * @brief 服务器端主程序
 * @author LiJianfeng
 * @date 2017年06月07日
 */

#include <iostream>
#include <unistd.h>
#include <boost/filesystem.hpp>
#include <libdevcore/FileSystem.h>
#include <libdevcore/Common.h>
#include <libdevcore/CommonIO.h>
#include <libethcore/BasicAuthority.h>
#include <libethcore/KeyManager.h>
#include <libp2p/Host.h>
#include <libp2p/Network.h>
#include <libweb3jsonrpc/SessionManager.h>
#include <libweb3jsonrpc/AccountHolder.h>
#include <libweb3jsonrpc/EthFace.h>
#include <libweb3jsonrpc/Eth.h>
#include <libweb3jsonrpc/PersonalFace.h>
#include <libweb3jsonrpc/Personal.h>
#include <libweb3jsonrpc/NetFace.h>
#include <libweb3jsonrpc/Net.h>
#include <jsonrpccpp/server/connectors/httpserver.h>
//#include <libesirpcserver/rpc_seal_server.h>
//#include <libesirpcserver/rpc_net_server.h>
//#include <libesirpcserver/rpc_core_server.h>
#include <libesipbftseal/pbft_client.h>
#include <libesiconsensus/consenter.h>
#include <libesiwebthree/web_three.h>
#include "genesis_info.h"

namespace fs = boost::filesystem;
using namespace std;
using namespace dev;
using namespace dev::eth;
using namespace dev::p2p;
using namespace dev::rpc;
using namespace p2p;
using namespace rpc;
using namespace genesis;
using namespace pbft;

void version()
{
        cout << "RPC CLI v1.0" << endl;
}

int main(int argc, char** argv)
{
    bool test_mode = false;
    string consensus_id = "";
    string peer_ips_ports[3];
    string peer_enodes[3];
    if(argc==2 && ((string(argv[1])=="--help") || (string(argv[1])=="--h")))
    {
        cout << "--c id(必输项，e.g.71)" << endl;
        cout << "--p ip1:port(e.g.10.10.200.71:30303) ip2:port2 ip3:port3" << endl;
        cout << "--e enode1(e.g.413852e5c4eed59edb37b3a34cba6fc69dc3fec8b57cce49458ffc7c7de94928a53c9c60efe1b4383132ec7f7649fbb0e814a839f7516036a0dfa84b9b938d41) enode2 enode3" << endl;
        return 1;
    }
    else
    {
        for(int i = 1; i < argc; ++i)
        {
            string arg = argv[i];
            if (arg == "--c" && i + 1 < argc)
                consensus_id = argv[++i];
            else if(arg == "--p" && i + 1 < argc)
            {
                peer_ips_ports[0] = argv[++i];
            }
            else if(arg == "--e" && i + 1 < argc)
            {
                peer_enodes[0] = argv[++i];
            }
            else if(arg == "--test")
            {
                test_mode = true;
            }
            else
            {
                cout << "搓搓搓，可以--h查询~_~" << endl;
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
            if((peer_ips_ports[0]=="" && peer_enodes[0]!="")
                || (peer_ips_ports[0]!="" && peer_enodes[0]==""))
            {
                cout<<"请同时指定--p与--e"<<endl;
                return -1;
            }
        }
    }
    g_logVerbosity = 10;
    //向密封引擎注册商注册
    NoProof::init();
    BasicAuthority::init();
    //构造指定json字符串的链参数
    ChainParams cp(genesis_info_ljf);
    //ChainParams cp(genesisInfo(eth::Network::Frontier), genesisStateRoot(eth::Network::Frontier));
    cp.allowFutureBlocks = true;
    if(test_mode)
        cp.chainID = -1;

    WithExisting we = WithExisting::Trust;

    string listenIP;
    unsigned short listenPort = 30303;
    bool upnp = false;
    auto net_prefs = NetworkPreferences(listenIP, listenPort, upnp);
    net_prefs.discovery = false;
    net_prefs.pin = true;

    auto hosts_state = contents(getDataDir()/fs::path( "network.rlp"));

    //构造Host、Client、Consenter
    WebThreeConsensus wt(consensus_id, "ESI BLCOKCHAIN V0.1", cp, net_prefs, &hosts_state, getDataDir());

    //RPC服务器端
    jsonrpc::HttpServer *hs = new jsonrpc::HttpServer(8548, "", "", 4);
    unique_ptr<ModularServer<>> rpc_server;

    KeyManager km;
    unique_ptr<SessionManager> sm;
    sm.reset(new SessionManager());
    auto getPassword = [&](const string& prompt)
    {
        string ret = dev::getPassword(prompt);
        return ret;
    };
    function<string(const Address&)> getAccountPassword = [&](const Address& a)
    {
        return getPassword("Enter password for address " + km.accountName(a)
        + " (" + a.abridged() + "; hint:" + km.passwordHint(a) + "): ");
    };
    function<bool(const TransactionSkeleton&, bool)> authenticator
        = [](const TransactionSkeleton&, bool) -> bool { return true; };
    unique_ptr<SimpleAccountHolder> ah(new SimpleAccountHolder(
        [&](){return wt.client();}, getAccountPassword, km, authenticator));

    EthFace* eth = new Eth(*wt.client(), *ah.get());
    PersonalFace* per = new Personal(km, *ah, *wt.client());
    NetFace* net = new Net(wt);

    rpc_server.reset(new ModularServer<EthFace, PersonalFace, NetFace>(eth, per, net));
    rpc_server->addConnector(hs);
    rpc_server->StartListening();
    
    cout << "@RPC服务器端开启监听成功。" << endl;
    string session_key;
    session_key = sm->newSession(SessionPermissions{{Privilege::Admin}});
    cout << "@会话密钥：" << session_key << endl;
    
    PBFTClient* pclient = static_cast<PBFTClient*>(wt.client());
    //区块链高度
    cout << "@区块链高度：" << pclient->getHeight() << endl;
    if(test_mode)
    {
        while(1)
        {
            sleep(1);
            pclient->testSealing();
        }
    }
    else
    {
       //开启节点
        wt.startNetwork();
        //本节点的识别地址
        cout << wt.enode() << endl;
 
        if(peer_ips_ports[0]!="")
        {
            for(int x=0; x<1; x++)
            {
                h512 e(peer_enodes[x]);
                wt.requirePeer(e, peer_ips_ports[x]);
            }
        }
        auto netData = wt.saveNetwork();
        if (!netData.empty())
            writeFile(getDataDir()/fs::path("/network.rlp"), netData);
        
        wt.insertValidator("72");
        wt.insertValidator("71");
 
 
        //开启共识
        while(wt.peerCount() < 1)//四个节点启动再开启
                ;
        cout << "@开启PBFT" <<endl;
        wt.startPBFT();
 
        while(true);
    }

    return 0;
}
