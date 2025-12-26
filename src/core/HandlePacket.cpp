#include "core/HandlePacket.h"

#include "Server.h"
#include "protocol/receiveGamingPacket_generated.h"
#include "state/MatchController.h"
#include "state/RoomContext.h"

#include "util/getTime.h"

void HandlePacket::handlePlayerReady(ClientID client_id, const myu::net::PlayerInfo *msg) {
    const auto *info = msg;

    std::string player_name = info->name()->c_str();
    bool is_ready = info->ready();

    auto &room = RoomContext::getInstance();

    auto it = room.players.find(client_id);
    if (it == room.players.end()) {
        if (RoomContext::getInstance().getReadyCount() == RoomContext::TARGET_PLAYERS) {
            spdlog::warn("房间已满，拒绝新玩家 {} (ID: {}) 加入对局", player_name, client_id);
            return;
        }
        PlayerTeam assigned_team = room.getTeamWithLessPlayers();

        PlayerInfo new_player;
        new_player.id = client_id;
        new_player.name = player_name;
        new_player.isReady = is_ready;
        new_player.team = assigned_team;

        room.players[client_id] = new_player;
        room.players_just_joined.push_back(client_id);

        spdlog::info("玩家进入服务器,加入: {} (ID: {}) 队伍 {}",
                     player_name, client_id, toString(assigned_team));
    } else {
        it->second.isReady = is_ready;
        it->second.name = player_name;

        spdlog::debug("Player {} (ID: {}) updated ready status to {}",
                      player_name, client_id, is_ready);
    }
}

void HandlePacket::handleFire(ClientID, const myu::net::FirePacket *msg) {
    /*TODO:先对所有数据包进行处理，然后将处理的后的数据包压入环形队列（包括tick信息），做向前舍弃，然后进行计算更新玩家数据存储，然后广播*/
    //TODO:考虑是否要做物理判定
    //parse
    //calculate
    //update
}

void HandlePacket::handleMove(ClientID, const myu::net::MovePacket *msg) {
    /*TODO:先对所有数据包进行处理，然后将处理的后的数据包压入环形队列（包括tick信息），做向前舍弃，然后进行计算更新玩家数据存储，然后广播*/
}

void HandlePacket::handlePurchase(ClientID, const myu::net::PurchaseEvent *msg) {
}

void HandlePacket::handlePlant(ClientID id, const myu::net::PlantBombEvent *msg) {
    if (!MatchController::Instance().plant_able()) {
        return;
    }
    //这里哪怕客户端误发了安放炸弹请求，比如说在非交火阶段发安放炸弹请求，状态机进入交火状态会清空炸弹安放状态
    MatchController::Instance().plantC4(parseNetBombSiteToBombSite(msg->bombSite()));
    spdlog::info("炸弹被安放");
    //发送消息
    uint16_t bomb_site = parseToNetBombSite(MatchController::Instance().c4_plant_site);
    if (bomb_site == -1) {
        spdlog::warn("炸弹安放位置解析错误");
        return;
    }
    if (bomb_site == 3) {
        spdlog::error("炸弹安放位置为None，逻辑出现错误");
        return;
    }
    flatbuffers::FlatBufferBuilder fbb;

    auto event = moe::net::CreateBombPlantedEvent(
        fbb,
        id,
        bomb_site
    );

    auto header = moe::net::CreateReceivedHeader(
        fbb,
        Server::instance().getTick(),
        myu::time::now_ms()
    );
    auto _msg = moe::net::CreateReceivedNetMessage(
        fbb,
        header,
        moe::net::ReceivedPacketUnion::ReceivedPacketUnion_GameEvent,
        event.Union()
    );
    fbb.Finish(_msg);
    SendPacket bomb_plant_packet = SendPacket(-1, CH_RELIABLE, fbb.GetBufferSpan(), true);
    myu::NetWork::getInstance().broadcast(bomb_plant_packet);
}

void HandlePacket::handleDefuse(ClientID id, const myu::net::DefuseBombEvent *msg) {
    if (!MatchController::Instance().defuse_able()) {
        return;
    }
    MatchController::Instance().defuseC4();
    spdlog::info("炸弹被拆除");
    //这里哪怕客户端误发了拆弹请求，比如说在非交火阶段发拆弹请求，状态机进入交火状态会清空炸弹安放状态
    //发送消息
    flatbuffers::FlatBufferBuilder fbb;

    auto event = moe::net::CreateBombDefusedEvent(
        fbb,
        id
    );

    auto header = moe::net::CreateReceivedHeader(
        fbb,
        Server::instance().getTick(),
        myu::time::now_ms()
    );

    auto _msg = moe::net::CreateReceivedNetMessage(
        fbb,
        header,
        moe::net::ReceivedPacketUnion::ReceivedPacketUnion_GameEvent,
        event.Union()
    );
    fbb.Finish(_msg);
    SendPacket bomb_defused_packet = SendPacket(-1, CH_RELIABLE, fbb.GetBufferSpan(), true);
    myu::NetWork::getInstance().broadcast(bomb_defused_packet);
}
