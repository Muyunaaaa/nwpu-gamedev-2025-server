#include "Config.h"
#include <toml++/toml.h>
#include <iostream>

namespace Config {

    namespace match {
        int WIN_PRIZE = 3000;
        int LOSE_PRIZE = 2000;
        int KILL_PRIZE = 300;
        int PLANT_PRIZE = 200;
        int DEFUSE_PRIZE = 200;
        int MAX_BALANCE = 6000;
    }

    namespace player {
        float MAX_HEALTH = 100.0f;
    }

    namespace network {
        int SERVER_PORT = 1234;
        int MAX_CLIENTS = 32;
    }

    namespace room {
        int TARGET_PLAYERS = 2;
        int MAX_ROUNDS = 2;
    }

    // ===============================
    // 配置加载函数
    // ===============================

    static void LoadMatchConfig(const toml::table& tbl) {
        if (auto v = tbl["win_prize"].value<int>())
            match::WIN_PRIZE = *v;
        if (auto v = tbl["lose_prize"].value<int>())
            match::LOSE_PRIZE = *v;
        if (auto v = tbl["kill_prize"].value<int>())
            match::KILL_PRIZE = *v;
        if (auto v = tbl["plant_prize"].value<int>())
            match::PLANT_PRIZE = *v;
        if (auto v = tbl["defuse_prize"].value<int>())
            match::DEFUSE_PRIZE = *v;
        if (auto v = tbl["max_balance"].value<int>())
            match::MAX_BALANCE = *v;
    }

    static void LoadPlayerConfig(const toml::table& tbl) {
        if (auto v = tbl["max_health"].value<float>())
            player::MAX_HEALTH = *v;
    }

    static void LoadNetworkConfig(const toml::table& tbl) {
        if (auto v = tbl["server_port"].value<int>())
            network::SERVER_PORT = *v;
        if (auto v = tbl["max_clients"].value<int>())
            network::MAX_CLIENTS = *v;
    }

    static void LoadRoomConfig(const toml::table& tbl) {
        if (auto v = tbl["target_players"].value<int>())
            room::TARGET_PLAYERS = *v;
        if (auto v = tbl["max_rounds"].value<int>())
            room::MAX_ROUNDS = *v;
    }

    // ===============================
    // 对外统一入口
    // ===============================
    void LoadFromToml(const std::string& path) {
        try {
            toml::table tbl = toml::parse_file(path);

            if (auto t = tbl["match"].as_table())
                LoadMatchConfig(*t);

            if (auto t = tbl["player"].as_table())
                LoadPlayerConfig(*t);

            if (auto t = tbl["network"].as_table())
                LoadNetworkConfig(*t);

            if (auto t = tbl["room"].as_table())
                LoadRoomConfig(*t);

        } catch (const toml::parse_error& err) {
            std::cerr << "[Config] Failed to load " << path << "\n"
                      << err.description() << "\n";
        }
    }

} // namespace Config
