/**
 * @Author: fjk
 * @Date:   2017-12-31T19:06:45+08:00
 * @Email:  sunnyfjk@gmail.com
 * @Filename: ThreadPool.c
 * @Last modified by:   fjk
 * @Last modified time: 2018-01-02T17:59:21+08:00
 */
#include <ThreadPool.h>

#ifndef PERR
#define PERR(fmt,arg ...) fprintf(stderr,"[%s:%d]"fmt,__FUNCTION__,__LINE__, ## arg)
#endif
static void *WorkThread(void *arg)
{
        struct WorkThread_t *wt=arg;
        struct Job_t *job=NULL;
        while(wt->tpool->ThreadPoolState!=THREAD_POOL_STATE_CLOSE)
        {
                if(pthread_mutex_lock(&(wt->tpool->ThreadPoolMutex)))
                        pthread_exit(NULL);
                while(wt->job ==NULL ) {
                        wt->tpool->IdelPoolSize++;
                        wt->WorkState=WORK_STATE_WAIT;
                        if(wt->tpool->NowJobSize>0)
                                break;
                        pthread_cond_wait(&wt->WorkCond,&(wt->tpool->ThreadPoolMutex));
                }
                if(wt->job ==NULL) {
                        wt->job=list_last_entry(&(wt->tpool->JobRoot),struct Job_t,JobNode);
                        list_del_init(&(wt->job->JobNode));
                        wt->tpool->NowJobSize--;


                }

                job=wt->job;
                wt->job=NULL;
                wt->WorkState=WORK_STATE_RUN;
                wt->tpool->IdelPoolSize--;
                pthread_mutex_unlock(&(wt->tpool->ThreadPoolMutex));
                job->JobFunction.JobCallbackRun(job->arg);
                job->JobFunction.JobCallbackStop(job->arg);
                free(job);
                job=NULL;

        }
        wt->WorkState=WORK_STATE_CLOSE;
        pthread_cond_signal(&(wt->tpool->ThreadPoolCond));
        pthread_exit(NULL);
}
struct ThreadPool_t *ThreadPoolNew(struct ThreadPoolConfig_t *tpc)
{
        int ret=0;
        struct ThreadPool_t *tp=NULL;
        tp=calloc(1,sizeof(*tp));
        if(tp==NULL) {
                PERR("Thread Pool calloc err\n");
                goto ThreadPoolCalloc_err;
        }

