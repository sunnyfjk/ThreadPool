/**
 * @Author: fjk
 * @Date:   2017-12-30T13:37:30+08:00
 * @Email:  sunnyfjk@gmail.com
 * @Filename: main.c
 * @Last modified by:   fjk
 * @Last modified time: 2018-01-03T09:25:12+08:00
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ThreadPool.h>
#include <unistd.h>

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

int main( void )
{
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

        return 0;
}
