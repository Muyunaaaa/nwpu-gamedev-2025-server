#include "util/time/getTime.h"

#include <chrono>
#include <cstdint>
//
// Created by 冯于洋 on 25-12-20.
//
uint64_t now_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()
    ).count();
}