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
#include <libethereum/SnapshotImporter.h>
#include <libethereum/SnapshotStorage.h>
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

/*区块操作模式*/
enum class OperationMode
{
    Node,
    Import,
    ImportSnapshot,
    Export
};

/*区块导出格式*/
enum class Format
{
    Binary,
    Hex,
    Human
};

struct MainChannel: public LogChannel 
{ 
    static const char* name()
    {
        return EthGreen "▉" EthGreen " ▉";
    } 
    static const int verbosity = 1; 
};

/*主程序退出管理器*/
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
    cout << EthRed
        << ver << " dev. via Eth." << endl
        << "网络协议版本：" << dev::eth::c_protocolVersion << endl
        << "数据库版本：" << dev::eth::c_databaseVersion << endl
        << EthReset;
    return 1;
}

int help()
{
    cout << EthMaroon
        << "--verbosity <0-21>          设置日志等级，默认：8" << endl
        << "--test                      开启本地测试模式" << endl
        << "-p/--peerset <公钥@IP地址:端口号>" << endl
        << "                            添加共识节点，需要多次按序输入" << endl
        << "--public-ip <IP地址>        设置公共网络地址，默认：自动获取" << endl
        << "--listen-ip <IP地址>        监听网络连接地址，默认：0.0.0.0" << endl
        << "--listen-port <端口号>      监听网络连接端口，默认：30303" << endl
        << "--no-upnp                   关闭UPNP" << endl
        << "--rpc-port <端口号>         指定RPC端口，默认：8548" << endl
        << "--rpccorsdomain <域名>      跨域访问" << endl
        << "-j/--json-rpc               启动RPC服务器，提供基础RPC服务" << endl
        << "--util-rpc                  提供工具类RPC服务" << endl
        << "--admin-rpc                 提供管理员权限类RPC服务" << endl
        << "--kill-blockchain           删除区块链数据库" << endl
        << "--rebuild-blockchain        重构/恢复区块链数据库" << endl
        << "--rescue-blockchain         修复区块链数据库" << endl
        << "-d/--db-path <目录名>       指定数据库路径，默认：" << getDataDir() << endl
        << "--config <文件名>           读取JSON格式配置文件，包括创世块配置" << endl
        << "--genesis-config <文件名>   读取JSON格式创世块文件" << endl
        << "--master <密码>             指定密钥管理器的主密码，即钱包密码，默认：空" << endl
        << "--password <密码>           缓存密码" << endl
        << "-s/--import-secret <私钥>   导入私钥" << endl
        << "-I/--import <文件名>        导入区块" << endl
        << "-E/--export <文件名>        导出区块" << endl
        << "--import-snapshot <文件名>  导入快照" << endl
        << "--dont-check                取消导入检查" << endl
        << "--from <高度/哈希>          指定导出区块的起始标识，默认：1" << endl
        << "--to <高度/哈希>            指定导出区块的结束标识，默认：latest" << endl
        << "--only <高度/哈希>          指定导出区块的唯一标识" << endl
        << "--format <binary/hex/human> 指定导出区块的格式，默认：binary" << endl
        << "-v/--version                查看版本" << endl
        << "-h/--help                   查看帮助" << endl
        << EthReset;
    return 1;
}

int generateNetworkRlp()
{
    KeyPair kp = KeyPair::create();
    RLPStream net_data(3);
    net_data << dev::p2p::c_protocolVersion << kp.secret().ref();
    int count = 0;
    net_data.appendList(count);

    writeFile(getDataDir()/fs::path("network.rlp"), net_data.out());
    writeFile(getDataDir()/fs::path("network.pub"), kp.pub().hex());
    clog(MainChannel) << "创建新的P2P&&PBFT网络ID/公钥：" << kp.pub().hex();
    return 1;
}

