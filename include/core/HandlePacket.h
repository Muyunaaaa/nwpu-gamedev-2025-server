#pragma once
#include "network/NetPacket.h"
#include "protocol/receiveGamingPacket_generated.h"

struct HandlePacket {
    static void handleMove(ClientID, const myu::net::MovePacket* msg);
    static void handleFire(ClientID, const myu::net::PacketHeader* header, const myu::net::FirePacket* msg);
    static void handlePurchase(ClientID, const myu::net::PurchaseEvent* msg);
    static void handleDefuse(ClientID, const myu::net::DefuseBombEvent* msg);
    static void handlePlant(ClientID, const myu::net::PlantBombEvent* msg);
    static void handlePlayerReady(ClientID, const myu::net::PlayerInfo* msg);
};
