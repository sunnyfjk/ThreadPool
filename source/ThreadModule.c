/**
 * @Author: fjk
 * @Date:   2018-01-03T10:26:45+08:00
 * @Email:  sunnyfjk@gmail.com
 * @Filename: ThreadModule.c
 * @Last modified by:   fjk
 * @Last modified time: 2018-01-03T16:17:34+08:00
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ThreadModule.h>

#ifndef PERR
#define PERR(fmt,arg ...) fprintf(stderr,"[%s:%d]"fmt,__FUNCTION__,__LINE__, ## arg)
#endif
static void *ThreadWork(void *arg)
{
        struct Thread_t *t=arg;
        struct ThreadJob_t *tj=NULL;
        while(1)
        {
                pthread_mutex_lock(&t->ThreadMutex);
                while(t->ThreadJobRoot.JobNodeSize <= 0 || (t->ThreadOperation.Operation != THREAD_OPERATION_RUN)) {
                        if(t->ThreadOperation.Operation==THREAD_OPERATION_CLOSE) {
                                t->ThreadState.State=THREAD_STATE_CLOSE;
                                pthread_cond_broadcast(&t->ThreadCond);
                                pthread_mutex_unlock(&t->ThreadMutex);
                                goto THREAD_CLOSE;
                        }
                        t->ThreadState.State=THREAD_STATE_WAIT;
                        pthread_cond_wait(&t->ThreadCond,&t->ThreadMutex);
                }
                t->ThreadState.State=THREAD_STATE_RUN;
                tj=list_last_entry(&(t->ThreadJobRoot.JobRoot),struct ThreadJob_t,JobNode);
                list_del_init(&tj->JobNode);
                t->ThreadJobRoot.JobNodeSize--;
                pthread_cond_broadcast(&t->ThreadCond);
                pthread_mutex_unlock(&t->ThreadMutex);
                PERR("\n");
                if(tj->CallBackFunction.CallBackStart!=NULL) {
                        (tj->CallBackFunction.CallBackStart)(tj->arg);
                }
                PERR("\n");
                if(tj->CallBackFunction.CallBackStop!=NULL) {
                        (tj->CallBackFunction.CallBackStop)(tj->arg);
                }
                PERR("\n");
                free(tj);
        }
THREAD_CLOSE:
        pthread_exit(NULL);
}
struct Thread_t *NewThread(void)
{
        struct Thread_t *t=NULL;
        int ret=0;
        t=calloc(1,sizeof(*t));
        if(t==NULL) {
                PERR("Calloc New Thread Err\n");
                goto CallocNewThreadErr;
        }
        t->ThreadOperation.Operation=THREAD_OPERATION_WAIT;
        INIT_LIST_HEAD(&(t->ThreadJobRoot.JobRoot));
        INIT_LIST_HEAD(&t->ThreadNode);
        ret=pthread_cond_init(&t->ThreadCond,NULL);
        if(ret<0) {
                PERR("pthread_cond_init_err\n");
                goto pthread_cond_init_err;
        }
        ret=pthread_mutex_init(&t->ThreadMutex,NULL);
        if(ret<0) {
                PERR("pthread_mutex_init_err\n");
                goto pthread_mutex_init_err;
        }
        ret=pthread_create(&t->tid,NULL,ThreadWork,t);
        if(ret<0) {
                PERR("pthread_create_err\n");
                goto pthread_create_err;
        }
        return t;
pthread_create_err:
        pthread_mutex_destroy(&t->ThreadMutex);
pthread_mutex_init_err:
        pthread_cond_destroy(&t->ThreadCond);
pthread_cond_init_err:
        free(t);
        t=NULL;
CallocNewThreadErr:
        return NULL;
}
void FreeThread(struct Thread_t *t,struct ThreadJobRoot_t *ThreadJobRoot)
{

        struct ThreadOperation_t op={
                .Operation=THREAD_OPERATION_CLOSE,
        };
        struct ThreadJob_t *pos=NULL,*n=NULL;
        int ret=0;
        if(t==NULL )
                return;
        ret=SetThreadOpetation(t,&op);
        if(ret<0)
                return;
        ret=pthread_mutex_lock(&t->ThreadMutex);
        if(ret<0)
                return;
        while(!(t->ThreadState.State==THREAD_STATE_CLOSE))
                pthread_cond_wait(&t->ThreadCond,&t->ThreadMutex);
        /*退出线程队列*/
        list_del_init(&t->ThreadNode);
        /*对剩余工作进行处理*/
        if(ThreadJobRoot != NULL) {

                /*取出剩余工作开始*/
                ThreadJobRoot->JobNodeSize+=t->ThreadJobRoot.JobNodeSize;
                list_splice_init(&(t->ThreadJobRoot.JobRoot),&ThreadJobRoot->JobRoot);
                /*取出剩余工作结束*/}
        else {
                list_for_each_entry_safe_reverse(pos,n,&(t->ThreadJobRoot.JobRoot),JobNode){
                        list_del_init(&pos->JobNode);
                        if(pos->CallBackFunction.CallBackStart!=NULL) {
                                (pos->CallBackFunction.CallBackStart)(pos->arg);
                        }
                        if(pos->CallBackFunction.CallBackStop!=NULL) {
                                (pos->CallBackFunction.CallBackStop)(pos->arg);
                        }
                        free(pos);
                        t->ThreadJobRoot.JobNodeSize--;
                }
        }

        pthread_mutex_unlock(&t->ThreadMutex);

        pthread_join(t->tid,NULL);
        pthread_mutex_destroy(&t->ThreadMutex);
        pthread_cond_destroy(&t->ThreadCond);
        free(t);

}
int SetThreadOpetation(struct Thread_t *t,struct ThreadOperation_t *to)
{
        int ret=0;
        ret=pthread_mutex_lock(&t->ThreadMutex);
        if(ret<0)
                return -1;
        if(t->ThreadOperation.Operation==THREAD_OPERATION_CLOSE) {
                pthread_cond_broadcast(&t->ThreadCond);
                pthread_mutex_unlock(&t->ThreadMutex);
                return -2;
        }
        memcpy(&t->ThreadOperation,to,sizeof(*to));
        pthread_cond_broadcast(&t->ThreadCond);
        pthread_mutex_unlock(&t->ThreadMutex);
        return 0;
}
int GetThreadState(struct Thread_t *t,struct ThreadState_t *ts)
{
        int ret=0;
        ret=pthread_mutex_lock(&t->ThreadMutex);
        if(ret<0)
                return -1;
        memcpy(ts,&t->ThreadState,sizeof(*ts));
        pthread_cond_broadcast(&t->ThreadCond);
        pthread_mutex_unlock(&t->ThreadMutex);
        return 0;
}
int AddThreadJob(struct Thread_t *t,struct ThreadJob_t *tj)
{

        int ret=0;
        INIT_LIST_HEAD(&tj->JobNode);
        ret=pthread_mutex_lock(&t->ThreadMutex);
        if(ret<0)
                return -1;
        if(t->ThreadOperation.Operation==THREAD_OPERATION_CLOSE) {
                pthread_cond_broadcast(&t->ThreadCond);
                pthread_mutex_unlock(&t->ThreadMutex);
                return -1;
        }
        list_add(&tj->JobNode,&t->ThreadJobRoot.JobRoot);
        t->ThreadJobRoot.JobNodeSize++;
        pthread_cond_broadcast(&t->ThreadCond);
        pthread_mutex_unlock(&t->ThreadMutex);
        return 0;
}
int StartThread(struct Thread_t *t)
{
        int ret=0;
        ret=pthread_mutex_lock(&t->ThreadMutex);
        if(ret<0)
                return -1;
        t->ThreadOperation.Operation=THREAD_OPERATION_RUN;
        pthread_cond_broadcast(&t->ThreadCond);
        pthread_mutex_unlock(&t->ThreadMutex);

        return 0;
}
int StopThread(struct Thread_t *t)
{
        int ret=0;
        ret=pthread_mutex_lock(&t->ThreadMutex);
        if(ret<0)
                return -1;
        t->ThreadOperation.Operation=THREAD_OPERATION_WAIT;
        pthread_cond_broadcast(&t->ThreadCond);
        pthread_mutex_unlock(&t->ThreadMutex);
        return 0;
}
int CloseThread(struct Thread_t *t)
{
        int ret=0;
        ret=pthread_mutex_lock(&t->ThreadMutex);
        if(ret<0)
                return -1;
        t->ThreadOperation.Operation=THREAD_OPERATION_CLOSE;
        pthread_cond_broadcast(&t->ThreadCond);
        pthread_mutex_unlock(&t->ThreadMutex);
        return 0;
}
struct ThreadJob_t *NewThreadJob(struct CallBackFunction_t *CallBack,void *arg)
{
        struct ThreadJob_t *tj=NULL;
        if(CallBack==NULL||CallBack->CallBackStop==NULL || CallBack->CallBackStart==NULL)
                return NULL;
        tj=calloc(1,sizeof(*tj));
        if(tj==NULL) {
                PERR("Calloc Thread job err\n");
                goto Calloc_Thread_Job_err;
        }
        memcpy(&tj->CallBackFunction, CallBack, sizeof(*CallBack));
        tj->arg=arg;
        INIT_LIST_HEAD(&tj->JobNode);
        return tj;
Calloc_Thread_Job_err:
        return NULL;
}

void FreeThreadJob(struct ThreadJob_t * tj)
{
        list_del_init(&tj->JobNode);
        if(tj->CallBackFunction.CallBackStart!=NULL) {
                (tj->CallBackFunction.CallBackStart)(tj->arg);
        }
        if(tj->CallBackFunction.CallBackStop!=NULL) {
                (tj->CallBackFunction.CallBackStop)(tj->arg);
        }
        free(tj);
}
