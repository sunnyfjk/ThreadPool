/**
 * @Author: fjk
 * @Date:   2017-12-30T13:38:05+08:00
 * @Email:  sunnyfjk@gmail.com
 * @Filename: ThreadPoll.h
 * @Last modified by:   fjk
 * @Last modified time: 2017-12-31T21:54:14+08:00
 */
#ifndef __INCLUDE_THREAD_POOL_H__
#define __INCLUDE_THREAD_POOL_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <list.h>
#include <pthread.h>
enum {
        WORK_STATE_CLOSE,
        WORK_STATE_WAIT,
        WORK_STATE_RUN,
};
enum {
        THREAD_POOL_STATE_CLOSE,
        THREAD_POOL_STATE_OPEN,
};
struct JobCallback_t {
        /*工作回调函数*/
        void (*JobCallbackRun)(void *arg);
        void (*JobCallbackStop)(void *arg);
};
struct Job_t {
        /*工作回调函数*/
        struct JobCallback_t JobFunction;
        /*工作回调函数 参数*/
        void *arg;
        struct list_head JobNode;
};
#define  ThreadPool struct ThreadPool_t
struct WorkThread_t {
        /*线程号*/
        pthread_t tid;
        /*线程条件变量，避免出现惊群问题*/
        pthread_cond_t WorkCond;
        /*线程状态*/
        int WorkState;
        /*线程池*/
        ThreadPool *tpool;
        /*工作*/
        struct Job_t *job;
        struct list_head ThreadNode;

};
struct ThreadPool_t {
        /*线程池状态*/
        int ThreadPoolState;
        /*等待添加任务个数*/
        int WaitAddJobThread;
        /*最大线程数量*/
        int MaxPoolSize;
        /*最小存在线程数量*/
        int MinPoolsize;
        /*现在存在的线程数量*/
        int NowPoolSize;
        /*空闲线程数量*/
        int IdelPoolSize;
        /*运行线程数量*/
        int RunPoolSize;
        /*最大任务数量*/
        int MaxJobSize;
        /*工作池中的数量*/
        int NowJobSize;
        /*线程池*/
        struct list_head ThreadRoot;
        /*工作池*/
        struct list_head JobRoot;
        /*线程池锁*/
        pthread_mutex_t ThreadPoolMutex;
        /*线程池条件变量*/
        pthread_cond_t ThreadPoolCond;

};
struct ThreadPoolConfig_t {
        /*最大线程数量*/
        int MaxPoolSize;
        /*最大任务数量*/
        int MaxJobSize;
};
struct ThreadPool_t *ThreadPoolNew(struct ThreadPoolConfig_t *tpc);
void ThreadPoolFree(struct ThreadPool_t *tp);
int ThreadPoolJobAdd(struct ThreadPool_t *tp,struct Job_t *job);
struct WorkThread_t *WorkThreadNew(struct ThreadPool_t *pool);
void WorkThreadFree(struct WorkThread_t *wt);
struct Job_t *JobNew(struct JobCallback_t *JobFunction,void *arg);
void JobFree(struct Job_t *jb);

#endif
