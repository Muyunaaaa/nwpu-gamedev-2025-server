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
        return now_ns() / 1'000;
    }

    /// 毫秒（最常用）
    inline uint64_t now_ms() {
        return now_ns() / 1'000'000;
    }

}
