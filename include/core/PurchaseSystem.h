#pragma once

#include "entity/Weapon.h"
#include "network/NetPacket.h"

class PurchaseSystem {
public:
    // 单例访问点
    static PurchaseSystem& Instance() {
        static PurchaseSystem instance;
        return instance;
    }

    void purchaseItem(int playerId, int itemId);

    bool processPurchase(ClientID client, Weapon weapon);

private:
    PurchaseSystem() = default;
    ~PurchaseSystem() = default;

    PurchaseSystem(const PurchaseSystem&) = delete;
    PurchaseSystem& operator=(const PurchaseSystem&) = delete;
};
