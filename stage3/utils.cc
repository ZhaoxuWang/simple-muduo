#include "utils.h"
#include <sys/time.h>
#include <memory>
#include<string>


int64_t get_now(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t now = tv.tv_sec * 1000000 + tv.tv_usec;
    return now;
}

int64_t addTime(int64_t time, double seconds){
    return time + seconds * 1000000;
}

std::string TimeToString(int64_t time){
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(time / 1000000);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    
    return buf;
}