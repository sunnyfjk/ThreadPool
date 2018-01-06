/**
 * @Author: fjk
 * @Date:   2017-12-30T13:37:30+08:00
 * @Email:  sunnyfjk@gmail.com
 * @Filename: main.c
 * @Last modified by:   fjk
 * @Last modified time: 2018-01-06T14:08:19+08:00
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <DynamicallyAdjustingThreadPool.h>
#if 1
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
  #if 0
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
  #if 0
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
  #if 0
        int i=0;
        struct ThreadPool_t *pool=NULL;
        struct ThreadJob_t *tj=NULL;
        struct CallBackFunction_t callback={
                .CallBackStart=workStart,
                .CallBackStop=workStop,
        };
        pool=NewThreadPool(20);
        for(i=0; i<100; i++) {
                tj=NewThreadJob(&callback,(void *)i);
                if(tj==NULL)
                        continue;
                AddThreadPoolJob(pool,tj);
        }
        printf("[%s:%d]\n",__FUNCTION__,__LINE__);
        sleep(1);
        printf("[%s:%d]\n",__FUNCTION__,__LINE__);
        FreeThreadPool(pool);
#endif
        int i=0;
        struct DAThreadPool_t *datp=NULL;
        struct DAThreadConfig_t datc ={
                .MaxThreadSize=20,
                .MaxIdelThreadSize=5,
        };
        struct CallBackFunction_t calback={
                .CallBackRun=workStart,
                .CallBackStop=workStop,
        };
        struct Job_t *tj=NULL;

        datp=NewDAThreadPool(&datc);
        for(i=0; i<100; i++) {
                tj=NewJob(&calback,(void *)i);
                if(tj==NULL)
                        continue;
                AddDAThreadPoolJob(datp,tj);
        }
        sleep(20);
        printf("-------------------[%s:%d]----------------------\n",__FUNCTION__,__LINE__ );
        FreeDAThreadPool(datp);
        printf("-------------------[%s:%d]----------------------\n",__FUNCTION__,__LINE__ );
        return 0;


}
