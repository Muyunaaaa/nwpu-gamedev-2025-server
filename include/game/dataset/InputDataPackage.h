//
// Created by 冯于洋 on 25-12-20.
//

#ifndef DATAPACKAGE_H
#define DATAPACKAGE_H
#include "spdlog/sinks/ansicolor_sink-inl.h"

namespace myu {
    namespace net {
        struct InputDataPackage {
            /* 后续需要调整数据包数据类型,根据flatbuffer做修改 */
            //数据包应包含玩家id，玩家位置，玩家朝向，玩家是否开火，玩家动作意图（物理），数据包时间戳
            char* data;
            size_t size;
            InputDataPackage(char* d, size_t s) : data(d), size(s) {}
        };
    }
}
#endif //DATAPACKAGE_H