int main(int argc, char** argv)
{
    /*日志等级*/
    g_logVerbosity = 8;
    
    /*本地测试/网络连接*/
    bool test_mode = false;
    
    /*PBFT共识节点、连接列表*/
    map<NodeID, NodeIPEndpoint> nodes;

    /*P2P*/
    string public_ip;
    string listen_ip;
    unsigned short listen_port = 30303;
    bool upnp = true;

    /*RPC*/
    int rpc_port = 8548;
    string rpc_cors_domain = "";
    bool rpc_curl = false;
    bool rpc_util = false;
    bool rpc_admin = false;

    /*钱包/密钥管理器、私钥与密码*/
    string master_password;
    bool master_set = false;
    strings passwords_to_note;
    Secrets to_import;

    /*区块导入与导出*/
    OperationMode mode = OperationMode::Node;
    string file_name;
    bool safe_import = false;//导入时需要验证
    string export_from = "1";
    string export_to = "latest";
    Format export_format = Format::Binary;

    /*链操作参数*/
    WithExisting we = WithExisting::Trust;
    ChainParams cp(genesis_info);
    string json_config;
    string json_genesis;
    
    /*解析CLI/命令行参数*/
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
        else if (arg == "--master" && i + 1 < argc)
        {
            master_password = argv[++i];
            master_set = true;
        }
        else if (arg == "--password" && i + 1 < argc)
        {
            passwords_to_note.push_back(argv[++i]);
        }
        else if ((arg == "-s" || arg == "--import-secret") && i + 1 < argc)
        {
            Secret s(fromHex(argv[++i]));
            to_import.emplace_back(s);
        }
        else if ((arg == "-I" || arg == "--import") && i + 1 < argc)
        {
            mode = OperationMode::Import;
            file_name = argv[++i];
        }
        else if ((arg == "-E" || arg == "--export") && i + 1 < argc)
        {
            mode = OperationMode::Export;
            file_name = argv[++i];
        }
        else if ((arg == "--import-snapshot") && i + 1 < argc)
        {
            mode = OperationMode::ImportSnapshot;
            file_name = argv[++i];
        }
        else if (arg == "--dont-check")
        {
            safe_import = true;
        }
        else if (arg == "--to" && i + 1 < argc)
        {    
            export_to = argv[++i];
        }
        else if (arg == "--from" && i + 1 < argc)
        {
            export_from = argv[++i];
        }
        else if (arg == "--only" && i + 1 < argc)
        {
            export_to = export_from = argv[++i];
        }
        else if (arg == "--format" && i + 1 < argc)
        {
            string m = argv[++i];
            if (m == "binary")
                export_format = Format::Binary;
            else if (m == "hex")
                export_format = Format::Hex;
            else if (m == "human")
                export_format = Format::Human;
            else
            {
                cerr << "参数[" << arg << ": " << m << "]错误" << endl;
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
            cout << endl;
            return help();
        }
        else
        {
            cout << "搓搓搓，输入-h查询~_~" << endl;
            return -1;
        }
    }

    /*注册密封引擎*/
    NoProof::init();
    BasicAuthority::init();

    /*重置链操作参数*/
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

    /*允许未来区块*/
    cp.allowFutureBlocks = true;

    if(test_mode)
        cp.chainID = -1;

    u256 ask_price = DefaultGasPrice;
    u256 bid_price = DefaultGasPrice;
    shared_ptr<eth::TrivialGasPricer> gp = make_shared<eth::TrivialGasPricer>(ask_price, bid_price);

    /*构造Host、Client、Consenter*/
    auto net_prefs = public_ip.empty()? 
        NetworkPreferences(public_ip, listen_ip, listen_port, upnp):
        NetworkPreferences(listen_ip, listen_port, upnp);
    net_prefs.discovery = false;
    net_prefs.pin = true;
    auto hosts_state = contents(getDataDir()/fs::path("network.rlp"));

    WebThreeConsensus wt(ver, cp, net_prefs, &hosts_state, getDataDir(), we);

    /*区块导入与导出*/
    auto to_number = [&](string const& s) -> unsigned {
        if (s == "latest")
            return wt.client()->number();
        if (s.size() == 64 || (s.size() == 66 && s.substr(0, 2) == "0x"))
            return wt.client()->blockChain().number(h256(s));
        try {
            return stol(s);
        }
        catch (...)
        {
            cerr << "区块高度/哈希错误：" << s << "\n";
            return -1;
        }
    };
    if (mode == OperationMode::Export)
    {
        ofstream fout(file_name, std::ofstream::binary);
        ostream& out = (file_name.empty() || file_name == "--") ? cout : fout;

        unsigned last = to_number(export_to);
        for (unsigned i = to_number(export_from); i <= last; ++i)
        {
            bytes block = wt.client()->blockChain().block(wt.client()->blockChain().numberHash(i));
            switch (export_format)
            {
                case Format::Binary: out.write((char const*)block.data(), block.size()); break;
                case Format::Hex: out << toHex(block) << "\n"; break;
                case Format::Human: out << RLP(block) << "\n"; break;
                default:;
            }
        }
        return 0;
    }
    if (mode == OperationMode::Import)
    {
        ifstream fin(file_name, std::ifstream::binary);
        istream& in = (file_name.empty() || file_name == "--") ? cin : fin;
        unsigned already_have = 0;
        unsigned good = 0;
        unsigned future_time = 0;
        unsigned unknown_parent = 0;
        unsigned bad = 0;
        chrono::steady_clock::time_point t = chrono::steady_clock::now();
        double last = 0;
        unsigned last_imported = 0;
        unsigned imported = 0;
        while (in.peek() != -1)
        {
            bytes block(8);
            in.read((char*)block.data(), 8);
            block.resize(RLP(block, RLP::LaissezFaire).actualSize());
            in.read((char*)block.data() + 8, block.size() - 8);

            switch (wt.client()->queueBlock(block, safe_import))
            {
                case ImportResult::Success: good++; break;
                case ImportResult::AlreadyKnown: already_have++; break;
                case ImportResult::UnknownParent: unknown_parent++; break;
                case ImportResult::FutureTimeUnknown: unknown_parent++; future_time++; break;
                case ImportResult::FutureTimeKnown: future_time++; break;
                default: bad++; break;
            }

            // sync chain with queue
            tuple<ImportRoute, bool, unsigned> r = wt.client()->syncQueue(10);
            imported += get<2>(r);

            double e = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - t).count() / 1000.0;
            if ((unsigned)e >= last + 10)
            {
                auto i = imported - last_imported;
                auto d = e - last;
                clog(MainChannel) << "较上次多导入" << i << "块，速度：" << (round(i * 10 / d) / 10) << " 块/s，本次共" << imported << "块导入，用时：" << e << "s，速度：" << (round(imported * 10 / e) / 10) << "块/s (#" << wt.client()->number() << ")";
                last = (unsigned)e;
                last_imported = imported;
            }
        }
        bool more_to_import = true;
        while (more_to_import)
        {
            this_thread::sleep_for(chrono::seconds(1));
            tie(ignore, more_to_import, ignore) = wt.client()->syncQueue(100000);
        }
        double e = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - t).count() / 1000.0;
        clog(MainChannel) << imported << "块导入用时：" << e << "s，速度：" << (round(imported * 10 / e) / 10) << "块/s (#" << wt.client()->number() << ")";
        return 0;
    }
    if (mode == OperationMode::ImportSnapshot)
    {
        try
        {
            auto state_importer = wt.client()->createStateImporter();
            auto block_chain_importer = wt.client()->createBlockChainImporter();
            SnapshotImporter importer(*state_importer, *block_chain_importer);

            auto snapshot_storage(createSnapshotStorage(file_name));
            importer.import(*snapshot_storage);
            // continue with regular sync from the snapshot block
        }
        catch (...)
        {
            cerr << "导入快照出错：" << boost::current_exception_diagnostic_information() << endl;
            return -1;
        }
    }

    /*创建密钥管理器*/
    fs::path secrets_path;
    if (test_mode)
        secrets_path = (boost::filesystem::path(getDataDir()) / "keystore");
    else
        secrets_path = SecretStore::defaultPath();
    KeyManager km(KeyManager::defaultPath(), secrets_path);
    for (auto const& s: passwords_to_note)
        km.notePassword(s);
    try
    {
        if (km.exists())
        {
            //if (!km.load(master_password) && master_set)
            if (!km.load(master_password))//主密码为空，也必须输入实际密码
            {
                short ct = 0;
                while (ct<3)
                {
                    master_password = getPassword("请输入钱包/主密码：");
                    if (km.load(master_password))
                        break;
                    ct++;
                    if(ct!=3)
                        cout << "密码错误"<<"\n";
                    else
                    {
                        cout << "密码错误，若想强制启动，可以选择手动删除钱包文件：" << (getDataDir("ethereum") / fs::path("keys.info")).string() << "\n";
                        return -1;
                    }
                }
            }
        }
        else
        {
            if (master_set)
                km.create(master_password);
            else
                km.create(std::string());
        }
    }
    catch(...)
    {
        cerr << "密钥管理器初始化异常：" << boost::current_exception_diagnostic_information() << "\n";
        return -1;
    }
    for (auto const& s: to_import)
    {
        km.import(s, "via CLI");
    }
    
    auto get_password = [&](const string& prompt)
    {
        string ret = dev::getPassword(prompt);
        return ret;
    };
    function<string(const Address&)> get_account_password = [&](const Address& a)
    {
        return get_password("请输入账户" + km.accountName(a)
        + "(账户地址: " + a.abridged() + ")密码(密码提示: " + km.passwordHint(a) + "): ");
    };
    function<bool(const TransactionSkeleton&, bool)> authenticator
        = [](const TransactionSkeleton&, bool) -> bool { return true; };

    unique_ptr<SimpleAccountHolder> ah(new SimpleAccountHolder(
        [&](){return wt.client();}, get_account_password, km, authenticator));

    unique_ptr<ExitHandler> eh(new ExitHandler());

    /*配置RPC服务器端*/
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
            clog(MainChannel) << "会话密钥：" << session_key;
        }

        rpc_server.reset(new ModularServer<EthFace, NetFace, DBFace, Web3Face,
            DebugFace, PersonalFace, AdminEthFace, AdminNetFace, AdminUtilsFace>
            (eth, net, db, w3, dbg, per, adm_eth, adm_net, adm_utl));
        rpc_server->addConnector(hs);
        rpc_server->StartListening();
        clog(MainChannel) << "RPC服务器端监听成功";
    }

    /*获取PBFT客户端并启动挖矿*/
    PBFTClient* pclient = static_cast<PBFTClient*>(wt.client());
    
    clog(MainChannel) << "区块链高度：" << pclient->getHeight();
    
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
        clog(MainChannel)<< wt.enode();
 
        for (auto const& p: nodes)
        {
            wt.insertValidator(p.first.abridged(), (Public)p.first);
            if(p.first == wt.id())
                continue;
            wt.requirePeer(p.first, p.second);
        }
        
        clog(MainChannel) << "PBFT共识节点总数：" << nodes.size();
        clog(MainChannel) << "等待节点连接";
        //开启共识
        while((wt.peerCount() < nodes.size()-1) && !eh->shouldExit())//所有共识节点启动再开启
            ;
        if(!eh->shouldExit())
        {
            clog(MainChannel) << "开启PBFT";
            wt.startPBFT();
        }
        
        while(!eh->shouldExit())
            sleep(1);
    }
    
    /*停止RPC服务*/
    if(rpc_server.get())
        rpc_server->StopListening();

    return 0;
}
