/**
 * @Author: fjk
 * @Date:   2017-12-30T13:37:30+08:00
 * @Email:  sunnyfjk@gmail.com
 * @Filename: main.c
 * @Last modified by:   fjk
 * @Last modified time: 2017-12-30T22:25:03+08:00
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ThreadPoll.h>
#include <unistd.h>

void work(void *arg)
{

}

int main( void )
{
        int i=0;
        TPool_t *tp;
        struct TPoolConfig_t st;
        st.MaxPoolSize=100;
        st.MaxWorkSize=1000;

        tp=TPoolNew(&st);

        for(i=0; i<5; i++)
        {

                TPoolAddJop(tp,work,NULL);
        }
        while(1) ;
        TPoolFree(tp);
        return 0;
}
