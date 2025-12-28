#include "physics/PhysicsMapController.h"

#include <Physics/GltfColliderFactory.hpp>

#include "config.h"
#include "physics/PhysicsEnginee.h"

void PhysicsMapController::createMapBody() {
    auto collider=
        moe::Physics::GltfColliderFactory::shapeFromGltf(Config::physics::PLAYGROUND_GLTF);
    JPH::BodyCreationSettings settings = JPH::BodyCreationSettings(
            collider,
            JPH::RVec3(0.0f, 0.0f, 0.0f),
            JPH::Quat::sIdentity(),
            JPH::EMotionType::Static,
            myu::Physics::Details::Layers::NON_MOVING);

    auto& bodyInterface = myu::PhysicsEngine::getInstance().getPhysicsSystem().GetBodyInterface();
    m_playgroundBody = bodyInterface.CreateAndAddBody(settings, JPH::EActivation::DontActivate);
}

void PhysicsMapController::destroyMapBody() {
    spdlog::info("Removing playground collider");

    auto& bodyInterface = myu::PhysicsEngine::getInstance().getPhysicsSystem().GetBodyInterface();
    bodyInterface.RemoveBody(m_playgroundBody);
    bodyInterface.DestroyBody(m_playgroundBody);
}

