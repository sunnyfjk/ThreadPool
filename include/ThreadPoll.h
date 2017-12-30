/**
 * @Author: fjk
 * @Date:   2017-12-30T13:38:05+08:00
 * @Email:  sunnyfjk@gmail.com
 * @Filename: ThreadPoll.h
 * @Last modified by:   fjk
 * @Last modified time: 2017-12-30T20:48:49+08:00
 */
#ifndef __INCLUDE_THREAD_POOL_H__
#define __INCLUDE_THREAD_POOL_H__

#define TPool_t  void

struct TPoolConfig_t {
        int MaxPoolSize;
        int MaxWorkSize;
};

TPool_t *TPoolNew(struct TPoolConfig_t *tpc);

void TPoolFree(TPool_t *tp);

int TPoolAddJop(TPool_t *tp,void (*JobCallBack)(void *),void *arg);

#endif
