#include "core/GameContext.h"

#include "state/RoomContext.h"
#include "core/GameContext.h"

#include "entity/PlayerState.h"
#include "entity/Weapon.h"

//TODO:检查
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

        if (ps.team == PlayerTeam::CT) {
            ps.secondary = std::make_unique<WeaponInstance>(
                CreateWeapon(Weapon::USP)
            );
        } else {
            ps.secondary = std::make_unique<WeaponInstance>(
                CreateWeapon(Weapon::GLOCK)
            );
        }
        ps.current_weapon = std::move(ps.secondary);
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


