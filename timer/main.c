#include "timer.h"
#include <stdio.h>
#include <unistd.h>

unsigned long timer_func(void* data1,void* data2)
{
    const char* p = data1;
    printf("this is time %s\n", p);
    return 1000;
}

int main(int argc,char** argv)
{
    TIMER_register( timer_func , 1000 , "xiaoming" , 0 );
    TIMER_register( timer_func , 1000 , "dabiao" , 0 );
    TIMER_register( timer_func , 1000 , "wokao" , 0 );
    TIMER_register( timer_func , 1000 , "diaoren" , 0 );
    TIMER_register( timer_func , 1000 , "cc" , 0 );
    TIMER_register( timer_func , 1000 , "girls" , 0 );
    TIMER_register( timer_func , 1000 , "boys" , 0 );
    TIMER_register( timer_func , 1000 , "fam" , 0 );

    while(1){
        usleep( 1000 * TIMER_loop() );
    }

    return 0;
};
