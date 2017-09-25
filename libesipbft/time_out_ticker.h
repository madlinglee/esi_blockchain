/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file time_out_ticker.h      
 * @brief 
 * @author Lilongpu
 * @date 2017年06月10日
 */
#ifndef TIME_OUT_TICKER_H
#define TIME_OUT_TICKER_H
#include "pbft_common.h"
#include "msg_collector.h"
#include "libdevcore/Worker.h"
using namespace dev;
/**
 * @brief 超时任务管理
 */
class TimeOutTicker : public Worker
{
public:
	/**
	 * @brief 
	 *
	 * @param[in] thr_name 线程名称
	 * @param[in] waitMS 等待时间
	 */
	TimeOutTicker(const std::string& thr_name = "TimeOutThread", unsigned int waitMS = 1)
		:dev::Worker(thr_name, waitMS), msg_collector_(NULL), vaild_info_(false), height_(0), round_(0)
	{
	}
	virtual ~TimeOutTicker(){}
	/**
	 * @brief 开启线程
	 */
	void start();
	/**
	 * @brief 设置超时信息 
	 *
	 * @param[in] height 高度
	 * @param[in] round 轮次
	 * @param[in] state 状态
	 * @param[in] intervalTime 时间间隔
	 */
	void setTimeInfo(long height, long round, PBFT_STATE state, int intervalTime = 10);
	/**
	 * @brief 注册信息收集器
	 *
	 * @param[in] collector设置消息收集器
	 */
	void registerMsgCollector(MsgCollectorI* collector);
	/**
	 * @brief 重置超时任务
	 */
	void pause();
	void stop(){stopWorking();}
private:
	virtual void doWork(); //重写
private:
	mutable Mutex    lock_;
	struct timeval   start_time_;
	PBFT_STATE 		 round_step_;
	int 			 interval_time_;
	MsgCollectorI*   msg_collector_;
	bool 			 vaild_info_; 
	long 			 height_;
	long 			 round_;
};
#endif //TIME_OUT_TICKER_H
