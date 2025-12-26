#include "core/HandlePacket.h"

#include "protocol/receiveGamingPacket_generated.h"
#include "state/RoomContext.h"

void HandlePacket::handlePlayerReady(ClientID client_id, const myu::net::PlayerInfo* msg) {
    const auto *info = msg;

    uint32_t player_id = client_id;
    std::string player_name = info->name()->c_str();
    bool is_ready = info->ready();

    auto &room = RoomContext::getInstance();

    auto it = room.players.find(player_id);
    if (it == room.players.end()) {
        if (RoomContext::getInstance().getReadyCount() == RoomContext::TARGET_PLAYERS) {
            spdlog::warn("房间已满，拒绝新玩家 {} (ID: {}) 加入对局", player_name, player_id);
            return;
        }
        Team assigned_team = room.getTeamWithLessPlayers();

        PlayerInfo new_player;
        new_player.id = player_id;
        new_player.name = player_name;
        new_player.isReady = is_ready;
        new_player.team = assigned_team;

        room.players[player_id] = new_player;
        room.players_just_joined.push_back(player_id);

        spdlog::info("New player joined: {} (ID: {}) assigned to team {}",
                     player_name, player_id, (int) assigned_team);
    } else {
        it->second.isReady = is_ready;
        it->second.name = player_name;

        spdlog::debug("Player {} (ID: {}) updated ready status to {}",
                      player_name, player_id, is_ready);
    }
}

void HandlePacket::handleFire(ClientID, const myu::net::FirePacket *msg) {
    //parse
    //calculate
    //update
}

void HandlePacket::handleMove(ClientID, const myu::net::MovePacket *msg) {

}

void HandlePacket::handlePurchase(ClientID, const myu::net::PurchaseEvent *msg) {

}

void HandlePacket::handlePlant(ClientID, const myu::net::PlantBombEvent *msg) {

}

void HandlePacket::handleDefuse(ClientID, const myu::net::DefuseBombEvent *msg) {

}



