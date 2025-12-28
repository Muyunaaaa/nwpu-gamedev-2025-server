#include "core/GameContext.h"

#include <Core/FileWriter.hpp>

#include "config.h"
#include "Server.h"
#include "state/RoomContext.h"
#include "entity/PlayerState.h"
#include "entity/Weapon.h"
#include "state/MatchController.h"
#include "util/getTime.h"

void GameContext::InitFromRoom() {
    players_.clear();

    const auto &room = RoomContext::getInstance();

    for (const auto &[id, info]: room.players) {
        PlayerState ps;
        ps.client_id = id;
        ps.name = info.name;
        ps.team = info.team;

        ps.health = 100;
        ps.money = 800;
        ps.weapon_slot = WeaponSlot::SECONDARY;

        if (ps.team == PlayerTeam::CT) {
            ps.secondary = CreateWeapon(Weapon::USP);
        } else {
            ps.secondary = CreateWeapon(Weapon::GLOCK);
        }
        ps.weapon_slot = WeaponSlot::SECONDARY;
        players_.emplace(id, std::move(ps));
    }

    spdlog::info("GameContext initialized with {} players", players_.size());
}

void GameContext::Reset() {
    players_.clear();
    spdlog::info("GameContext reset");
}

const std::unordered_map<ClientID, PlayerState> &GameContext::Players() const {
    return players_;
}

PlayerState *GameContext::GetPlayer(ClientID id) {
    auto it = players_.find(id);
    return it == players_.end() ? nullptr : &it->second;
}

void GameContext::addDefuseAndReward(ClientID playerID) {
    players_[playerID].defuse++;
    addMoneyToPlayer(playerID, Config::match::DEFUSE_PRIZE);
}

void GameContext::addPlantAndReward(ClientID playerID) {
    players_[playerID].plants++;
    addMoneyToPlayer(playerID, Config::match::PLANT_PRIZE);
}

void GameContext::addDeath(ClientID playerID) {
    players_[playerID].deaths++;
}

void GameContext::addKillAndReward(ClientID playerID) {
    players_[playerID].kills++;
    addMoneyToPlayer(playerID, Config::match::KILL_PRIZE);
}

void GameContext::addMoneyToPlayer(ClientID playerID, int amount) {
    if (players_[playerID].money + amount > Config::match::MAX_BALANCE) {
        players_[playerID].money = Config::match::MAX_BALANCE;
    } else {
        players_[playerID].money += amount;
    }
}

void GameContext::playerShotted(ClientID Attacker, ClientID Victim, float damage) {
    //受击者死亡
    if (players_[Victim].health - damage <= 0) {
        players_[Victim].health = 0;
        setPlayerDied(Victim);
        addKillAndReward(Attacker);
        players_[Victim].killer = Attacker;
    } else {
        players_[Victim].health -= damage;
    }
    //添加记录
    players_[Attacker].shot_records.push_back(PlayerState::ShotRecord{
        .attacker = Attacker,
        .victim = Victim,
        .damage = damage
    });
    players_[Victim].damage_records.push_back(PlayerState::ShotRecord{
        .attacker = Attacker,
        .victim = Victim,
        .damage = damage
    });
}

void GameContext::setPlayerDied(ClientID playerID) {
    players_[playerID].alive = false;
    addDeath(playerID);
    players_[playerID].primary = CreateWeapon(Weapon::WEAPON_NONE);
    if (players_[playerID].team == PlayerTeam::CT) {
        players_[playerID].secondary = CreateWeapon(Weapon::USP);
        spdlog::info("玩家{}死亡，将其装备恢复为USP", players_[playerID].name);
    } else if (players_[playerID].team == PlayerTeam::T) {
        players_[playerID].secondary = CreateWeapon(Weapon::GLOCK);
        spdlog::info("玩家{}死亡，将其装备恢复为GLOCK", players_[playerID].name);
    } else {
        spdlog::error("玩家死亡时无队伍");
    }
}

void GameContext::addPlayerUpdateHistory(ClientID playerID, const PlayerState::PlayerUpdate update) {
    players_[playerID].position_history.push(update);
}

