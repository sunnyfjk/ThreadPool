/**
 * @Author: fjk
 * @Date:   2018-01-03T16:53:01+08:00
 * @Email:  sunnyfjk@gmail.com
 * @Filename: ThreadPoolModule.h
 * @Last modified by:   fjk
 * @Last modified time: 2018-01-04T09:51:01+08:00
 */
#ifndef __THREAD_POOL_MODULE_H__
#define __THREAD_POOL_MODULE_H__
#include <ThreadModule.h>

enum {
        THREAD_POOL_OPEN,
        THREAD_POOL_WAIT,
        THREAD_POOL_CLOSE,
};
enum {
        THREAD_POOL_STATE_RUN,
        THREAD_POOL_STATE_WAIT,
        THREAD_POOL_STATE_CLOSE,
};
struct ThreadPool_t {
        int ThreadPoolSwitch;
        int ThreadPoolState;
        int NowThreadSize;
        int IdelThreadSize;
        int NowWorkSize;
        int MaxThreadSize;
        struct list_head ThreadRoot;
        struct list_head JobRoot;
        pthread_mutex_t ThreadPoolMutex;
        pthread_cond_t ThreadPoolCond;
        pthread_t ThreadManage;
};
struct ThreadPool_t *NewThreadPool(int MaxThreadSize);
void FreeThreadPool(struct ThreadPool_t *tp);
int CloseThreadPool(struct ThreadPool_t *tp);
int AddThreadPoolJob(struct ThreadPool_t *tp,struct ThreadJob_t *tj);
#endif
