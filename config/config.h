#pragma once
#include <chrono>

#include "enet/enet.h"
#include "entity/PlantZone.h"
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
                MilliSecDuration(30.0f * 1000.0f);

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

        static constexpr PlantZone BOMB_ZONE_A = {
            myu::math::Vec3(-10.0f, 0.0f, -10.0f), // Min
            myu::math::Vec3(10.0f, 5.0f, 10.0f) // Max (高度给了5米，防止跳起来不能下包)
        };

        // B 包点区域
        static constexpr PlantZone BOMB_ZONE_B = {
            myu::math::Vec3(90.0f, 0.0f, 90.0f), // Min
            myu::math::Vec3(110.0f, 5.0f, 110.0f) // Max
        };

        //todo:根据实际地图调整
        static constexpr myu::math::Vec3 DEFULAT_CT_HEAD_ROTATION = myu::math::Vec3(0.0f, 0.0f, 0.0f);
        static constexpr myu::math::Vec3 DEFULAT_T_HEAD_ROTATION = myu::math::Vec3(0.0f, 0.0f, 0.0f);
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

        static constexpr myu::math::Vec3 DUST2_CT_SPAWNS[] = {
            {0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            // {2.0f, 1.0f, 0.0f},
            // {4.0f, 1.0f, 0.0f},
            // {0.0f, 1.0f, 2.0f},
            // {2.0f, 1.0f, 2.0f}
        };

        static constexpr myu::math::Vec3 DUST2_T_SPAWNS[] = {
            {0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            // {0.0f, 20.0f, 0.0f},
            // {0.0f, 20.0f, 0.0f},
            // {0.0f, 20.0f, 0.0f},
            // {0.0f, 20.0f, 0.0f},
            // {0.0f, 20.0f, 0.0f},
            // {102.0f, 1.0f, 100.0f},
            // {104.0f, 1.0f, 100.0f},
            // {100.0f, 1.0f, 102.0f},
            // {102.0f, 1.0f, 102.0f}
        };
    }

    namespace physics {
        static constexpr float PLAYER_SPEED = 3.0f;
        static constexpr float PLAYER_JUMP_VELOCITY = 5.0f;

        static constexpr float PLAYER_HALF_HEIGHT = 0.8f;
        static constexpr float PLAYER_RADIUS = 0.4f;

        // offset from mass center (half height) to camera position (eye level)
        static constexpr float PLAYER_CAMERA_OFFSET_ = 0.6f;

        static constexpr float PLAYER_SUPPORTING_VOLUME_CONSTANT = -0.5f;
        static constexpr float PLAYER_MAX_SLOPE_ANGLE_DEGREES = 45.0f;

        static constexpr float PLAYER_STICK_TO_FLOOR_STEP_DOWN = -0.4f;
        static constexpr float PLAYER_WALK_STAIRS_STEP_UP = 0.4f;

        static constexpr std::string_view DUST2_ROOT = DUST2_BASE;
        static constexpr std::string_view DUST2_CONFIG = DUST2_BASE "dust2.config";
        static constexpr std::string_view DUST2_GLTF = DUST2_BASE "dust2.glb";

        static constexpr std::string_view PLAYGROUND_GLTF = PLAYGROUND_BASE "playground.glb";
    }
}
