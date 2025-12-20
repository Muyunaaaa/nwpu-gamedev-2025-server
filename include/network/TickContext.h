//
// Created by 冯于洋 on 25-12-20.
//

#ifndef TICKCONTEXT_H
#define TICKCONTEXT_H
#include <cstdint>

struct TickContext {
    uint64_t tick_id;
    uint64_t tick_start_time;
};

#endif //TICKCONTEXT_H
