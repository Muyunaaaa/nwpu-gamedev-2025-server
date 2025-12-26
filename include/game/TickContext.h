//
// Created by 冯于洋 on 25-12-20.
//

#ifndef TICKCONTEXT_H
#define TICKCONTEXT_H
#include <cstdint>

/*
 *需要维护的每个Tick的上下文信息，比如Tick ID，Tick开始时间等，存储所有tick信息
 */
struct TickContext {
    uint64_t tick_id;
    uint64_t tick_start_time;
};

#endif //TICKCONTEXT_H
