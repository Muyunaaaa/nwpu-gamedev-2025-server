//
// Created by Administrator on 25-12-26.
//

#ifndef ECONOMYSYSTEM_H
#define ECONOMYSYSTEM_H
#include <unordered_map>

#include "entity/Weapon.h"
#include "network/NetPacket.h"

#pragma once

#include "entity/Weapon.h"
#include "network/NetPacket.h"

class EconomySystem {
public:
    // 单例访问点
    static EconomySystem& Instance() {
        static EconomySystem instance;
        return instance;
    }

    // 发出购买请求
    bool processPurchase(ClientID client, Weapon weapon);

private:
    EconomySystem() = default;
    ~EconomySystem() = default;

    EconomySystem(const EconomySystem&) = delete;
    EconomySystem& operator=(const EconomySystem&) = delete;
};
