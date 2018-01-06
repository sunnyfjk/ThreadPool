/**
 * @Author: fjk
 * @Date:   2018-01-05T21:26:14+08:00
 * @Email:  sunnyfjk@gmail.com
 * @Filename: DynamicallyAdjustingThreadPool.c
 * @Last modified by:   fjk
<<<<<<< HEAD
 * @Last modified time: 2018-01-06T16:53:22+08:00
=======
<<<<<<< HEAD
 * @Last modified time: 2018-01-06T14:12:14+08:00
=======
 * @Last modified time: 2018-01-06T14:01:00+08:00
>>>>>>> d4bc0ca56d5cc2704f38f638f6d0333d22e8bbea
>>>>>>> cf057c9575421316227787fbd3c4640b67fbd8f3
 */
#include <DynamicallyAdjustingThreadPool.h>

#ifndef PERR
#define PERR(fmt,arg ...) fprintf(stderr,"[%s:%d]"fmt,__FUNCTION__,__LINE__, ## arg)
#endif
static void *MonitorThread(void *arg)
{
        struct DAThreadPool_t * datp=arg;
        struct WorkThread_t *pos=NULL,*n=NULL;
        while(1) {
                pthread_mutex_lock(&datp->DAThreadMutex);
                while(datp->DeadThreadSize<=0 && datp->NowThreadSize>0)
                        pthread_cond_wait(&datp->CloseThreadCond,&datp->DAThreadMutex);
                list_for_each_entry_safe_reverse(pos,n,&datp->ThreadRoot,ThreadNode){
                        if(pos->ThreadState==THREAD_STATE_CLOSE)
                        {
                                list_del_init(&pos->ThreadNode);
                                pthread_join(pos->ThreadId,NULL);
                                pthread_cond_destroy(&pos->ThreadCond);
                                free(pos);
                                datp->DeadThreadSize--;
                                datp->NowThreadSize--;
#ifdef __DEBUG__
                                PERR("NowThreadSize=%d\n",datp->NowThreadSize);
#endif
                        }
                }
                if(datp->NowThreadSize<=0)
                        break;

                pthread_mutex_unlock(&datp->DAThreadMutex);
        }
        pthread_mutex_unlock(&datp->DAThreadMutex);
        pthread_exit(NULL);
}
static void *WorkThread(void *arg)
{
        struct Job_t *job=NULL;
        struct WorkThread_t *wt=arg;
        struct DAThreadPool_t * datp=wt->DAThreadPool;
        while(1)
        {
                pthread_mutex_lock(&datp->DAThreadMutex);
                while(datp->JobSize<=0 || !(wt->ThreadSwitch==THREAD_SWITCH_RUN)) {
                        if(wt->ThreadState!=THREAD_STATE_WAIT) {
                                wt->ThreadState=THREAD_STATE_WAIT;
                                datp->IdelThreadSize++;
                        }
                        if( (datp->IdelThreadSize)>(datp->MaxIdelThreadSize) || wt->ThreadSwitch==THREAD_SWITCH_CLOSE) {
                                wt->ThreadState=THREAD_STATE_CLOSE;
                                datp->IdelThreadSize--;
                                datp->DeadThreadSize++;
                                goto __WORK_THREAD_END__;
                        }
                        pthread_cond_wait(&wt->ThreadCond,&datp->DAThreadMutex);
                }
                if(wt->ThreadState!=THREAD_STATE_RUN) {
                        wt->ThreadState=THREAD_STATE_RUN;
                        datp->IdelThreadSize--;
                }
                job=list_last_entry(&datp->JobRoot,struct Job_t,JobNode);
                list_del_init(&job->JobNode);
                datp->JobSize--;
                pthread_cond_signal(&datp->JobCond);
                pthread_mutex_unlock(&datp->DAThreadMutex);
                if(job->CallBackFunction.CallBackRun!=NULL)
                        (job->CallBackFunction.CallBackRun)(job->arg);
                if(job->CallBackFunction.CallBackStop!=NULL)
                        (job->CallBackFunction.CallBackStop)(job->arg);
                free(job);
                job=NULL;
        }
__WORK_THREAD_END__:
        pthread_cond_signal(&datp->CloseThreadCond);
        pthread_mutex_unlock(&datp->DAThreadMutex);
        pthread_exit(NULL);
}
struct DAThreadPool_t *NewDAThreadPool(struct DAThreadConfig_t *datc)
{
        struct DAThreadPool_t * datp=NULL;
        datp=calloc(1,sizeof(*datp));
        if(datp==NULL) {
                PERR("DAThreadPoolCallocErr\n");
                goto DAThreadPoolCallocErr;
        }
        datp->MaxThreadSize=datc->MaxThreadSize;
        datp->MaxIdelThreadSize=datc->MaxIdelThreadSize;
        datp->ThreadPoolSwitch=THREAD_POOL_SWITCH_OPEN;
        INIT_LIST_HEAD(&datp->ThreadRoot);
        INIT_LIST_HEAD(&datp->JobRoot);
        if(pthread_cond_init(&datp->JobCond,NULL)<0) {
                PERR("pthread_cond_init_job_cond_err\n");
                goto pthread_cond_init_job_cond_err;
        }
        if(pthread_cond_init(&datp->CloseThreadCond,NULL)<0) {
                PERR("pthread_cond_init_close_cond_err\n");
                goto pthread_cond_init_close_cond_err;
        }
        if(pthread_mutex_init(&datp->DAThreadMutex,NULL)<0) {
                PERR("pthread_mutex_init_DAThread_Mutex_err\n");
                goto pthread_mutex_init_DAThread_Mutex_err;
        }
        if(pthread_create(&datp->MonitorThread,NULL,MonitorThread,datp)<0) {
                PERR("pthread_create_Monitor_Thread_err\n");
                goto pthread_create_Monitor_Thread_err;
        }
        return datp;
pthread_create_Monitor_Thread_err:
        pthread_mutex_destroy(&datp->DAThreadMutex);
pthread_mutex_init_DAThread_Mutex_err:
        pthread_cond_destroy(&datp->CloseThreadCond);
pthread_cond_init_close_cond_err:
        pthread_cond_destroy(&datp->JobCond);
pthread_cond_init_job_cond_err:
        free(datp);
        datp=NULL;
DAThreadPoolCallocErr:
        return NULL;
}
void FreeDAThreadPool(struct DAThreadPool_t *datp)
{
        struct WorkThread_t *pos=NULL,*n=NULL;
        /*不允许添加任务*/
        pthread_mutex_lock(&datp->DAThreadMutex);
        datp->ThreadPoolSwitch=THREAD_POOL_SWITCH_CLOSE;
        pthread_mutex_unlock(&datp->DAThreadMutex);
        /*完成现有任务*/
        pthread_mutex_lock(&datp->DAThreadMutex);
        while(datp->JobSize>0) {
                pthread_cond_wait(&datp->JobCond,&datp->DAThreadMutex);
                PERR("JobSize=%d\n",datp->JobSize);
        }

        pthread_mutex_unlock(&datp->DAThreadMutex);
        /*关闭全部线程*/
        list_for_each_entry_safe_reverse(pos,n,&datp->ThreadRoot,ThreadNode){
                pthread_mutex_lock(&datp->DAThreadMutex);
                pos->ThreadSwitch=THREAD_STATE_CLOSE;
                pthread_cond_signal(&pos->ThreadCond);
                pthread_mutex_unlock(&datp->DAThreadMutex);
        }
        /*回收监测线程*/

        pthread_join(datp->MonitorThread,NULL);
        /*释放锁、信号量*/
        pthread_mutex_destroy(&datp->DAThreadMutex);
        pthread_cond_destroy(&datp->CloseThreadCond);
        pthread_cond_destroy(&datp->JobCond);
        free(datp);
        datp=NULL;
}
int AddDAThreadPoolJob(struct DAThreadPool_t *datp,struct Job_t *jb)
{
        int ret=0;
        struct WorkThread_t *pos=NULL,*n=NULL;

        pthread_mutex_lock(&datp->DAThreadMutex);
        if(datp->ThreadPoolSwitch==THREAD_POOL_SWITCH_CLOSE) {
                pthread_mutex_unlock(&datp->DAThreadMutex);
                return -1;
        }
        INIT_LIST_HEAD(&jb->JobNode);
        list_add(&jb->JobNode,&datp->JobRoot);
        datp->JobSize++;
        /*寻找空闲线程直接激活*/
        if(datp->IdelThreadSize>0) {
                list_for_each_entry_safe_reverse(pos,n,&datp->ThreadRoot,ThreadNode){
                        if(pos->ThreadState==THREAD_STATE_WAIT)
                        {
                                pos->Job=jb;
                                pthread_cond_signal(&pos->ThreadCond);
                                pthread_mutex_unlock(&datp->DAThreadMutex);
                                return 0;
                        }

                }
        }
        /*没有空闲线程，并且现在存在的线程小于最大线程则创建新的线程*/
        if((datp->NowThreadSize)<(datp->MaxThreadSize)) {
                pos=calloc(1,sizeof(*pos));
                INIT_LIST_HEAD(&pos->ThreadNode);
                pos->ThreadSwitch=THREAD_SWITCH_RUN;
                pos->ThreadState=THREAD_STATE_RUN;
                pos->DAThreadPool=datp;
                ret=pthread_cond_init(&pos->ThreadCond,NULL);
                if(ret<0) {
                        PERR("pthread_cond_init_pos_Thread_Cond_err\n");
                        free(pos);
                        pos=NULL;
                        goto __ADD_JOB_END__;
                }
                ret=pthread_create(&pos->ThreadId,NULL,WorkThread,pos);
                if(ret<0) {
                        PERR("pthread_create_pos_Thread_err\n");
                        pthread_cond_destroy(&pos->ThreadCond);
                        free(pos);
                        pos=NULL;
                        goto __ADD_JOB_END__;
                }
                list_add(&pos->ThreadNode,&datp->ThreadRoot);
                datp->NowThreadSize++;
#ifdef __DEBUG__
                PERR("NowThreadSize=%d\n",datp->NowThreadSize);
#endif
        }
__ADD_JOB_END__:
        pthread_mutex_unlock(&datp->DAThreadMutex);
        return ret;
}
struct Job_t *NewJob(struct CallBackFunction_t *CallBack,void *arg)
{
        struct Job_t *tj=NULL;
        if(CallBack==NULL||CallBack->CallBackStop==NULL || CallBack->CallBackRun==NULL)
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