        ret=pthread_mutex_init(&tp->ThreadPoolMutex,NULL);
        if(ret<0) {
                PERR("pthread_mutex_init_ThreadPoolMutex_err\n");
                goto pthread_mutex_init_ThreadPoolMutex_err;
        }
        ret=pthread_cond_init(&tp->ThreadPoolCond,NULL);
        if(ret<0) {
                PERR("pthread_cond_init_ThreadPoolCond_err\n");
                goto pthread_cond_init_ThreadPoolCond_err;
        }
        INIT_LIST_HEAD(&tp->ThreadRoot);
        INIT_LIST_HEAD(&tp->JobRoot);
        tp->MaxPoolSize=tpc->MaxPoolSize;
        tp->MaxJobSize=tpc->MaxJobSize;
        tp->ThreadPoolState=THREAD_POOL_STATE_OPEN;
        return tp;
pthread_cond_init_ThreadPoolCond_err:
        pthread_mutex_destroy(&tp->ThreadPoolMutex);
pthread_mutex_init_ThreadPoolMutex_err:
        free(tp);
        tp=NULL;
ThreadPoolCalloc_err:
        return NULL;

}
void ThreadPoolFree(struct ThreadPool_t *tp)
{
        struct WorkThread_t *pos=NULL,*n=NULL;
        struct Job_t *jpos=NULL,*jn=NULL;
        tp->ThreadPoolState=THREAD_POOL_STATE_CLOSE;
        list_for_each_entry_safe_reverse(pos,n,&tp->ThreadRoot,ThreadNode){
                pthread_mutex_lock(&tp->ThreadPoolMutex);
                while(pos->WorkState != WORK_STATE_CLOSE) {
                        pthread_cond_signal(&pos->WorkCond);
                        pthread_cond_wait(&tp->ThreadPoolCond,&tp->ThreadPoolMutex);
                }
                list_del_init(&pos->ThreadNode);
                pthread_mutex_unlock(&tp->ThreadPoolMutex);
                WorkThreadFree(pos);
        }
        pthread_mutex_lock(&tp->ThreadPoolMutex);
        list_for_each_entry_safe_reverse(jpos,jn,&tp->JobRoot,JobNode){
                list_del_init(&jpos->JobNode);
                JobFree(jpos);
        }
        pthread_mutex_unlock(&tp->ThreadPoolMutex);
        pthread_mutex_destroy(&tp->ThreadPoolMutex);
        pthread_cond_destroy(&tp->ThreadPoolCond);
        free(tp);
}
int ThreadPoolJobAdd(struct ThreadPool_t *tp,struct Job_t *job)
{
        struct WorkThread_t *pos=NULL,*n=NULL;
        if(pthread_mutex_lock(&tp->ThreadPoolMutex)<0)
                return -3;
        if(tp->ThreadPoolState==THREAD_POOL_STATE_CLOSE) {
                pthread_mutex_unlock(&tp->ThreadPoolMutex);
                return -1;
        }
        if(tp->IdelPoolSize>0) {
                list_for_each_entry_safe_reverse(pos,n,&tp->ThreadRoot,ThreadNode){
                        if(pos->WorkState==WORK_STATE_WAIT) {
                                pos->job=job;
                                pthread_cond_signal(&pos->WorkCond);
                                pthread_mutex_unlock(&tp->ThreadPoolMutex);
                                return 0;
                        }
                }
        }
        else if(tp->NowPoolSize>tp->MaxPoolSize) {
                tp->NowJobSize++;
                list_add(&job->JobNode,&tp->JobRoot);
        }
        else if(tp->NowJobSize<tp->MaxPoolSize)
        {
                pos=WorkThreadNew(tp);
                pos->job=job;
                if(pthread_create(&pos->tid,NULL,WorkThread,pos)<0) {
                        WorkThreadFree(pos);
                        pthread_mutex_unlock(&tp->ThreadPoolMutex);
                        return -2;
                }
                if(pthread_detach(pos->tid)<0) {
                        pthread_cancel(pos->tid);
                        pthread_join(pos->tid,NULL);
                        WorkThreadFree(pos);
                }
                tp->NowPoolSize++;
        }
        pthread_mutex_unlock(&tp->ThreadPoolMutex);


        return 0;
}
struct WorkThread_t *WorkThreadNew(struct ThreadPool_t *pool)
{
        struct WorkThread_t *wt=NULL;
        wt=calloc(1,sizeof(*wt));
        if(wt<0) {
                PERR("WorkThreadNew_calloc_err\n");
                goto WorkThreadNew_calloc_err;
        }
        if(pthread_cond_init(&wt->WorkCond,NULL)<0) {
                PERR("pthread_cond_init_WorkCond_err\n");
                goto pthread_cond_init_WorkCond_err;
        }
        wt->WorkState=WORK_STATE_RUN;
        wt->tpool=pool;
        INIT_LIST_HEAD(&wt->ThreadNode);
        return wt;
pthread_cond_init_WorkCond_err:
        free(wt);
WorkThreadNew_calloc_err:
        return NULL;
}
void WorkThreadFree(struct WorkThread_t *wt)
{
        pthread_cond_destroy(&wt->WorkCond);
        if(wt->job!=NULL) {
                wt->job->JobFunction.JobCallbackRun(wt->job->arg);
                wt->job->JobFunction.JobCallbackStop(wt->job->arg);
                free(wt);
        }
}
struct Job_t *JobNew(struct JobCallback_t *JobFunction,void *arg)
{
        struct Job_t *jb=NULL;
        jb=calloc(1,sizeof(*jb));
        if(jb==NULL) {
                PERR("JOB_CALLOC_ERR\n");
                goto JOB_CALLOC_ERR;
        }
        memcpy(&(jb->JobFunction),JobFunction,sizeof(*JobFunction));
        jb->arg=arg;
        INIT_LIST_HEAD(&jb->JobNode);
        return jb;
JOB_CALLOC_ERR:
        return NULL;
}
void JobFree(struct Job_t *jb)
{
        jb->JobFunction.JobCallbackRun(jb->arg);
        jb->JobFunction.JobCallbackStop(jb->arg);
        free(jb);
}
