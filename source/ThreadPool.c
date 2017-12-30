/**
 * @Author: fjk
 * @Date:   2017-12-30T13:38:53+08:00
 * @Email:  sunnyfjk@gmail.com
 * @Filename: ThreadPool.c
 * @Last modified by:   fjk
 * @Last modified time: 2017-12-30T22:57:35+08:00
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <list.h>
#include <ThreadPoll.h>

#undef PERR
#define PERR(fmt,arg ...) fprintf(stderr,"[%s:%d]"fmt,__FUNCTION__,__LINE__, ## arg)

enum {
        WORK_STATE_CLOSE,
        WORK_STATE_RUN,
        WORK_STATE_WAIT,
};
enum {
        WORK_SWITCH_OPEN,
        WORK_SWITCH_CLOSE,
};
enum {
        THREAD_POOL_SWITCH_OPEN,
        THREAD_POOL_SWITCH_CLOSE,
};
struct ThreadState_t {
        pthread_t tid;
        int WorkState;
        int WorkSwitch;
};
struct TPool {
        /*线程池状态*/
        int TPoolSwitch;
        /*线程池中最大线程数量*/
        int MaxPoolSize;
        /*现在有的线程*/
        int NowWorkThreadSize;
        /*工作的线程*/
        int RunWorkThreadSize;
        /*处于等待状态的线程*/
        int WaitWorkThreadSize;
        /*现在有的工作数量*/
        int WorkSize;
        /*最大存储数量*/
        int MaxWorkSize;
        /*线程池空闲工作线程上限*/
        int IdleUpperLimitPoolSize;
        /*增加线程最小工作量程下限*/
        float WorkLowerLimitPoolSize;
        /*线程池*/
        struct ThreadState_t *threads;
        /*任务锁*/
        pthread_mutex_t JobMutex;
        /*任务条件变量*/
        pthread_cond_t JobCond;
        /*工作池*/
        struct list_head JobRoot;
        /*监测线程池线程*/
        pthread_t MonitorThread;
        pthread_mutex_t ChangeThreadState;

};

struct TJob {
        struct list_head JobNode;
        void (*JobCallBack)(void *);
        void *arg;
};
struct TJob *TJobNew(void (*JobCallBack)(void *),void *arg)
{
        struct TJob *t=malloc(sizeof(*t));
        if(t==NULL || JobCallBack==NULL )
        {
                PERR("tjob malloc err\n");
                goto tjob_malloc_err;
        }
        memset(t,0,sizeof(*t));
        t->JobCallBack=JobCallBack;
        t->arg=arg;
        INIT_LIST_HEAD(&t->JobNode);
        return t;
tjob_malloc_err:
        return NULL;
}

