#include "Server.h"
#include "core/GameContext.h"
#include "core/PurchaseSystem.h"
#include "util/getTime.h"

bool PurchaseSystem::processPurchase(ClientID client, Weapon weapon) {
    //先判断是主武器还是副武器，检查对应武器是否已有，已有则覆盖
    //根据client id获取玩家金钱
    PlayerState *state = GameContext::Instance().GetPlayer(client);
    if (!state) {
        spdlog::error("PurchaseSystem::processPurchase: Player not found for client {}", client);
        return false;
    }
    //获取对应的武器实例

    auto new_weapon_instance = CreateWeapon(weapon);
    if (!new_weapon_instance || !new_weapon_instance->config) {
        spdlog::warn("玩家{}尝试购买不存在的武器", state->name);
        return false;
    }

    //获取玩家已有的武器对应网络枚举
    moe::net::Weapon net_primary_weapon = moe::net::Weapon::Weapon_WEAPON_NONE;
    moe::net::Weapon net_secondary_weapon = moe::net::Weapon::Weapon_WEAPON_NONE;
    if (state->primary) {
        net_primary_weapon = parseToNetWeapon(state->primary->config->weapon_id);
    }
    if (state->secondary) {
        net_secondary_weapon = parseToNetWeapon(state->secondary->config->weapon_id);
    }

    //获取玩家金钱和武器价格
    int& currentMoney = state->money;
    int weapon_cost = new_weapon_instance->config->price;

    //如果无法购买，则将玩家之前的武器类型传回客户端，告知无法购买
    if (currentMoney < weapon_cost) {
        spdlog::info("玩家{}金钱不足，无法购买武器{}", state->name, new_weapon_instance->config->name);
        //获取玩家自身武器的类型
        flatbuffers::FlatBufferBuilder fbb;
        auto event = moe::net::CreatePurchaseEvent(
            fbb,
            net_primary_weapon,
            net_secondary_weapon,
            false
        );

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
        SendPacket purchase_fail_packet = SendPacket(client, CH_RELIABLE, fbb.GetBufferSpan(), true);
        myu::NetWork::getInstance().pushPacket(purchase_fail_packet);
        return false;
    }

    //可以购买
    //扣除玩家金钱
    currentMoney -= weapon_cost;
    WeaponType weapon_type = new_weapon_instance->config->type;
    if (weapon_type == WeaponType::PRIMARY) {
        spdlog::info("玩家{}购买主武器{}成功，剩余金钱{}", state->name, new_weapon_instance->config->name, currentMoney);
        state->primary = std::move(new_weapon_instance);
    } else if (weapon_type == WeaponType::SECONDARY) {
        spdlog::info("玩家{}购买副武器{}成功，剩余金钱{}", state->name, new_weapon_instance->config->name, currentMoney);
        state->secondary = std::move(new_weapon_instance);
    }

    //更新网络武器类型
    if (state->primary) {
        net_primary_weapon = parseToNetWeapon(state->primary->config->weapon_id);
    }
    if (state->secondary) {
        net_secondary_weapon = parseToNetWeapon(state->secondary->config->weapon_id);
    }
    flatbuffers::FlatBufferBuilder fbb;
    auto event = moe::net::CreatePurchaseEvent(
        fbb,
        net_primary_weapon,
        net_secondary_weapon,
        true,
        currentMoney
    );

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
    SendPacket purchase_success_packet = SendPacket(client, CH_RELIABLE, fbb.GetBufferSpan(), true);
    myu::NetWork::getInstance().pushPacket(purchase_success_packet);
    return true;
}