void GameContext::resetARound() {
    for (auto &[id, state]: players_) {
        state.alive = true;
        state.health = Config::player::MAX_HEALTH;
        if (MatchController::Instance().winner_team != PlayerTeam::NONE) {
            if (MatchController::Instance().winner_team == state.team) {
                addMoneyToPlayer(id, Config::match::WIN_PRIZE);
            } else {
                addMoneyToPlayer(id, Config::match::LOSE_PRIZE);
            }
        }

        state.killer = 0;
        state.weapon_slot = WeaponSlot::SECONDARY;

        if (state.team == PlayerTeam::T) {
            state.position_history.push(PlayerState::PlayerUpdate(
                    Config::match::C4_DEFAULT_PLANT_POSITION_T_SIDE,
                    myu::math::Vec3(0, 0, 0),
                    Config::match::DEFULAT_T_HEAD_ROTATION
                )
            );
        } else if (state.team == PlayerTeam::CT) {
            state.position_history.push(PlayerState::PlayerUpdate(
                    Config::match::C4_DEFAULT_PLANT_POSITION_CT_SIDE,
                    myu::math::Vec3(0, 0, 0),
                    Config::match::DEFULAT_CT_HEAD_ROTATION
                )
            );
        } else {
            spdlog::error("玩家{}无队伍，无法重置位置", state.name);
        }

        //检测所有装备为默认装备的玩家，广播购买事件
        flatbuffers::FlatBufferBuilder fbb;
        flatbuffers::Offset<moe::net::PurchaseEvent> event;

        if (state.team == PlayerTeam::CT) {
            if (state.primary == CreateWeapon(Weapon::WEAPON_NONE)
                && state.secondary == CreateWeapon(Weapon::USP)
            ) {
                spdlog::info("玩家{}回合开始时装备为默认装备，广播购买事件", state.name);
                event = moe::net::CreatePurchaseEvent(
                    fbb,
                    moe::net::Weapon::Weapon_WEAPON_NONE,
                    moe::net::Weapon::Weapon_USP,
                    true
                );
            }
        } else if (state.team == PlayerTeam::T) {
            if (state.primary == CreateWeapon(Weapon::WEAPON_NONE)
                && state.secondary == CreateWeapon(Weapon::GLOCK)
            ) {
                spdlog::info("玩家{}回合开始时装备为默认装备，广播购买事件", state.name);
                event = moe::net::CreatePurchaseEvent(
                    fbb,
                    moe::net::Weapon::Weapon_WEAPON_NONE,
                    moe::net::Weapon::Weapon_GLOCK,
                    true
                );
            }
        } else {
            spdlog::error("玩家{}无队伍，无法检测默认装备", state.name);
        }

        auto header = moe::net::CreateReceivedHeader(
            fbb,
            Server::instance().getTick(),
            myu::time::now_ms()
        );

        auto eventWrapper = moe::net::CreateGameEvent(
            fbb,
            moe::net::EventData::EventData_PurchaseEvent,
            event.Union()
        );

        auto _msg = moe::net::CreateReceivedNetMessage(
            fbb,
            header,
            moe::net::ReceivedPacketUnion::ReceivedPacketUnion_GameEvent,
            eventWrapper.Union()
        );
        fbb.Finish(_msg);
        SendPacket purchase_fail_packet = SendPacket(id, CH_RELIABLE, fbb.GetBufferSpan(), true);
        myu::NetWork::getInstance().pushPacket(purchase_fail_packet);
    }
}


int GameContext::countLifes() {
    int count = 0;
    for (const auto &[id, state]: players_) {
        if (state.alive) {
            count++;
        }
    }
    return count;
}

int GameContext::countLifes(PlayerTeam team) {
    int count = 0;
    for (const auto &[id, state]: players_) {
        if (state.alive && state.team == team) {
            count++;
        }
    }
    return count;
}

void GameContext::flushShotRecords() {
    std::string filepath = "data/shot_records.txt";

    std::string content;
    for (auto &[id, state]: players_) {
        content += std::format("Player ID: {}\n", id);
        for (const auto &record: state.shot_records) {
            // 假设 record 有一些可以打印的属性
            content += std::format("  - Shot at: [x,y,z...]\n");
        }
    }

    if (!content.empty()) {
        bool success = moe::FileWriter::writeToFile(filepath, content);
        if (!success) {
            spdlog::warn("无法写入射击记录到文件 {}", filepath);
        }
    }

    for (auto &[id, state]: players_) {
        state.damage_records.clear();
        state.shot_records.clear();
    }
}

void GameContext::playerSnyc() {
    flatbuffers::FlatBufferBuilder fbb;
    auto player_updates = std::vector<flatbuffers::Offset<moe::net::PlayerUpdate>>();
    for (const auto &[id, state]: players_) {
        if (state.position_history.empty()) {
            spdlog::warn("出现了玩家 {} 无位置历史记录的情况，跳过同步", id);
            continue;
        }
        auto& history = state.position_history.back();
        moe::net::Vec3 pos = history.position.ToMOEVec3();
        moe::net::Vec3 vel = history.velocity.ToMOEVec3();
        moe::net::Vec3 head = history.head.ToMOEVec3();
        //todo:提醒lzm重新编译
        auto new_update = moe::net::CreatePlayerUpdate(
            fbb,
            id,
            &pos,
            &vel,
            &head,
            moe::net::PlayerMotionState::PlayerMotionState_NORMAL,
            parseToNetWeapon(state.getCurrentWeapon())
        );
        player_updates.push_back(new_update);
    }
    auto event = moe::net::CreateAllPlayerUpdate(
        fbb,
        fbb.CreateVector(player_updates)
    );
    auto header = moe::net::CreateReceivedHeader(
        fbb,
        Server::instance().getTick(),
        myu::time::now_ms()
    );
    auto msg = moe::net::CreateReceivedNetMessage(
        fbb,
        header,
        moe::net::ReceivedPacketUnion::ReceivedPacketUnion_AllPlayerUpdate,
        event.Union()
    );
    fbb.Finish(msg);
    SendPacket player_update_packet = SendPacket(-1, CH_STATE, fbb.GetBufferSpan(), false);
    myu::NetWork::getInstance().broadcast(player_update_packet);
}
