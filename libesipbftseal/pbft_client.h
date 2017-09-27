/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file pbft_client.h
 * @brief BPFT客户端
 * @author LiJianfeng
 * @date 2017年06月26日
 */

#pragma once

#include <boost/filesystem/path.hpp>
#include <libethereum/EthereumHost.h>
#include <libethereum/Client.h>
#include <libesipbft/pbft_interface.h>

using namespace dev;
using namespace dev::eth;

namespace pbft{

class PBFTClient : public Client, public PBFTInterface
{
public:
    PBFTClient(
        const ChainParams& params,
        int network_id,
        p2p::Host* host,
        std::shared_ptr<GasPricer> gp,
        const boost::filesystem::path& db_path = std::string(),
        WithExisting we = WithExisting::Trust,
        const TransactionQueue::Limits& l = TransactionQueue::Limits{102400, 102400}
    );

    /**
     * @brief 测试创建、验证、提交区块
     */
    void testSealing();
    
    /**
     * @brief 开启密封，空实现（当前由PBFT状态机控制）
     */
    void startSealing() override 
    {}

    /**
     * @brief 同步区块、交易等
     */
    void doWork() override;

    /**
     * @brief 同步区块后过滤器更新
     *
     * @param[in] blocks 入链区块哈希列表
     * @param[out] changed 过滤器
     */
    void onNewBlocks(const h256s& blocks, h256Hash& changed) override;

    /**
     * @brief 通知广播交易
     */
    void onTransactionQueueReady() override;

    /**
     * @brief 同步交易队列
     */
    void syncTransactionQueue() override;

    /**
     * @brief 产生共识数据，即创建区块
     *
     * @return 区块字节数组
     */
    bytes createConsensusData() override;

    /**
     * @brief 验证区块
     *
     * @param[in] block 区块字节数组
     *
     * @return true 是通过 false 是失败
     */
    bool verify(const bytes& block) override;

    /**
     * @brief 共识提过提交，即区块入链及状态更新
     *
     * @param[in] block 区块字节数组
     *
     * @return true 是成功 false 是失败
     */
    bool commit(const bytes& block) override;

    /**
     * @brief 获取交易队列中当前交易数量
     *
     * @return 交易数量
     */
    long getTxNum() override;

    /**
     * @brief 获取当前区块链高度
     *
     * @return 区块链高度
     */
    long getHeight() override;

    /**
     * @brief 区块入链后的状态同步
     *
     * @param[in] ir 入链涉及的区块和交易
     */
    void syncStateAfterImport(const ImportRoute& ir);

    /**
     * @brief 广播PBFT消息给其它节点
     *
     * @param[in] msg 消息字节数组
     */
    void broadcastPBFTMsgs(const bytes& msg);

    /**
     * @brief 获取从其它节点接收的PBFT消息
     *
     * @return 所有消息字节数组
     */
    std::vector<bytes> getRecvMsgs()
    {
       return host()->getRecvMsgs();
    }

    EthereumHost* host()
    {
        if(auto h = m_host.lock())
            return h.get();
        BOOST_THROW_EXCEPTION(NoNetworking());
    }
private:
    bool sealCurrentBlockWithLock(const BlockHeader& bi);
};

}
