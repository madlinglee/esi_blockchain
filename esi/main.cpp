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
#include <jsonrpccpp/server/connectors/httpserver.h>
//#include <libesirpcserver/rpc_seal_server.h>
//#include <libesirpcserver/rpc_net_server.h>
//#include <libesirpcserver/rpc_core_server.h>
#include <libesipbftseal/pbft_client.h>
#include <libesiconsensus/consenter.h>
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
    string consensus_id;
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
            else if(arg == "--p" && i + 3 < argc)
            {
                peer_ips_ports[0] = argv[++i];
                peer_ips_ports[1] = argv[++i];
                peer_ips_ports[2] = argv[++i];
            }
            else if(arg == "--e" && i + 3 < argc)
            {
                peer_enodes[0] = argv[++i];
                peer_enodes[1] = argv[++i];
                peer_enodes[2] = argv[++i];
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
    g_logVerbosity = 8;
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

    //构造Host
    Host host("TESTv1.0", net_prefs, &hosts_state);

    //构造client
    unique_ptr<PBFTClient> client(new PBFTClient(cp, (int)cp.networkID, &host, shared_ptr<GasPricer>(), getDataDir(), we));
    //设置区块地址
    //client->setAuthor(toAddress(secret_ljf));
    //cout << "@区块地址：" << client->author() << endl;
    /*
    //BasicAuthority引擎设置区块签名私钥与验证地址
    client->setSealOption("authority", rlp(secret_ljf.makeInsecure()));
    client->setSealOption("authorities", rlpList(toAddress(secret_ljf)));
    */

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
        [&](){return client.get();}, getAccountPassword, km, authenticator));

    EthFace* eth = new Eth(*client.get(), *ah.get());
    PersonalFace* per = new Personal(km, *ah, *client.get());
    /* RPCCoreServerFace* cs = new RPCCoreServer(*client.get(), km, *sm.get());
    RPCSealServerFace* ss = new RPCSealServer(consenter, *sm.get());
    RPCNetServerFace* ns = new RPCNetServer(host, *sm.get());

    rpc_server.reset(new ModularServer<EthFace, RPCSealServerFace, RPCNetServerFace,
        RPCCoreServerFace>(eth, ss, ns, cs));
    */
    rpc_server.reset(new ModularServer<EthFace, PersonalFace>(eth, per));
    rpc_server->addConnector(hs);
    rpc_server->StartListening();
    cout << "@RPC服务器端开启监听成功。" << endl;
    string session_key;
    session_key = sm->newSession(SessionPermissions{{Privilege::Admin}});
    cout << "@会话密钥：" << session_key << endl;
    
    //区块链高度
    cout << "@区块链高度：" << client->getHeight() << endl;
    if(test_mode)
    {
        while(1)
        {
            sleep(1);
            client->testSealing();
        }
    }
    else
    {
       //开启节点
        host.start();
        //本节点的识别地址
        cout << host.enode() << endl;
 
        if(peer_ips_ports[0]!="")
        {
            for(int x=0; x<3; x++)
            {
                bi::tcp::endpoint endpoint = dev::p2p::Network::resolveHost(peer_ips_ports[x]);
                host.requirePeer(NodeID(peer_enodes[x]), 
                NodeIPEndpoint(endpoint.address(), endpoint.port(), endpoint.port()));
            }
            auto netData = host.saveNetwork();
            if (!netData.empty())
                writeFile(getDataDir()/fs::path("/network.rlp"), netData);
        }
 
        //共识
        Consenter consenter(consensus_id, client.get());
        consenter.insertValidator("72");
        consenter.insertValidator("71");
        consenter.insertValidator("91");
        consenter.insertValidator("92");
 
 
        //开启共识
        while(host.peerCount() < 3)//四个节点启动再开启
                ;
        cout << "@开启PBFT" <<endl;
        consenter.startPBFT();
 
        while(true);
    }

    return 0;
}
