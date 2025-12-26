#include "entity/Weapon.h"

#include <fstream>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>


WeaponConfigManager& WeaponConfigManager::Instance() {
    static WeaponConfigManager instance;
    return instance;
}

void WeaponConfigManager::LoadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        spdlog::error("Failed to open weapon config file: {}", path);
        return;
    }

    nlohmann::json j;
    file >> j;

    configs_.clear();

    if (!j.contains("weapons") || !j["weapons"].is_array()) {
        spdlog::error("Invalid weapon config format");
        return;
    }

    for (const auto& w : j["weapons"]) {
        WeaponConfig cfg;

        cfg.weapon_id = w.at("id").get<WeaponID>();
        cfg.name      = w.at("name").get<std::string>();
        cfg.price     = w.at("price").get<int>();
        cfg.damage    = w.at("damage").get<int>();
        cfg.fire_rate = w.at("fire_rate").get<float>();
        cfg.clip_size = w.at("clip_size").get<int>();

        configs_.emplace(cfg.weapon_id, std::move(cfg));
    }

    spdlog::info("Loaded {} weapon configs", configs_.size());
}

const WeaponConfig* WeaponConfigManager::Get(Weapon weapon) const {
    WeaponID id = static_cast<WeaponID>(weapon);

    auto it = configs_.find(id);
    if (it == configs_.end()) {
        return nullptr;
    }

    return &it->second;
}
