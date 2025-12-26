#pragma once
#include <atomic>
#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>
enum class Weapon{
    WEAPON_NONE = 0,
    GLOCK = 1,
    USP = 2,
    DEAGLE = 3,
    AK47 = 4,
    M4A1 = 5,
};

using WeaponID = uint32_t;

struct WeaponConfig {
    WeaponID weapon_id;      // 配置中的唯一ID
    std::string name;        // 枪名
    int price;               // 价格
    int damage;              // 单发伤害
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
    std::unordered_map<WeaponID, WeaponConfig> configs_;
};

inline WeaponInstance CreateWeapon(Weapon weapon) {
    static std::atomic<uint64_t> next_instance_id{1};

    const WeaponConfig* config =
        WeaponConfigManager::Instance().Get(weapon);

    assert(config);

    return WeaponInstance{
        .instance_id = next_instance_id++,
        .config = config,
        .ammo_in_clip = config->clip_size
    };
}