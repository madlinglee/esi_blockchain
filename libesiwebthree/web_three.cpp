/**
 * @file web_three.cpp
 * @brief webthree
 * @author LiJianfeng
 * @date 2017年09月28日
 */

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <libethereum/Defaults.h>
#include <libethereum/EthereumHost.h>
#include <libesipbftseal/pbft_client.h>
#include "BuildInfo.h"
#include "web_three.h"

using namespace std;
using namespace dev;
using namespace dev::p2p;
using namespace dev::eth;
using namespace dev::shh;
using namespace pbft;

WebThreeConsensus::WebThreeConsensus(
    const std::string& consensus_id,
	const std::string& client_version,
    PBFTClient* client,
    p2p::Host& host,
	const boost::filesystem::path& db
):Consenter(consensus_id, client),
	client_version_(client_version),
	net_(host)
{
	if (db.size())
		Defaults::setDBPath(db);
    NoProof::init();
}

WebThreeConsensus::~WebThreeConsensus()
{
	net_.stop();
}

std::string WebThreeConsensus::composeClientVersion(std::string const& client)
{
	return client + "/" + \
		"v" + dev::Version + "/" + \
		DEV_QUOTED(ETH_BUILD_OS) + "/" + \
		DEV_QUOTED(ETH_BUILD_COMPILER) + "/" + \
		DEV_QUOTED(ETH_BUILD_JIT_MODE) + "/" + \
		DEV_QUOTED(ETH_BUILD_TYPE) + "/" + \
		string(DEV_QUOTED(ETH_COMMIT_HASH)).substr(0, 8) + \
		(ETH_CLEAN_REPO ? "" : "*") + "/";
}

p2p::NetworkPreferences const& WebThreeConsensus::networkPreferences() const
{
	return net_.networkPreferences();
}

void WebThreeConsensus::setNetworkPreferences(const p2p::NetworkPreferences& n, bool drop)
{
	auto had = isNetworkStarted();
	if (had)
		stopNetwork();
	net_.setNetworkPreferences(n, drop);
	if (had)
		startNetwork();
}

std::vector<PeerSessionInfo> WebThreeConsensus::peers()
{
	return net_.peerSessionInfo();
}

size_t WebThreeConsensus::peerCount() const
{
	return net_.peerCount();
}

void WebThreeConsensus::setIdealPeerCount(size_t n)
{
	return net_.setIdealPeerCount(n);
}

void WebThreeConsensus::setPeerStretch(size_t n)
{
	return net_.setPeerStretch(n);
}

bytes WebThreeConsensus::saveNetwork()
{
	return net_.saveNetwork();
}

void WebThreeConsensus::addNode(const NodeID& node, const bi::tcp::endpoint& host)
{
	net_.addNode(node, NodeIPEndpoint(host.address(), host.port(), host.port()));
}

void WebThreeConsensus::requirePeer(const NodeID& node, const bi::tcp::endpoint& host)
{
	net_.requirePeer(node, NodeIPEndpoint(host.address(), host.port(), host.port()));
}

void WebThreeConsensus::addPeer(const NodeSpec& s, PeerType t)
{
	net_.addPeer(s, t);
}
