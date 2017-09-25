/**
 * @file time_out_ticker.cpp      
 * @brief 
 * @author Lilongpu
 * @date 2017年06月13日
 */

#include "time_out_ticker.h"
#include "libdevcore/RLP.h"
#include "utils.h"
using namespace dev;
void TimeOutTicker::start()
{
	Worker::startWorking();
}
void TimeOutTicker::doWork() //重写
{
	DEV_GUARDED(lock_)
	{
		int gap_time = 0;
		if(vaild_info_)
		{
			struct timeval   end_time;
			gettimeofday(&end_time, NULL);
			gap_time = (end_time.tv_sec-start_time_.tv_sec)*1000 + (end_time.tv_usec - start_time_.tv_usec)/1000;
			if(gap_time >= interval_time_)
			{
				RLPStream rlp_stream;
				rlp_stream.appendList(3);
				rlp_stream.append(long2String(height_));
				rlp_stream.append(long2String(round_));
				rlp_stream.append(long2String(round_step_));
				msg_collector_->pushMsg( PBFTMsg(TIME_OUT, rlp_stream.out()) );
				vaild_info_ = false;
			}
		}
	}
}
void TimeOutTicker::setTimeInfo(long height, long round, PBFT_STATE state, int intervalTime)
{
	DEV_GUARDED(lock_)
	{
		gettimeofday(&start_time_, NULL);
		height_ 		= height;
		round_ 			= round;
		round_step_ 	= state;
		interval_time_ 	= intervalTime;
		vaild_info_ 	= true;
	}
}
void TimeOutTicker::registerMsgCollector(MsgCollectorI* collector)
{
	msg_collector_ = collector;
}
void TimeOutTicker::pause()
{
	DEV_GUARDED(lock_)
	{
		vaild_info_ = false;
	}
}
