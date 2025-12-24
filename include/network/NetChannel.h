//
// Created by Administrator on 25-12-25.
//

#ifndef NETCHANNEL_H
#define NETCHANNEL_H
#include <cstdint>
enum NetChannel : uint8_t {
    CH_RELIABLE = 0,   // 登录、加入房间、重要状态
    CH_STATE    = 1,   // 高频状态同步（可丢包）
    CH_CHAT     = 2,   // 聊天
};
#endif //NETCHANNEL_H
