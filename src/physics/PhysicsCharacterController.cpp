#include "physics/PhysicsCharacterController.h"

#include "config.h"
#include "Server.h"
#include "core/GameContext.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "physics/PhysicsEnginee.h"
#include "Physics/JoltIncludes.hpp"

void PhysicsCharacterController::createCharacter(myu::math::Vec3 spawnPoint) {
    JPH::RVec3Arg spawnPos(spawnPoint.x, spawnPoint.y, spawnPoint.z);
    auto &physics = myu::PhysicsEngine::getInstance();
    // auto& bodyInterface = physics.getPhysicsSystem().GetBodyInterface();

    auto capsuleShape =
            JPH::CapsuleShapeSettings(
                Config::physics::PLAYER_HALF_HEIGHT,
                Config::physics::PLAYER_RADIUS)
            .Create()
            .Get();
    auto characterSettings = JPH::CharacterVirtualSettings();
    characterSettings.mShape = capsuleShape;
    characterSettings.mSupportingVolume =
            JPH::Plane(
                JPH::Vec3::sAxisY(),
                Config::physics::PLAYER_SUPPORTING_VOLUME_CONSTANT);
    characterSettings.mMaxSlopeAngle =
            JPH::DegreesToRadians(Config::physics::PLAYER_MAX_SLOPE_ANGLE_DEGREES);

    auto character =
            new JPH::CharacterVirtual(
                &characterSettings,
                spawnPos,
                JPH::QuatArg::sIdentity(),
                &physics.getPhysicsSystem());
    m_character = character;
}

void PhysicsCharacterController::destroyCharacter() {
    if (m_character != nullptr) {
        m_character = nullptr;
    }
}

void PhysicsCharacterController::updateCharacterPhysics(float deltaTime, const InputIntent &intent) {
    if (!m_character) {
        return;
    }
    auto character = m_character;
    auto &physicsSystem = myu::PhysicsEngine::getInstance().getPhysicsSystem();

    //设定基本参数
    //归一化速度向量，但需要物理引擎处理真正的方向，发包给客户端
    auto dir = intent.dir;
    auto velocity = dir * Config::physics::PLAYER_SPEED;
    auto lastVel = character->GetLinearVelocity();

    auto velXoZ = JPH::Vec3(velocity.x, 0, velocity.z);
    float velY = 0.0f;

    bool jumpRequested = velocity.y > 0.001f ? true : false;

    auto groundState = character->GetGroundState();

    bool onGround = groundState == JPH::CharacterVirtual::EGroundState::OnGround;
    bool onSteepGround = groundState == JPH::CharacterVirtual::EGroundState::OnSteepGround; // steep slope

    auto gravity = -character->GetUp() * physicsSystem.GetGravity().Length();
    if (onGround) {
        if (jumpRequested) {
            // apply jump impulse
            spdlog::info("LocalPlayerState: Jump requested");
            velY = Config::physics::PLAYER_JUMP_VELOCITY;
        } else {
            // on ground, preserve horizontal velocity, reset vertical velocity
            velY = 0.0f;
        }
        // todo: apply ground friction later
    } else if (onSteepGround) {
        // on steep ground, slide down
        JPH::Vec3 groundNormal = character->GetGroundNormal();
        auto velocity = velXoZ;

        float dot = velocity.Dot(groundNormal);
        if (dot < 0.0f) {
            // remove component against ground normal
            velocity -= groundNormal * dot;
        }

        velXoZ = velocity;

        velY = lastVel.GetY() + gravity.GetY() * deltaTime;
    } else {
        // in air, preserve Y velocity
        // ! fixme: players can still move freely in air, need to limit air control later
        velY = lastVel.GetY() + gravity.GetY() * deltaTime;
    }
    JPH::Vec3 finalVel(velXoZ.GetX(), velY, velXoZ.GetZ());

    // check collision
    JPH::CharacterVirtual::ExtendedUpdateSettings settings;
    // todo: fill in this settings, to allow character to climb
    if (!onGround) {
        settings.mStickToFloorStepDown = JPH::Vec3::sZero(); // disable stick to floor when in air
        settings.mWalkStairsStepUp = JPH::Vec3::sZero(); // disable walk stairs when in air
    } else {
        // if on ground, enable stick to floor and walk stairs
        // ! fixme: this still needs tuning
        settings.mStickToFloorStepDown =
                JPH::Vec3(
                    0,
                    Config::physics::PLAYER_STICK_TO_FLOOR_STEP_DOWN,
                    0);

        settings.mWalkStairsStepUp =
                JPH::Vec3(
                    0,
                    Config::physics::PLAYER_WALK_STAIRS_STEP_UP,
                    0);
    }
    character->SetLinearVelocity(finalVel);
    character->ExtendedUpdate(
        deltaTime,
        gravity,
        settings,
        physicsSystem.GetDefaultBroadPhaseLayerFilter(
            myu::Physics::Details::Layers::MOVING),
        physicsSystem.GetDefaultLayerFilter(
            myu::Physics::Details::Layers::MOVING),
        {}, {},
        myu::PhysicsEngine::getInstance().getTempAllocator());
    auto real_position = character->GetPosition();
    auto real_vel = character->GetLinearVelocity();
    auto head_rot = intent.getHeadDirection();
    PlayerState::PlayerUpdate update = PlayerState::PlayerUpdate(
        myu::math::Vec3::JPHToMyuVec3(real_position),
        myu::math::Vec3::JPHToMyuVec3(real_vel),
        head_rot
    );
    assert(real_position.Length() < 20000.0f); // prevent insane position
    update.tick = Server::instance().getTick();
    GameContext::Instance().addPlayerUpdateHistory(
        m_clientID,
        update
    );
}
