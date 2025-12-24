//
// Created by 冯于洋 on 25-12-20.
//
//这里主要将每一个数据包进行解码，转换为PlayerState数据结构以及对应的时间戳，方便后续的逻辑处理
#include <ctime>
#include <map>
#include <flatbuffers/flatbuffer_builder.h>
#include <game/dataset/InputDataPackage.h>
#include <game/dataset/PlayerState.h>
#include <util/math/common.h>

#include "glm/vec3.hpp"



flatbuffers::FlatBufferBuilder createBufferBuilder() {

}
// void decodePerPacket() {
//     flatbuffers::FlatBufferBuilder builder;
//
//     // struct 是值类型
//     game::protocol::Vec3 pos(1.0f, 2.0f, 3.0f);
//     game::protocol::Vec3 rot(0.0f, 90.0f, 0.0f);
//
//     auto entity = game::protocol::CreateEntityState(
//         builder,
//         now_ms(),   // timestamp
//         1,          // id
//         &pos,
//         &rot,
//         100,        // health
//         true,       // isAlive
//         false       // isShooting
//     );
//
//     builder.Finish(entity);
// }
