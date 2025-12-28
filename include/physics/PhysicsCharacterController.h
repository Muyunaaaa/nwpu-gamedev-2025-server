#pragma once
#include "InputIntent.h"
#include "Physics/JoltIncludes.hpp"

#include "Jolt/Physics/Character/CharacterVirtual.h"
#include "math/common.h"
#include "network/NetPacket.h"

struct PhysicsCharacterController {
private:
    JPH::Ref<JPH::CharacterVirtual> m_character;
    ClientID m_clientID;

public:
    void createCharacter(const JPH::Shape &shape);

    void createCharacter(myu::math::Vec3 spawnPoint); //默认创建胶囊体
    void destroyCharacter();

    void updateCharacterPhysics(float deltaTime, const InputIntent &intent);

    PhysicsCharacterController(ClientID id): m_clientID(id) {}
};
