/**
 * @Author: fjk
 * @Date:   2018-01-03T16:53:13+08:00
 * @Email:  sunnyfjk@gmail.com
 * @Filename: ThreadPoolModule.c
 * @Last modified by:   fjk
 * @Last modified time: 2018-01-04T16:34:06+08:00
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ThreadPoolModule.h>

#ifndef PERR
#define PERR(fmt,arg ...) fprintf(stderr,"[%s:%d]"fmt,__FUNCTION__,__LINE__, ## arg)
#endif
static void *ThreadPoolWork(void *arg)
{
        struct ThreadPool_t *tp=arg;
        struct ThreadJob_t *tj=NULL;
        struct Thread_t *t=NULL,*pos=NULL,*n=NULL;
        struct ThreadState_t ts;

        while(1) {
                pthread_mutex_lock(&tp->ThreadPoolMutex);
                while(tp->NowWorkSize<=0 || !(tp->ThreadPoolSwitch==THREAD_POOL_OPEN)) {
                        if(tp->ThreadPoolSwitch==THREAD_POOL_CLOSE) {
                                tp->ThreadPoolState=THREAD_POOL_STATE_CLOSE;
                                pthread_cond_broadcast(&tp->ThreadPoolCond);
                                pthread_mutex_unlock(&tp->ThreadPoolMutex);
                                goto ThreadPoolClose;
                        }
                        tp->ThreadPoolState=THREAD_POOL_STATE_WAIT;
                        pthread_cond_broadcast(&tp->ThreadPoolCond);
                        pthread_cond_wait(&tp->ThreadPoolCond,&tp->ThreadPoolMutex);
                }

                tp->ThreadPoolState=THREAD_POOL_STATE_RUN;
                t=NULL;
                if(tp->NowThreadSize>0) {
                        list_for_each_entry_safe_reverse(pos,n,&tp->ThreadRoot,ThreadNode){
                                memset(&ts, 0, sizeof(ts));
                                GetThreadState(pos,&ts);
                                if(ts.State==THREAD_STATE_WAIT) {
                                        t=pos;
                                        break;
                                }
                        }
                }
                if(t==NULL && tp->NowThreadSize < tp->MaxThreadSize)
                {
                        t=NewThread();
                        if(t!=NULL) {
                                StartThread(t);
                                list_add(&t->ThreadNode,&tp->ThreadRoot);
                                tp->NowThreadSize++;

                        }
                }
                if(t!=NULL) {
                        tj=list_last_entry(&(tp->JobRoot),struct ThreadJob_t,JobNode);
                        list_del_init(&tj->JobNode);
                        tp->NowWorkSize--;
                        AddThreadJob(t,tj);

                }
                pthread_cond_broadcast(&tp->ThreadPoolCond);
                pthread_mutex_unlock(&tp->ThreadPoolMutex);
        }
ThreadPoolClose:
        pthread_exit(NULL);
}
struct ThreadPool_t *NewThreadPool(int MaxThreadSize)
{
        struct ThreadPool_t *tp=NULL;
        int ret=0;
        tp=calloc(1,sizeof(*tp));
        if(tp==NULL) {
                PERR("Calloc Thread Pool Err\n");
                goto Calloc_Thread_Pool_Err;
        }
        INIT_LIST_HEAD(&tp->ThreadRoot);
        INIT_LIST_HEAD(&tp->JobRoot);
        tp->ThreadPoolSwitch=THREAD_POOL_OPEN;
        tp->MaxThreadSize=MaxThreadSize;
        ret=pthread_mutex_init(&tp->ThreadPoolMutex,NULL);
        if(ret<0) {
                PERR("pthread_mutex_init_err\n");
                goto pthread_mutex_init_err;
        }
        ret=pthread_cond_init(&tp->ThreadPoolCond,NULL);
        if(ret<0) {
                PERR("pthread_cond_init_err\n");
                goto pthread_cond_init_err;
        }
        ret=pthread_create(&tp->ThreadManage,NULL,ThreadPoolWork,tp);
        if(ret<0) {
                PERR("pthread_create_Thread_Manage_err\n");
                goto pthread_create_Thread_Manage_err;
        }
        return tp;
pthread_create_Thread_Manage_err:
        pthread_cond_destroy(&tp->ThreadPoolCond);
pthread_cond_init_err:
        pthread_mutex_destroy(&tp->ThreadPoolMutex);
pthread_mutex_init_err:
        free(tp);
        tp=NULL;
Calloc_Thread_Pool_Err:
        return NULL;

}
void FreeThreadPool(struct ThreadPool_t *tp)
{
        int ret = 0;
        struct Thread_t *pos=NULL,*n=NULL;
        struct ThreadJob_t *tpos=NULL,*tn=NULL;
        ret=CloseThreadPool(tp);
        if(ret<0)
                return;
        ret=pthread_mutex_lock(&tp->ThreadPoolMutex);
        if(ret<0)
                return;
        while(!(tp->ThreadPoolState==THREAD_POOL_STATE_CLOSE))
                pthread_cond_wait(&tp->ThreadPoolCond,&tp->ThreadPoolMutex);
        pthread_join(tp->ThreadManage,NULL);
        pthread_mutex_unlock(&tp->ThreadPoolMutex);
        if(tp->NowThreadSize>0) {
                list_for_each_entry_safe_reverse(pos,n,&tp->ThreadRoot,ThreadNode){
                        list_del_init(&pos->ThreadNode);
                        FreeThread(pos,NULL);
                        pos=NULL;
                        tp->NowThreadSize--;
                }
        }
        if(tp->NowWorkSize>0)
        {
                list_for_each_entry_safe_reverse(tpos,tn,&tp->JobRoot,JobNode){
                        list_del_init(&tpos->JobNode);
                        if(tpos->CallBackFunction.CallBackStart!=NULL) {
                                (tpos->CallBackFunction.CallBackStart)(tpos->arg);
                        }
                        if(tpos->CallBackFunction.CallBackStop!=NULL) {
                                (tpos->CallBackFunction.CallBackStop)(tpos->arg);
                        }
                        free(tpos);
                        tp->NowWorkSize--;
                }
        }
        pthread_mutex_destroy(&tp->ThreadPoolMutex);
        pthread_cond_destroy(&tp->ThreadPoolCond);
        free(tp);
        tp=NULL;
}
int CloseThreadPool(struct ThreadPool_t *tp)
{
        int ret = 0;
        ret=pthread_mutex_lock(&tp->ThreadPoolMutex);
        if(ret<0)
                return ret;
        tp->ThreadPoolSwitch=THREAD_POOL_CLOSE;
        pthread_cond_broadcast(&tp->ThreadPoolCond);
        pthread_mutex_unlock(&tp->ThreadPoolMutex);
        return 0;
}
int AddThreadPoolJob(struct ThreadPool_t *tp,struct ThreadJob_t *tj)
{
        int ret = 0;
        ret=pthread_mutex_lock(&tp->ThreadPoolMutex);
        if(ret<0)
                return ret;
        if(tp->ThreadPoolSwitch==THREAD_POOL_CLOSE) {
                pthread_cond_broadcast(&tp->ThreadPoolCond);
                pthread_mutex_lock(&tp->ThreadPoolMutex);
                return -1;
        }
        list_add(&tj->JobNode,&tp->JobRoot);
        tp->NowWorkSize++;
        pthread_cond_broadcast(&tp->ThreadPoolCond);
        pthread_mutex_unlock(&tp->ThreadPoolMutex);
        return 0;
}
