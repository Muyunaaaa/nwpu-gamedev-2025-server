#pragma once
#include <memory>
#include <string>
#include <glm/vec3.hpp>

#include "game/PlayerTeam.h"
#include "entity/Weapon.h"
#include "network/NetPacket.h"

struct PlayerState {
    // 身份
    ClientID client_id;
    std::string name;
    PlayerTeam team;

    // 位置 & 物理
    glm::vec3 position{0, 0, 0};
    glm::vec3 velocity{0, 0, 0};
    bool on_ground = false;

    // 生命
    int health = 100;
    bool alive = true;

    // 拥有武器
    std::unique_ptr<WeaponInstance> primary;
    std::unique_ptr<WeaponInstance> secondary;

    //持有武器
    std::unique_ptr<WeaponInstance> current_weapon;

    // 经济 & 战绩
    int money = 800;
    uint32_t kills = 0;
    uint32_t deaths = 0;
};
