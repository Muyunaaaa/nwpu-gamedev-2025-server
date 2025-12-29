#pragma once
#include <atomic>
#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "protocol/Events_generated.h"
#include "protocol/receiveGamingPacket_generated.h"
#include "protocol//Weapon_generated.h"

enum class Weapon{
    WEAPON_NONE = 0,
    GLOCK = 1,
    USP = 2,
    DEAGLE = 3,
    AK47 = 4,
    M4A1 = 5,

};

enum class WeaponSlot {
    PRIMARY = 0,
    SECONDARY = 1,
};

inline std::string toString(Weapon weapon) {
    switch (weapon) {
        case Weapon::GLOCK:
            return "GLOCK";
        case Weapon::USP:
            return "USP";
        case Weapon::DEAGLE:
            return "DEAGLE";
        case Weapon::AK47:
            return "AK47";
        case Weapon::M4A1:
            return "M4A1";
        case Weapon::WEAPON_NONE:
            default:
                return "WEAPON_NONE";
    }
}

enum WeaponType {
    PRIMARY,
    SECONDARY,
};

struct WeaponConfig {
    Weapon weapon_id;      // 配置中的唯一ID
    WeaponType type;        // 武器类型
    std::string name;        // 枪名
    int price;               // 价格
    float hit_head_damage;
    float hit_body_damage;
    float fire_rate;         // 射速
    int clip_size;           // 弹夹容量
};

using InstanceID = uint64_t;

struct WeaponInstance {
    InstanceID instance_id;              // 每把枪唯一
    const WeaponConfig* config;           // 指向预设（关键）
    int ammo_in_clip;                     // 当前弹夹
};

class WeaponConfigManager {
public:
    static WeaponConfigManager& Instance();

    void LoadFromFile(const std::string& path);

    const WeaponConfig* Get(Weapon weapon) const;

private:
    std::unordered_map<Weapon, WeaponConfig> configs_;
};

inline std::unique_ptr<WeaponInstance> CreateWeapon(Weapon weapon) {
    static std::atomic<uint64_t> next_instance_id{1};

    const WeaponConfig* config =
        WeaponConfigManager::Instance().Get(weapon);

    if (config == nullptr) {
        return nullptr;
    }

    WeaponInstance instance = WeaponInstance{
        .instance_id = next_instance_id++,
        .config = config,
        .ammo_in_clip = config->clip_size
    };
    return std::make_unique<WeaponInstance>(instance);
}

inline moe::net::Weapon parseToNetWeapon(Weapon weapon) {
    switch (weapon) {
        case Weapon::GLOCK:
            return moe::net::Weapon::Weapon_GLOCK;
        case Weapon::USP:
            return moe::net::Weapon::Weapon_USP;
        case Weapon::DEAGLE:
            return moe::net::Weapon::Weapon_DEAGLE;
        case Weapon::AK47:
            return moe::net::Weapon::Weapon_AK47;
        case Weapon::M4A1:
            return moe::net::Weapon::Weapon_M4A1;
        case Weapon::WEAPON_NONE:
        default:
            return moe::net::Weapon::Weapon_WEAPON_NONE;
    }
}

inline Weapon parseNetWeaponToLocalWeapon(myu::net::Weapon weapon) {
    switch (weapon) {
        case myu::net::Weapon::Weapon_GLOCK:
            return Weapon::GLOCK;
        case myu::net::Weapon::Weapon_USP:
            return Weapon::USP;
        case myu::net::Weapon::Weapon_DEAGLE:
            return Weapon::DEAGLE;
        case myu::net::Weapon::Weapon_AK47:
            return Weapon::AK47;
        case myu::net::Weapon::Weapon_M4A1:
            return Weapon::M4A1;
        case myu::net::Weapon::Weapon_WEAPON_NONE:
        default:
            return Weapon::WEAPON_NONE;
    }
}

inline WeaponSlot parseNetWeaponSlotToLocalWeaponSlot(myu::net::WeaponSlot slot) {
    switch (slot) {
        case myu::net::WeaponSlot::WeaponSlot_SLOT_PRIMARY:
            return WeaponSlot::PRIMARY;
        case myu::net::WeaponSlot::WeaponSlot_SLOT_SECONDARY:
            return WeaponSlot::SECONDARY;
        default:
            return WeaponSlot::SECONDARY;
    }
}

inline WeaponType parseWeaponToType(Weapon weapon) {
    switch (weapon) {
        case Weapon::GLOCK:
        case Weapon::USP:
        case Weapon::DEAGLE:
            return WeaponType::SECONDARY;
        case Weapon::AK47:
        case Weapon::M4A1:
            return WeaponType::PRIMARY;
        case Weapon::WEAPON_NONE:
        default:
            return WeaponType::SECONDARY;
    }
}
