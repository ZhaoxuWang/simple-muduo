#pragma once
#include <sys/time.h>
#include <memory>

inline int64_t get_now(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t now = tv.tv_sec * 1000000 + tv.tv_usec;
    return now;
}

inline int64_t addTime(int64_t time, double seconds){
    return time + seconds * 1000000;
}