void TJobFree(struct TJob *tj)
{
        free(tj);
}
int ThreadStateInit(struct ThreadState_t *ts){
        if(ts==NULL)
                return -1;
        ts->tid=0;
        ts->WorkState=WORK_STATE_CLOSE;
        ts->WorkSwitch=WORK_SWITCH_CLOSE;
        return 0;
}
void *WorkThread(void *arg)
{

        while(1)
        {

        }
        pthread_exit(NULL);
}
void *MonitorThread(void *arg)
{

        pthread_exit(NULL);
}
TPool_t *TPoolNew(struct TPoolConfig_t *tpc)
{
        struct TPool *p=NULL;
        int ret=0;
        p=malloc(sizeof(*p));
        int i=0;
        if(p==NULL) {
                PERR("malloc TPool err\n");
                goto malloc_TPool_err;
        }
        memset(p, 0, sizeof(*p));

        p->TPoolSwitch=THREAD_POOL_SWITCH_OPEN;
        p->IdleUpperLimitPoolSize=5;
        p->MaxPoolSize=tpc->MaxPoolSize;
        p->MaxWorkSize=tpc->MaxWorkSize;
        INIT_LIST_HEAD(&p->JobRoot);
        ret=pthread_cond_init(&p->JobCond,NULL);
        if(ret<0) {
                PERR("pthread_cond_init_err\n");
                goto pthread_cond_init_err;
        }
        ret=pthread_mutex_init(&p->JobMutex,NULL);
        if(ret<0) {
                PERR("pthread_mutex_init_err\n");
                goto pthread_mutex_init_err;
        }
        ret=pthread_mutex_init(&p->ChangeThreadState,NULL);
        if(ret<0)
        {
                PERR("pthread_mutex_init_ChangeThreadState_err\n");
                goto pthread_mutex_init_ChangeThreadState_err;
        }

        p->threads=malloc(sizeof(*(p->threads))*(p->MaxPoolSize));
        if(p->threads==NULL)
        {
                PERR("thread malloc err\n");
                goto thread_malloc_err;
        }
        for(i=0; i<p->MaxPoolSize; i++)
        {
                ThreadStateInit(&(p->threads[i]));
        }

        ret=pthread_create(&p->MonitorThread,NULL, MonitorThread,p);
        if(ret<0) {
                PERR("pthread_create_err\n");
                goto pthread_create;
        }

        return p;
pthread_create:
        free(p->threads);
thread_malloc_err:
        pthread_mutex_destroy(&p->ChangeThreadState);
pthread_mutex_init_ChangeThreadState_err:
        pthread_mutex_destroy(&p->JobMutex);
pthread_mutex_init_err:
        pthread_cond_destroy(&p->JobCond);
pthread_cond_init_err:
        free(p);
malloc_TPool_err:
        return NULL;
}

void TPoolFree(TPool_t *tp)
{
        int i=0,flag=0;
        struct TPool *p=tp;
        struct TJob *pos=NULL,*n=NULL;
        p->TPoolSwitch=THREAD_POOL_SWITCH_CLOSE;
        pthread_mutex_lock(&p->JobMutex);
        list_for_each_entry_safe_reverse(pos,n,&(p->JobRoot),JobNode){
                list_del(&pos->JobNode);
                p->WorkSize--;
                free(pos);
        }
        pthread_cond_broadcast(&p->JobCond);
        pthread_mutex_unlock(&p->JobMutex);
        while(1)
        {
                for(i=0; i<p->MaxPoolSize; i++)
                {
                        pthread_mutex_lock(&p->ChangeThreadState);
                        if(p->threads[i].WorkState!=WORK_STATE_CLOSE)
                        {
                                flag=1;
                                pthread_mutex_unlock(&p->ChangeThreadState);
                                break;
                        }
                        pthread_mutex_unlock(&p->ChangeThreadState);

                }
                if(!flag)
                        break;
        }


        pthread_mutex_destroy(&p->ChangeThreadState);
        pthread_mutex_destroy(&p->JobMutex);
        pthread_cond_destroy(&p->JobCond);
        free(p->threads);
        free(p);
}



int TPoolAddJop(TPool_t *tp,void (*JobCallBack)(void *),void *arg)
{
        struct TPool *p=tp;
        struct TJob *t=NULL;
        if(JobCallBack==NULL || tp ==NULL )
                return -1;
        pthread_mutex_lock(&p->JobMutex);
        while(p->WorkSize>p->MaxWorkSize || p->TPoolSwitch==THREAD_POOL_SWITCH_CLOSE)
        {
                if(p->TPoolSwitch==THREAD_POOL_SWITCH_CLOSE)
                {
                        pthread_cond_broadcast(&p->JobCond);
                        pthread_mutex_unlock(&p->JobMutex);
                        return -2;
                }
                pthread_cond_wait(&p->JobCond, &p->JobMutex);
        }
        t=TJobNew(JobCallBack,arg);
        if(t==NULL) {
                pthread_cond_broadcast(&p->JobCond);
                pthread_mutex_unlock(&p->JobMutex);
                return -3;
        }
        list_add(&t->JobNode,&p->JobRoot);
        p->WorkSize++;
        pthread_mutex_unlock(&p->JobMutex);

        return 0;
}
