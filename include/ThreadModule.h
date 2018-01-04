/**
 * @Author: fjk
 * @Date:   2018-01-03T09:46:30+08:00
 * @Email:  sunnyfjk@gmail.com
 * @Filename: ThreadModule.h
 * @Last modified by:   fjk
 * @Last modified time: 2018-01-04T10:46:16+08:00
 */
#ifndef __THREAD_MOUDLE_H__
#define __THREAD_MOUDLE_H__
#include <pthread.h>
#include <list.h>
enum {
        THREAD_STATE_CLOSE,
        THREAD_STATE_WAIT,
        THREAD_STATE_RUN,
};
enum {
        THREAD_OPERATION_CLOSE,
        THREAD_OPERATION_WAIT,
        THREAD_OPERATION_RUN,
};
struct ThreadState_t {
        /*运行状态*/
        int State;
};
struct ThreadOperation_t {
        /*线程操作*/
        int Operation;
};
struct CallBackFunction_t {
        void (*CallBackStart)(void *arg);
        void (*CallBackStop)(void *arg);
};
struct ThreadJob_t {
        struct list_head JobNode;
        struct CallBackFunction_t CallBackFunction;
        void *arg;
};
struct ThreadJobRoot_t {
        size_t JobNodeSize;
        struct list_head JobRoot;
};
struct Thread_t {
        struct list_head ThreadNode;
        /*只在线程内修改，线程外只读*/
        struct ThreadState_t ThreadState;
        /*只在线程外改变，线程内只读*/
        struct ThreadOperation_t ThreadOperation;
        struct ThreadJobRoot_t ThreadJobRoot;
        pthread_t tid;
        pthread_cond_t ThreadCond;
        pthread_mutex_t ThreadMutex;

};
struct Thread_t *NewThread(void);

void FreeThread(struct Thread_t *t,struct ThreadJobRoot_t *ThreadJobRoot);

int SetThreadOpetation(struct Thread_t *t,struct ThreadOperation_t *to);

int GetThreadState(struct Thread_t *t,struct ThreadState_t *ts);

int GetThreadJobSize(struct Thread_t *t);

int AddThreadJob(struct Thread_t *t,struct ThreadJob_t *tj);

int StartThread(struct Thread_t *t);

int StopThread(struct Thread_t *t);

int CloseThread(struct Thread_t *t);

struct ThreadJob_t *NewThreadJob(struct CallBackFunction_t *CallBack,void *arg);

void FreeThreadJob(struct ThreadJob_t * tj);
#endif
