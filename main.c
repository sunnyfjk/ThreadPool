/**
 * @Author: fjk
 * @Date:   2017-12-30T13:37:30+08:00
 * @Email:  sunnyfjk@gmail.com
 * @Filename: main.c
 * @Last modified by:   fjk
 * @Last modified time: 2018-01-07T17:37:12+08:00
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
