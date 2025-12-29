#include "core/HandlePacket.h"

#include "Server.h"
#include "core/GameContext.h"
#include "core/PurchaseSystem.h"
#include "physics/EvalPhysDamage.h"
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
        if (RoomContext::getInstance().getReadyCount() == Config::room::TARGET_PLAYERS) {
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

void HandlePacket::handleFire(ClientID id, const myu::net::PacketHeader *header, const myu::net::FirePacket *msg) {
    ClientID client_id = msg->tempId();
    auto player = GameContext::Instance().GetPlayer(client_id);
    if (!player) {
        spdlog::error("Received FirePacket for unknown Client ID {}", client_id);
        return;
    }

    auto &seq = player->fire_sequence_number;
    if (seq > msg->sequence()) {
        spdlog::error("Received out-of-order FirePacket from Client {}: sequence={}, expected greater than {}",
                      id,
                      msg->sequence(),
                      seq);
        return;
    }
    seq = std::max(seq, msg->sequence());
    myu::math::Vec3 fire_point = myu::math::Vec3(
        msg->originX(),
        msg->originY(),
        msg->originZ()
    );
    //注意这里是归一化向量，但在物理的recast中，需要的是非归一化向量，需要带上射程
    myu::math::Vec3 fire_dir = myu::math::Vec3(
        msg->dirX(),
        msg->dirY(),
        msg->dirZ()
    );
    WeaponSlot weapon_slot = parseNetWeaponSlotToLocalWeaponSlot(msg->weaponSlot());
    ClientID shooter_id = msg->tempId();
    uint64_t shot_tick = header->clientTick();
    PhysRaycastHit raycast_hit = PhysRaycastHit(
        fire_point,
        fire_dir,
        shooter_id
    );
    int shot_result = raycast_hit.isHit(shot_tick);
    if (shot_result == -1) {
        spdlog::info("玩家 {} 开火，未击中任何玩家", shooter_id);
        return;
    } else {
        GameContext::Instance().GetPlayer(shooter_id)->weapon_slot = weapon_slot;
        Weapon current_weapon = GameContext::Instance().GetPlayer(shooter_id)->getCurrentWeapon();
        if (current_weapon == Weapon::WEAPON_NONE) {
            spdlog::error("玩家 {} 开火时未持有任何武器", shooter_id);
            return;
        }
        float damage = GameContext::Instance().
                playerShotted(
                    shooter_id,
                    shot_result,
                    CreateWeapon(current_weapon).get()->config->hit_body_damage
                );
        Server::instance().fire_logger->info("玩家 {} 开火，击中玩家 {}，造成 {} 点伤害", shooter_id, shot_result, damage);
        Server::instance().fire_logger->info("玩家 {} 当前生命值 {}", shot_result,
                                             GameContext::Instance().GetPlayer(shot_result)->health);

        flatbuffers::FlatBufferBuilder fbb;
        if (GameContext::Instance().GetPlayer(shot_result)->position_history.empty()) {
            spdlog::warn("出现了玩家 {} 无位置历史记录的情况，跳过开火事件发布", shot_result);
        } else {
            myu::math::Vec3 shotter_pos = GameContext::Instance().GetPlayer(shot_result)->position_history.back().
                    position;
            auto event = moe::net::CreatePlayerOpenFireEvent(
                fbb,
                shooter_id,
                shotter_pos.x,
                shotter_pos.y,
                shotter_pos.z
            );

            auto header = moe::net::CreateReceivedHeader(
                fbb,
                Server::instance().getTick(),
                myu::time::now_ms()
            );

            auto eventWrapper = moe::net::CreateGameEvent(
                fbb,
                moe::net::EventData::EventData_PlayerOpenFireEvent,
                event.Union()
            );

            auto _msg = moe::net::CreateReceivedNetMessage(
                fbb,
                header,
                moe::net::ReceivedPacketUnion::ReceivedPacketUnion_GameEvent,
                eventWrapper.Union()
            );
            fbb.Finish(_msg);
            SendPacket shotter_fire_packet = SendPacket(-1, CH_RELIABLE, fbb.GetBufferSpan(), true);
            myu::NetWork::getInstance().broadcast(shotter_fire_packet);
        }

        if (GameContext::Instance().GetPlayer(shot_result)->health == 0) {
            flatbuffers::FlatBufferBuilder fbb;
            auto event = moe::net::CreatePlayerKilledEvent(
                fbb,
                shot_result,
                shooter_id,
                parseToNetWeapon(current_weapon)
            );

            auto header = moe::net::CreateReceivedHeader(
                fbb,
                Server::instance().getTick(),
                myu::time::now_ms()
            );

            auto eventWrapper = moe::net::CreateGameEvent(
                fbb,
                moe::net::EventData::EventData_PlayerKilledEvent,
                event.Union()
            );

            auto _msg = moe::net::CreateReceivedNetMessage(
                fbb,
                header,
                moe::net::ReceivedPacketUnion::ReceivedPacketUnion_GameEvent,
                eventWrapper.Union()
            );
            fbb.Finish(_msg);
            SendPacket player_killed_packet = SendPacket(-1, CH_RELIABLE, fbb.GetBufferSpan(), true);
            myu::NetWork::getInstance().broadcast(player_killed_packet);
        }
    }
}

void HandlePacket::handleMove(ClientID id, const myu::net::MovePacket *msg) {
    ClientID client_id = msg->tempId();
    auto player = GameContext::Instance().GetPlayer(client_id);
    if (!player) {
        spdlog::error("Received MovePacket for unknown Client ID {}", client_id);
        return;
    }

    auto &seq = player->move_sequence_number;
    if (seq > msg->sequence()) {
        spdlog::error("Received out-of-order MovePacket from Client {}: sequence={}, expected greater than {}",
                      id,
                      msg->sequence(),
                      seq);
        return;
    }
    seq = std::max(seq, msg->sequence());
    //调用计算函数计算位置信息放入环形队列
    myu::math::Vec3 move_dir = myu::math::Vec3(
        msg->moveX(),
        msg->moveY(),
        msg->moveZ()
    );
    float yaw_radian = msg->yawRadian();
    float pitch_radian = msg->pitchRadian();
    auto delta_time = std::chrono::duration<float, std::chrono::seconds::period>(1.0 / Config::server::TPS);
    InputIntent input_interface = InputIntent(
        move_dir.ToGLMVec3(),
        yaw_radian,
        pitch_radian
    );
    //TODO:测试成功后注释
    Server::instance().move_logger->info("Received MovePacket from Client {}: move_dir=({}, {}, {}), yaw={}, pitch={}",
                                         client_id,
                                         move_dir.x, move_dir.y, move_dir.z,
                                         yaw_radian,
                                         pitch_radian);
    GameContext::Instance().GetPlayer(client_id)->physics_controller->updateCharacterPhysics(
        delta_time.count(),
        input_interface
    );
}

void HandlePacket::handlePurchase(ClientID id, const myu::net::PurchaseEvent *msg) {
    spdlog::info("玩家 {} 请求购买武器 {}", id, toString(parseNetWeaponToLocalWeapon(msg->weapon())));
    if (!MatchController::Instance().purchase_able()) {
        return;
    }
    if (PurchaseSystem::Instance().processPurchase(id, parseNetWeaponToLocalWeapon(msg->weapon()))) {
        spdlog::info("玩家 {} 购买了武器 {}", id, toString(parseNetWeaponToLocalWeapon(msg->weapon())));
    } else {
        spdlog::info("玩家 {} 购买武器 {} 失败", id, toString(parseNetWeaponToLocalWeapon(msg->weapon())));
    }
}

void HandlePacket::handlePlant(ClientID id, const myu::net::PlantBombEvent *msg) {
    if (!MatchController::Instance().plant_able()) {
        return;
    }
    //这里哪怕客户端误发了安放炸弹请求，比如说在非交火阶段发安放炸弹请求，状态机进入交火状态会清空炸弹安放状态
    MatchController::Instance().plantC4(parseNetBombSiteToBombSite(msg->bombSite()));
    GameContext::Instance().addPlantAndReward(id);
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

    auto eventWrapper = moe::net::CreateGameEvent(
        fbb,
        moe::net::EventData::EventData_BombPlantedEvent,
        event.Union()
    );

    auto _msg = moe::net::CreateReceivedNetMessage(
        fbb,
        header,
        moe::net::ReceivedPacketUnion::ReceivedPacketUnion_GameEvent,
        eventWrapper.Union()
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
    GameContext::Instance().addDefuseAndReward(id);
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

    auto eventWrapper = moe::net::CreateGameEvent(
        fbb,
        moe::net::EventData::EventData_BombDefusedEvent,
        event.Union()
    );

    auto _msg = moe::net::CreateReceivedNetMessage(
        fbb,
        header,
        moe::net::ReceivedPacketUnion::ReceivedPacketUnion_GameEvent,
        eventWrapper.Union()
    );
    fbb.Finish(_msg);
    SendPacket bomb_defused_packet = SendPacket(-1, CH_RELIABLE, fbb.GetBufferSpan(), true);
    myu::NetWork::getInstance().broadcast(bomb_defused_packet);
}
