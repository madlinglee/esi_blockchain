/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file web_three.h
 * @brief webthree
 * @author LiJianfeng
 * @date 2017年09月28日
 */

#pragma once

#include <thread>
#include <mutex>
#include <list>
#include <atomic>
#include <boost/asio.hpp> 
#include <boost/utility.hpp>
#include <libdevcore/Common.h>
#include <libdevcore/CommonIO.h>
#include <libdevcore/Guards.h>
#include <libdevcore/Exceptions.h>
#include <libp2p/Host.h>
#include <libwhisper/WhisperHost.h>
#include <libethereum/Client.h>
#include <libethereum/ChainParams.h>
#include <libesiconsensus/consenter.h>
#include <libwebthree/WebThree.h>

using namespace dev;

class WebThreeConsensus: public Consenter, public NetworkFace
{
public:
	WebThreeConsensus(
        const std::string& consensus_id,
		const std::string& client_version,
        PBFTClient* client,
        p2p::Host& host,
		const boost::filesystem::path& db
	);

	~WebThreeConsensus();

    void insertValidator(const std::string& name){Consenter::insertValidator(name);}
    void startPBFT(){Consenter::startPBFT();}

	static std::string composeClientVersion(const std::string& client);
	std::string const& clientVersion() const { return client_version_; }

	std::vector<p2p::PeerSessionInfo> peers() override;

	size_t peerCount() const override;
	
	virtual void addPeer(const p2p::NodeSpec& node, p2p::PeerType t) override;

	virtual void addNode(const p2p::NodeID& node, const bi::tcp::endpoint& endpoint) override;

	void addNode(const p2p::NodeID& node, const std::string& host) { addNode(node, p2p::Network::resolveHost(host)); }
	
	void addNode(const bi::tcp::endpoint& endpoint) { addNode(p2p::NodeID(), endpoint); }

	void addNode(const std::string& _hostString) { addNode(p2p::NodeID(), _hostString); }
	
	void requirePeer(const p2p::NodeID& node, const bi::tcp::endpoint& endpoint) override;

	void requirePeer(const p2p::NodeID& node, const std::string& host) { requirePeer(node, p2p::Network::resolveHost(host)); }

	dev::bytes saveNetwork() override;

	void setIdealPeerCount(size_t n) override;

	void setPeerStretch(size_t n);
	
	bool haveNetwork() const override { return net_.haveNetwork(); }

	p2p::NetworkPreferences const& networkPreferences() const override;

	void setNetworkPreferences(const p2p::NetworkPreferences& n, bool drop = false) override;

	p2p::NodeInfo nodeInfo() const override { return net_.nodeInfo(); }

	p2p::NodeID id() const override { return net_.id(); }

	u256 networkId() const override { return Consenter::client()->networkId(); }

	std::string enode() const override { return net_.enode(); }

	p2p::Peers nodes() const override { return net_.getPeers(); }

	void startNetwork() override { net_.start(); }

	void stopNetwork() override { net_.stop(); }

	bool isNetworkStarted() const override { return net_.isStarted(); }

private:
	std::string client_version_;

	p2p::Host& net_;		
};
