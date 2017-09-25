/**************************************************************************/
/* Copyright(c) 2017-2017 Zhengzhou Esunny Information & Technology Co,.Ltd */
/*本软件文档资料是易盛公司的资产,任何人士阅读和使用本资料必须获得相应的书面*/
/*授权,承担保密责任和接受相应的法律约束.                                  */
/***************************************************************************/
/**
 * @file lock_info.h
 * @brief
 * @author Lilongpu
 * @date 2017年06月22日
 */

#ifndef LOCK_INFO_H_
#define LOCK_INFO_H_
#include "proposal_info.h"
/**
 * @brief 锁定信息
 */
typedef struct LockInfo_tag
{
    long          lock_height; ///<锁定高度
    long          lock_round;  ///<锁定轮次
    ProposalInfo lock_data;   ///<锁定数据
    bool         is_valid;    ///<是否有效
    /**
     * @brief 构造函数
     *
     */
    LockInfo_tag():lock_height(-1),lock_round(-1),lock_data(),is_valid(false)
    {}

    /**
     * @brief 更新锁定信息
     *
     * @param h 高度
     * @param r 轮次
     * @param data 数据
     * @param valid 有效性
     */
    void update(long h, long r, const ProposalInfo& data, bool valid = true)
    {
        lock_height = h;
        lock_round = r;
        lock_data = data;
        is_valid = true;
    }

    /**
     * @brief 清除锁定信息
     */
    void clear()
    {
        lock_height = -1;
        lock_round = -1;
        lock_data.clear();
        is_valid = false;
    }
}LockInfo;
#endif //LOCK_INFO_H_
