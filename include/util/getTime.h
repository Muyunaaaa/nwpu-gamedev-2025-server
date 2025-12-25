// TimeUtils.h
#pragma once
#include <cstdint>
#include <chrono>

namespace myu::time {

    /// 获取当前时间戳（纳秒，单调时钟）
    inline uint64_t now_ns() {
        using namespace std::chrono;
        return duration_cast<nanoseconds>(
            steady_clock::now().time_since_epoch()
        ).count();
    }

    /// 微秒
    inline uint64_t now_us() {
        using namespace std::chrono;
        return duration_cast<microseconds>(
            steady_clock::now().time_since_epoch()
        ).count();
    }

    /// 毫秒（最常用）
    inline uint64_t now_ms() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(
            steady_clock::now().time_since_epoch()
        ).count();
    }

}
