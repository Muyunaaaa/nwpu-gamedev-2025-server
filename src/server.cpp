#include <cstdint>
#include "util/time/getTime.h"
#include "network/common.cpp"
//
// Created by 冯于洋 on 25-12-20.
//
int main() {
    constexpr double TICK_INTERVAL = 1.0 / 60.0;
    //临时
    bool running = true;
    bool isConnected = false;
    ENetHost* host = nullptr; // 假设已经初始化并绑定好了 ENetHost

    while (running) {
        if (!isConnected) continue;

        uint64_t tick_start = now_ms();

        PacketQueue queue = PollNetwork(host);      // 收包 → 入队
        RunOneTick(queue);           // 清队列 → 裁决 → 广播

        SleepUntilNextTick(tick_start);
    }

}