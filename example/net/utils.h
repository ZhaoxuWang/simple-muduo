#pragma once
#include <sys/time.h>
#include <string>
#include <stdint.h>

int64_t get_now();

int64_t addTime(int64_t time, double seconds);

std::string TimeToString(int64_t time);