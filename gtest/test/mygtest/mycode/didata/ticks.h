#ifndef _TICKS_H_
#define _TICKS_H_

#include <unistd.h>
#include <time.h>



static inline unsigned long get_tick_count(){
    struct timespec t;
    clock_gettime( CLOCK_MONOTONIC_COARSE , &t );
    return t.tv_sec * 1000 + t.tv_nsec / (1000*1000);
}

#endif
