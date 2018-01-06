/**
 * @Author: fjk
 * @Date:   2018-01-05T21:26:05+08:00
 * @Email:  sunnyfjk@gmail.com
 * @Filename: DynamicallyAdjustingThreadPool.h
 * @Last modified by:   fjk
 * @Last modified time: 2018-01-06T01:25:21+08:00
 */
#ifndef __DYNAMICALLY_ADJUSTING_THREAD_POOL__
#define __DYNAMICALLY_ADJUSTING_THREAD_POOL__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <list.h>
enum {
        THREAD_POOL_SWITCH_OPEN,
        THREAD_POOL_SWITCH_CLOSE,
};
enum {
        THREAD_STATE_CLOSE,
        THREAD_STATE_WAIT,
        THREAD_STATE_RUN,
};
enum {
        THREAD_SWITCH_CLOSE,
        THREAD_SWITCH_RUN,
};
struct CallBackFunction_t {
        void (*CallBackRun)(void *arg);
        void (*CallBackStop)(void *arg);
};
struct Job_t {
        struct list_head JobNode;
        struct CallBackFunction_t CallBackFunction;
        void *arg;
};
struct DAThreadPool_t;
struct WorkThread_t {
        struct list_head ThreadNode;
        pthread_t ThreadId;
        pthread_cond_t ThreadCond;
        int ThreadSwitch;
        int ThreadState;
        struct Job_t *Job;
        struct DAThreadPool_t *DAThreadPool;
};
struct DAThreadPool_t {
        /*线程池开关*/
        int ThreadPoolSwitch;
        /*最大线程数量*/
        int MaxThreadSize;
        /*存在的线程*/
        int NowThreadSize;
        /*空闲线程数量*/
        int IdelThreadSize;
        /*最先线程空闲数量*/
        int MaxIdelThreadSize;
        /*回收线程数量*/
        int DeadThreadSize;
        /*工作数量*/
        int JobSize;
        /*线程队列*/
        struct list_head ThreadRoot;
        /*工作队列*/
        struct list_head JobRoot;
        /*监测线程*/
        pthread_t MonitorThread;
        /*关闭线程条件变量*/
        pthread_cond_t CloseThreadCond;
        /*任务条件变量*/
        pthread_cond_t JobCond;
        /*任务锁*/
        pthread_mutex_t DAThreadMutex;
};
struct DAThreadConfig_t {
        /*最大线程数量*/
        int MaxThreadSize;
        /*最先线程空闲数量*/
        int MaxIdelThreadSize;
};
struct DAThreadPool_t *NewDAThreadPool(struct DAThreadConfig_t *datc);
void FreeDAThreadPool(struct DAThreadPool_t *datp);
int AddDAThreadPoolJob(struct DAThreadPool_t *datp,struct Job_t *jb);
struct Job_t *NewJob(struct CallBackFunction_t *CallBack,void *arg);
#endif
