#include "server.h"
#include "spdlog/spdlog.h"
#include <thread>

#include "flatbuffers/verifier.h"
#include "protocol/receiveGamingPacket_generated.h"
#include "state/MatchController.h"
#include "state/RoomContext.h"
#include "state/WaitState.h"
#include "state/WarmupState.h"

#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "core/HandlePacket.h"
#include "entity/Weapon.h"
#include "entity//Weapon.h"
#include "physics/PhysicsEnginee.h"
#include "protocol/Main_generated.h"
#include "util/getTime.h"

void init_logging() {
    spdlog::init_thread_pool(8192, 1);
    auto logger = spdlog::stdout_color_mt<spdlog::async_factory>("async_logger");
    spdlog::set_default_logger(logger);
    spdlog::flush_every(std::chrono::seconds(3));
}


void Server::start() {
    Config::LoadFromToml("settings.toml");
    auto &network = myu::NetWork::getInstance();
    myu::PhysicsEngine::getInstance().init();
    WeaponConfigManager::Instance().LoadFromFile("./config/weapons.json");
    network.init();
    network.startNetworkThread();
    init_logging();
    running = true;
}

void Server::run() {
    spdlog::info("Server main loop started");
    auto &network = myu::NetWork::getInstance();
    auto &matchController = MatchController::Instance();
    matchController.ChangeState(std::make_unique<WaitState>());
    spdlog::info("房间已创建，等待玩家加入...");

    constexpr auto delta_time = std::chrono::duration<float, std::chrono::milliseconds::period>(1000.0 / Config::server::TPS);
    const auto step = std::chrono::duration_cast<std::chrono::nanoseconds>(delta_time);
    auto nextTick = std::chrono::high_resolution_clock::now();

    auto _lastT = std::chrono::high_resolution_clock::now();

    while (running) {
        RecvPacket pkt;
        int pkt_count = 0;
        //处理数据包
        while (network.popPacket(pkt)) {
            if (pkt_count > Config::server::MAX_PER_TICK_PACKET_PROCESS) {
                spdlog::warn("本帧处理数据包过多，可能出现死循环，跳过本tick，并清空数据包队列");
                while (network.popPacket(pkt)){}
                break;
            }
            switch (pkt.type) {
                case NetPacketType::Connect:
                    onClientConnect(pkt.client);
                    break;

                case NetPacketType::Disconnect:
                    onClientDisconnect(pkt.client);
                    break;

                case NetPacketType::Packet:
                    onClientPacket(pkt);
                    break;
            }
            pkt_count++;
        }

        auto _now = std::chrono::high_resolution_clock::now();
        auto _frameTime = _now - _lastT;
        _lastT = _now;
        if (_frameTime > std::chrono::milliseconds(20)) {
            spdlog::warn("服务器主循环卡顿过久：{} ms",
                         std::chrono::duration_cast<std::chrono::milliseconds>(_frameTime).count());
        }

        matchController.Tick(delta_time.count()); // 固定时间步长为16ms

        // playerSync();

        nextTick += step;

        while (true) {
            auto now = std::chrono::high_resolution_clock::now();
            auto remaining = nextTick - now;

            if (remaining > std::chrono::nanoseconds(0)) {
                // yield to other threads
                // still busy waiting though φ(゜▽゜*)♪
                std::this_thread::yield();
            } else {
                break;
            }

            // extremely large lag compensation
            if (remaining < -step * 5) {
                nextTick = now;
                break;
            }
        }

        tick.fetch_add(1);
        // spdlog::info("Tick {}", tick.load());
    }
}

void Server::stop() {
    running = false;
    auto &network = myu::NetWork::getInstance();
    myu::PhysicsEngine::getInstance().destroy();
    network.stopNetworkThread();
}

void Server::onClientConnect(ClientID client) {
    spdlog::info("Client {} connected", client);
}

void Server::onClientDisconnect(ClientID client) {
    spdlog::info("Client {} disconnected", client);
}

void Server::onClientPacket(const RecvPacket &pkt) {
    if (pkt.payload.empty()) {
        spdlog::warn("Empty packet from client {}", pkt.client);
        return;
    }

    flatbuffers::Verifier verifier(
        pkt.payload.data(),
        pkt.payload.size()
    );

    if (!myu::net::VerifyNetMessageBuffer(verifier)) {
        spdlog::warn("Invalid NetMessage from client {}", pkt.client);
        return;
    }

    const myu::net::NetMessage *msg =
            myu::net::GetNetMessage(pkt.payload.data());

    dispatchNetMessage(pkt.client, msg);
}

void Server::dispatchNetMessage(
    ClientID client,
    const myu::net::NetMessage *msg
) {
    const auto *header = msg->header();

    if (!header) {
        spdlog::warn("Packet without header from {}", client);
        return;
    }

    switch (msg->packet_type()) {
        case myu::net::PacketUnion::PacketUnion_MovePacket: {
            HandlePacket::handleMove(client, msg->packet_as_MovePacket());
            break;
        }

        case myu::net::PacketUnion::PacketUnion_FirePacket: {
            HandlePacket::handleFire(client, msg->header(), msg->packet_as_FirePacket());
            break;
        }

        case myu::net::PacketUnion::PacketUnion_PurchaseEvent: {
            HandlePacket::handlePurchase(client, msg->packet_as_PurchaseEvent());
            break;
        }

        case myu::net::PacketUnion::PacketUnion_DefuseBombEvent: {
            HandlePacket::handleDefuse(client, msg->packet_as_DefuseBombEvent());
            break;
        }

        case myu::net::PacketUnion::PacketUnion_PlantBombEvent: {
            HandlePacket::handlePlant(client, msg->packet_as_PlantBombEvent());
            break;
        }

        case myu::net::PacketUnion::PacketUnion_PlayerInfo: {
            HandlePacket::handlePlayerReady(client, msg->packet_as_PlayerInfo());
            break;
        }

        case myu::net::PacketUnion::PacketUnion_NONE: {
            spdlog::error("Packet with none from {}", client);
            break;
        }

        default:
            spdlog::warn("Unknown PacketType {} from {}",
                         int(msg->packet_type()), client);
            break;
    }
}

size_t Server::getTick() const {
    return tick;
}

