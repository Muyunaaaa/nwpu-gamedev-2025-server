//
// Created by 冯于洋 on 25-12-20.
//

#ifndef PLAYERSTATE_H
#define PLAYERSTATE_H

#include <cstdint>
#include "math/common.h"
#include "math/glm_include.h"

class PlayerState {
private:
    uint32_t id;
    myu::math::Vec3 position;
    glm::vec3 rotation;
    uint32_t health;
    bool isAlive;
    bool isShooting;

public:
    PlayerState(){}
    PlayerState(uint32_t id, myu::math::Vec3 pos, glm::vec3 rot, uint32_t health, bool alive, bool shooting)
        : id(id), position(pos), rotation(glm::normalize(rot)), health(health), isAlive(alive), isShooting(shooting) {
    }

    [[nodiscard]] uint32_t id1() const {
        return id;
    }

    void set_id(uint32_t id) {
        this->id = id;
    }

    [[nodiscard]] myu::math::Vec3 position1() const {
        return position;
    }

    void set_position(const myu::math::Vec3 &position) {
        this->position = position;
    }

    [[nodiscard]] glm::vec3 rotation1() const {
        return rotation;
    }

    void set_rotation(const glm::vec3 &rotation) {
        this->rotation = rotation;
    }

    [[nodiscard]] uint32_t health1() const {
        return health;
    }

    void set_health(uint32_t health) {
        this->health = health;
    }

    [[nodiscard]] bool is_alive() const {
        return isAlive;
    }

    void set_is_alive(bool is_alive) {
        isAlive = is_alive;
    }

    [[nodiscard]] bool is_shooting() const {
        return isShooting;
    }

    void set_is_shooting(bool is_shooting) {
        isShooting = is_shooting;
    }
};


#endif //PLAYERSTATE_H
