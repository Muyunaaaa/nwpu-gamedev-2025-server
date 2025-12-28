#pragma once
#include "Physics/JoltIncludes.hpp"

class PhysicsMapController {
public:
    // 获取单例实例
    static PhysicsMapController& Instance() {
        static PhysicsMapController instance;
        return instance;
    }

    // 禁止拷贝和赋值，确保单例唯一性
    PhysicsMapController(const PhysicsMapController&) = delete;
    PhysicsMapController& operator=(const PhysicsMapController&) = delete;

    void createMapBody();
    void destroyMapBody();

private:
    // 将构造函数私有化
    PhysicsMapController() = default;
    ~PhysicsMapController() = default;

    JPH::BodyID m_playgroundBody;
};