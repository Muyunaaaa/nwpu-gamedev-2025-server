#pragma once
#include <chrono>

#include "enet/enet.h"
#include "math/common.h"

namespace Config {
    namespace match {
        using MilliSecDuration =
        std::chrono::duration<float, std::chrono::milliseconds::period>;
        static constexpr MilliSecDuration PURCHASE_TIMER =
                MilliSecDuration(10.0f * 1000.0f);

        static constexpr MilliSecDuration ROUND_TIMER =
                MilliSecDuration(10.0f * 1000.0f);

        static constexpr MilliSecDuration C4_TIMER =
                MilliSecDuration(45.0f * 1000.0f);

        static constexpr MilliSecDuration END_UNTIL_SERVER_CLOSE_TIMER =
                MilliSecDuration(15.0f * 1000.0f);

        static constexpr int WIN_PRIZE = 3000;
        static constexpr int LOSE_PRIZE = 2000;
        static constexpr int KILL_PRIZE = 300;
        static constexpr int PLANT_PRIZE = 200;
        static constexpr int DEFUSE_PRIZE = 200;
        static constexpr int MAX_BALANCE = 6000;

        static constexpr myu::math::Vec3 C4_DEFAULT_PLANT_POSITION_T_SIDE = myu::math::Vec3(0.0f, 0.0f, 0.0f);
        static constexpr myu::math::Vec3 C4_DEFAULT_PLANT_POSITION_CT_SIDE = myu::math::Vec3(100.0f, 0.0f, 100.0f);
    }
    namespace server {
        static constexpr size_t MAX_PER_TICK_PACKET_PROCESS = 1024;
        constexpr float TPS = 60.0f;
    }
    namespace player {
        // static constexpr float WALK_SPEED = 3.0f;      // m/s
        // static constexpr float RUN_SPEED = 5.0f;       // m/s
        // static constexpr float CROUCH_SPEED = 1.5f;    // m/s
        // static constexpr float JUMP_VELOCITY = 5.0f;   // m/s
        static constexpr float MAX_HEALTH = 100.0f;
    }
    namespace network {
        static constexpr int SERVER_PORT = 1234;
        static constexpr int SERVER_HOST = ENET_HOST_ANY;
        static constexpr int MAX_CLIENTS = 32;
        static constexpr int MAX_EACH_CLIENT_CHANNELS = 2;
    }
    namespace room {
        static constexpr int TARGET_PLAYERS = 2;
        static constexpr int MAX_ROUNDS = 2;
    }
}
