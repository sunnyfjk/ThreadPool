/**
 * @Author: fjk
 * @Date:   2017-12-30T13:37:30+08:00
 * @Email:  sunnyfjk@gmail.com
 * @Filename: main.c
 * @Last modified by:   fjk
 * @Last modified time: 2018-01-03T16:18:15+08:00
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ThreadModule.h>
#if defined(__THREAD_MOUDLE_H__) || defined(__INCLUDE_THREAD_POOL_H__)
static void workStart(void *arg)
{
        sleep(1);
        printf("Job Start %d\n",(int)arg);
}
static void workStop(void *arg)
{
        sleep(1);
        printf("Job Stop %d\n",(int)arg);
}
#endif
int main( void )
{
  #if defined(__INCLUDE_THREAD_POOL_H__)
        int i = 0;
        struct ThreadPool_t *pool=NULL;
        struct Job_t *job=NULL;
        static struct ThreadPoolConfig_t conf={
                .MaxJobSize=1000,
                .MaxPoolSize=5,
        };
        static struct JobCallback_t jc={
                .JobCallbackRun=workStart,
                .JobCallbackStop=workStop,
        };
        pool=ThreadPoolNew(&conf);
        if(pool==NULL)
                return -1;
        printf("[%s:%d] add Job Satrt\n",__FUNCTION__,__LINE__);
        for(i=0; i<1000; i++)
        {

                job=JobNew(&jc,(void *)i);
                if(job==NULL)
                        continue;
                ThreadPoolJobAdd(pool,job);

        }
        printf("[%s:%d] add Job End\n",__FUNCTION__,__LINE__);
        ThreadPoolFree(pool);
  #endif
  #if defined(__THREAD_MOUDLE_H__)
        struct Thread_t *t=NULL;
        struct ThreadJob_t *tj=NULL;
        struct CallBackFunction_t callback={
                .CallBackStart=workStart,
                .CallBackStop=workStop,
        };
        int i = 0;
        t=NewThread();

        if(t==NULL)
                return -1;
        printf("[%s:%d] add Job Satrt\n",__FUNCTION__,__LINE__);
        StartThread(t);
        for(i=0; i<100; i++) {
                tj=NewThreadJob(&callback,(void *)i);
                if(tj==NULL)
                        continue;
                AddThreadJob(t,tj);
        }
        printf("[%s:%d] add Job end\n",__FUNCTION__,__LINE__);
        printf("[%s:%d] StartThread\n",__FUNCTION__,__LINE__);
        // StartThread(t);
        // sleep(10);
        printf("[%s:%d] StopThread\n",__FUNCTION__,__LINE__);
        StopThread(t);
        sleep(10);
        StartThread(t);
        printf("[%s:%d] StartThread\n",__FUNCTION__,__LINE__);
        StartThread(t);
        sleep(10);
        FreeThread(t,NULL);

  #endif
        return 0;
}
