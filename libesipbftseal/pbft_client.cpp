/**
 * @file pbft_client.cpp
 * @brief PBFT客户端
 * @author LiJianfeng
 * @date 2017年06月26日
 */

#include <iostream>
#include "pbft_client.h"

using namespace std;
using namespace dev::eth;
using namespace pbft;

struct ClientWarn: public LogChannel
{ 
    static const char* name()
    {
        return EthYellow "⧫" EthBlue " !";
    }
    static const int verbosity = 8; 
};

struct ClientError: public LogChannel
{ 
    static const char* name()
    {
        return EthYellow "⧫" EthRed " ✘";
    }
    static const int verbosity = 8; 
};

PBFTClient::PBFTClient(
        const ChainParams& params,
        int network_id,
        p2p::Host* host,
        std::shared_ptr<GasPricer> gp,
        const boost::filesystem::path& db_path,
        WithExisting we,
        const TransactionQueue::Limits& l
    ) : Client(params, network_id, host, gp, db_path, we, l)
{}

void PBFTClient::doWork()
{
    bool t = true;
    if (m_syncBlockQueue.compare_exchange_strong(t, false))
        syncBlockQueue();//同步本地区块，而不广播出去

    if (m_needStateReset)
    {
        resetState();
        m_needStateReset = false;
    }

    tick();

    callQueuedFunctions();

    if (!m_syncBlockQueue)
    {
        std::unique_lock<std::mutex> l(x_signalled);
        m_signalled.wait_for(l, chrono::seconds(6));
    }
}

void PBFTClient::onNewBlocks(const h256s& blocks, h256Hash& changed)
{//取消了通知EthereumHost广播
    for (auto const& h: blocks)
        clog(ClientTrace) << "Live block:" << h;

    for (auto const& h: blocks)
        appendFromBlock(h, BlockPolarity::Live, changed);
}

void PBFTClient::onTransactionQueueReady()
{//取消notify，增加通知广播交易
    m_syncTransactionQueue = true;
    if(auto h = m_host.lock())
        h->noteNewTransactions();
}

void PBFTClient::syncTransactionQueue()
{//取消通知广播交易
    Timer timer;

    h256Hash changeds;
    TransactionReceipts new_pending_receipts;

    DEV_WRITE_GUARDED(x_working)
    {
        if (m_working.isSealed())
        {
            clog(ClientWarn) << "Skipping txq sync for a sealed block.";
            return;
        }

        tie(new_pending_receipts, m_syncTransactionQueue) = m_working.sync(bc(), m_tq, *m_gp);
    }

    if (new_pending_receipts.empty())
    {
        auto s = m_tq.status();
        clog(ClientDetail) << "No transactions to process. " << m_working.pending().size() << " pending, " << s.current << " queued, " << s.future << " future, " << s.unverified << " unverified";
        return;
    }

    DEV_READ_GUARDED(x_working)
        DEV_WRITE_GUARDED(x_postSeal)
            m_postSeal = m_working;

    DEV_READ_GUARDED(x_postSeal)
        for (size_t i = 0; i < new_pending_receipts.size(); i++)
            appendFromNewPending(new_pending_receipts[i], changeds, m_postSeal.pending()[i].sha3());

    // Tell farm about new transaction (i.e. restart mining).
    onPostStateChanged();

    // Tell watches about the new transactions.
    noteChanged(changeds);

    clog(ClientDetail) << "Processed " << new_pending_receipts.size()
        << " transactions in" << (timer.elapsed() * 1000)
        << "ms (" << (bool)m_syncTransactionQueue << ")";
    clog(ClientDetail) << "#交易处理速度："
        << ((new_pending_receipts.size()*1000)/((timer.elapsed()*1000000)/1000))
        << "笔/s";
}

bool PBFTClient::sealCurrentBlockWithLock(const BlockHeader& bi)
{
    RLPStream s;
    bi.streamRLP(s);
    if(m_working.sealBlock(s.out()))
        return true;
    else
        return false;
}

bytes PBFTClient::createConsensusData()
{
    if(!isMajorSyncing())
    {
        {
            //UpgradableGuard l(x_working);
            {
                if(m_working.isSealed())
                {
                    //UpgradeGuard l2(l);
                    //m_working.resetCurrent();
                    return m_working.blockData();
                }
            }
        }
        bool t = true;
        if(!isSyncing() && !m_remoteWorking && m_syncTransactionQueue.compare_exchange_strong(t, false))
            syncTransactionQueue();
        DEV_WRITE_GUARDED(x_working)
        {
            m_working.commitToSeal(bc(), m_extraData);

            m_sealingInfo = m_working.info();

            clog(ClientTrace) << "Generating seal on" << m_sealingInfo.hash(WithoutSeal) << "#" << m_sealingInfo.number();
            clog(ClientDetail) << "Include" << m_working.pending().size() << "transactions";
            clog(ClientDetail) << "Caching new commited block";

            bc().addBlockCache(m_working, m_working.info().difficulty());

            if(sealCurrentBlockWithLock(m_sealingInfo))
            {
                DEV_WRITE_GUARDED(x_postSeal)
                    m_postSeal = m_working;
            }
            else
            {
                clog(ClientWarn) << "Seal null block";
                return bytes();
            }
        }
    }
    return m_working.blockData();
}

bool PBFTClient::verify(const bytes& block)
{
    bc().verifyFromPBFT(block, m_stateDB);
    return true;
}

bool PBFTClient::commit(const bytes& block)
{
    ImportRoute ir;
    if(block.empty())
    {
        clog(ClientWarn) << "Stop commit null block";
        return false;
    }
    try
    {
        Timer t;
        //ir = bc().import(block, m_stateDB, true);
        ir = bc().importFromPBFT(block, m_stateDB, true);
        double elapsed = t.elapsed();
        clog(ClientDetail) << "1 block imported in" << unsigned(elapsed * 1000)
            << "ms (" << (1 / elapsed) << "blocks/s) in #" << bc().number();
    }
    catch(...)
    {
        clog(ClientWarn) << "Commit failed";
        resyncStateFromChain();
        return false;
    }
    syncStateAfterImport(ir);
    return true;
}

void PBFTClient::syncStateAfterImport(const ImportRoute& ir)
{
    if (ir.liveBlocks.empty())
        return;
    h256Hash changeds;
    onDeadBlocks(ir.deadBlocks, changeds);
    for (auto const& t: ir.goodTranactions)
    {
        clog(ClientTrace) << "Safely dropping transaction " << t.sha3();
        m_tq.dropGood(t);
    }
    for (auto const& h: ir.liveBlocks)
        appendFromBlock(h, BlockPolarity::Live, changeds);
    resyncStateFromChain();
    noteChanged(changeds);
}

void PBFTClient::broadcastPBFTMsgs(const bytes& msg)
{
    host()->broadcastPBFTMsgs(msg);
}

void PBFTClient::testSealing()
{
    if(bc().chainParams().chainID != -1)
    {
        clog(ClientWarn) << "As not test mode";
        return;
    }
    if(getTxNum() > 0)
    {
        bytes b = createConsensusData();
        if(verify(b))
            commit(b);
    }
}

long PBFTClient::getTxNum()
{
    return m_tq.status().current;
}

long PBFTClient::getHeight()
{
    return bc().number();
}
