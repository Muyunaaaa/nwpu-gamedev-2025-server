//todo:完成物理引擎，能够根据传入的物体状态进行物理计算，返回新的物体状态
#include "physics/PhysicsEnginee.h"
void myu::PhysicsEngine::init() {
    spdlog::info("Initializing physics engine...");

    JPH::RegisterDefaultAllocator();

    JPH::Trace = Physics::Details::TraceImpl;
    JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = Physics::Details::AssertFailedImpl;)

    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

    spdlog::info("Jolt Physics version: {}.{}.{}",
                 JPH_VERSION_MAJOR,
                 JPH_VERSION_MINOR,
                 JPH_VERSION_PATCH);
    spdlog::info("Initializing physics utilities...");
    m_broadPhaseLayerInterface = std::make_unique<Physics::Details::BPLayerInterfaceImpl>();
    m_objectVsBroadPhaseLayerFilter = std::make_unique<Physics::Details::ObjectVsBroadPhaseLayerFilterImpl>();
    m_objectLayerFilter = std::make_unique<Physics::Details::ObjectLayerFilterImpl>();
    m_tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(1024 * 1024);
    m_jobSystem = std::make_unique<JPH::JobSystemThreadPool>(
            JPH::cMaxPhysicsJobs,
            JPH::cMaxPhysicsBarriers);

    spdlog::info("Initializing physics system...");
    m_physicsSystem = std::make_unique<JPH::PhysicsSystem>();
    m_physicsSystem->Init(
            MAX_BODIES, NUM_BODY_MUTEXES,
            MAX_BODY_PAIRS, MAX_CONTACT_CONSTRAINTS,
            *m_broadPhaseLayerInterface,
            *m_objectVsBroadPhaseLayerFilter,
            *m_objectLayerFilter);

    spdlog::info("Physics system initialized with {} max bodies, {} max body pairs, {} max contact constraints",
                 MAX_BODIES, MAX_BODY_PAIRS, MAX_CONTACT_CONSTRAINTS);
    m_initialized = true;

    spdlog::info("Physics engine initialized");

    //TODO: 初始化物理世界,载入模型
}

void myu::PhysicsEngine::destroy() {
    if (!m_initialized){
        spdlog::warn("Physics engine not initialized, skipping destroy");
        return;
    }
    spdlog::info("Shutting down physics engine...");

    JPH::UnregisterTypes();
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;

    spdlog::info("Physics engine shut down");
    m_initialized = false;
}