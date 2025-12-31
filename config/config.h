#pragma once
#include <chrono>

#include "enet/enet.h"
#include "math/common.h"

#define MAP_ROOT "assets/maps/"
#define DUST2_BASE MAP_ROOT "de_dust2/"
#define PLAYGROUND_BASE MAP_ROOT "playground/"

namespace Config {
    namespace match {
        using MilliSecDuration =
        std::chrono::duration<float, std::chrono::milliseconds::period>;
        static constexpr MilliSecDuration PURCHASE_TIMER =
                MilliSecDuration(10.0f * 1000.0f);

        static constexpr MilliSecDuration ROUND_TIMER =
                MilliSecDuration(120.0f * 1000.0f);

        static constexpr MilliSecDuration C4_TIMER =
                MilliSecDuration(45.0f * 1000.0f);

        static constexpr MilliSecDuration END_UNTIL_SERVER_CLOSE_TIMER =
                MilliSecDuration(15.0f * 1000.0f);

        extern int WIN_PRIZE;
        extern int LOSE_PRIZE;
        extern int KILL_PRIZE;
        extern int PLANT_PRIZE;
        extern int DEFUSE_PRIZE;
        extern int MAX_BALANCE;

        // static constexpr PlantZone BOMB_ZONE_A = {
        //     myu::math::Vec3(-10.0f, 0.0f, -10.0f), // Min
        //     myu::math::Vec3(10.0f, 5.0f, 10.0f) // Max (高度给了5米，防止跳起来不能下包)
        // };
        //
        // // B 包点区域
        // static constexpr PlantZone BOMB_ZONE_B = {
        //     myu::math::Vec3(90.0f, 0.0f, 90.0f), // Min
        //     myu::math::Vec3(110.0f, 5.0f, 110.0f) // Max
        // };

        static constexpr myu::math::Vec3 DEFULAT_CT_HEAD_ROTATION = myu::math::Vec3(-1.0f, 0.0f, 0.0f);
        static constexpr myu::math::Vec3 DEFULAT_T_HEAD_ROTATION = myu::math::Vec3(1.0f, 0.0f, 0.0f);
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
        extern float MAX_HEALTH;
    }

    namespace network {
        extern int SERVER_PORT;
        static constexpr int SERVER_HOST = ENET_HOST_ANY;
        extern int MAX_CLIENTS;
        static constexpr int MAX_EACH_CLIENT_CHANNELS = 2;
    }

    namespace room {
        extern int TARGET_PLAYERS;
        extern int MAX_ROUNDS;

        static constexpr myu::math::Vec3 DUST2_CT_SPAWNS[] = {
            {23.194490f, -6.605331f, 16.552546f},
            {23.194490f, -6.605331f, 11.714236f},
            {23.194490f, -6.605331f, 14.133350f},
            {20.032581f, -6.605331f, 11.714236f},
            {20.326876f, -6.922765f, 17.384354f}
        };

        static constexpr myu::math::Vec3 DUST2_T_SPAWNS[] = {
            {-34.308010f, -0.583404f, -2.000000f},
            {-34.308010f, -0.583404f, -0.820000f},
            {-34.854240f, -2.000000f, 3.245000f},
            {-34.854240f, -1.374268f, 0.442785f},
            {-34.089184f, -2.442827f, 6.057330f}
        };
    }

    namespace physics {
        static constexpr float PLAYER_SPEED = 6.0f;
        static constexpr float PLAYER_JUMP_VELOCITY = 3.0f;

        static constexpr float PLAYER_HALF_HEIGHT = 0.5f;
        static constexpr float PLAYER_RADIUS = 0.3f;

        // offset from mass center (half height) to camera position (eye level)
        static constexpr float PLAYER_CAMERA_OFFSET_ = 0.6f;

        static constexpr float PLAYER_SUPPORTING_VOLUME_CONSTANT = -0.5f;
        static constexpr float PLAYER_MAX_SLOPE_ANGLE_DEGREES = 45.0f;

        static constexpr float PLAYER_STICK_TO_FLOOR_STEP_DOWN = -0.4f;
        static constexpr float PLAYER_WALK_STAIRS_STEP_UP = 0.4f;

        static constexpr std::string_view DUST2_ROOT = DUST2_BASE;
        static constexpr std::string_view DUST2_CONFIG = DUST2_BASE "dust2.config";
        static constexpr std::string_view DUST2_GLTF = DUST2_BASE "de_dust2.glb";

        static constexpr std::string_view PLAYGROUND_GLTF = PLAYGROUND_BASE "playground.glb";
    }

    void LoadFromToml(const std::string& path);
}
