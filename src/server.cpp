#include "Server.h"
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

void init_logging() {
    // 1. 初始化线程池：队列大小 8192，后端 1 个工作线程
    // 队列大小必须是 2 的幂
    spdlog::init_thread_pool(8192, 1);

    // 2. 创建异步 Logger（控制台颜色输出）
    auto logger = spdlog::stdout_color_mt<spdlog::async_factory>("async_logger");

    // 3. 设置为全局默认 Logger（这样你代码里的 spdlog::info 就会走异步了）
    spdlog::set_default_logger(logger);

    // 4. (可选) 设置刷新策略：每 3 秒刷新一次磁盘，或遇到 Warn 级别立即刷新
    spdlog::flush_every(std::chrono::seconds(3));
}


void Server::start() {
    auto& network = myu::NetWork::getInstance();
    network.init();
    network.startNetworkThread();
    init_logging();
    running = true;
}

void Server::run() {
    spdlog::info("Server main loop started");
    auto& network = myu::NetWork::getInstance();
    MatchController matchController;
    //FIXME:MEMLEAK
    matchController.ChangeState(std::make_unique<WaitState>());
    spdlog::info("房间已创建，等待玩家加入...");

    using clock = std::chrono::steady_clock;
    constexpr float TPS = 60.0f; // 60 ticks per second
    constexpr auto delta_time = std::chrono::duration<float, std::chrono::milliseconds::period>(1000.0 / TPS);
    const auto step = std::chrono::duration_cast<std::chrono::nanoseconds>(delta_time);
    auto nextTick = std::chrono::high_resolution_clock::now();

    auto _lastT = std::chrono::high_resolution_clock::now();
    while (running) {
        RecvPacket pkt;
        //TODO:这里要防止一直循环，需要根据实际情况限制获取数据包数量
        //处理数据包
        while (network.popPacket(pkt)) {
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
        }

        auto _now = std::chrono::high_resolution_clock::now();
        auto _frameTime = _now - _lastT;
        _lastT = _now;
        if (_frameTime > std::chrono::milliseconds(20)) {
            spdlog::warn("服务器主循环卡顿过久：{} ms",
                         std::chrono::duration_cast<std::chrono::milliseconds>(_frameTime).count());
        }

        //FIXME:更新状态机状态
        matchController.Tick(delta_time.count()); // 固定时间步长为16ms

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
            //todo: 时间漂移以后deltatime会改变，不再是16.7ms
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
    auto& network = myu::NetWork::getInstance();
    network.stopNetworkThread();
}

void Server::onClientConnect(ClientID client) {
    //TODO:需要将连接数据包解包
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
    const auto *payload = msg->packet_as_FirePacket();

    if (!header) {
        spdlog::warn("Packet without header from {}", client);
        return;
    }

    switch (msg->packet_type()) {
        case myu::net::PacketUnion::PacketUnion_MovePacket: {
            //handleMove(client, msg->packet_as_MovePacket());
            break;
        }

        case myu::net::PacketUnion::PacketUnion_FirePacket: {
            //handleFire(client, msg->packet_as_FirePacket());
            break;
        }

        case myu::net::PacketUnion::PacketUnion_PurchaseEvent: {
            //handlePlayerState(client, msg->packet_as_PlayerStatePacket());
            break;
        }

        case myu::net::PacketUnion::PacketUnion_DefuseBombEvent: {

        }

        case myu::net::PacketUnion::PacketUnion_PlantBombEvent: {

        }

        case myu::net::PacketUnion::PacketUnion_PlayerPositionPacket: {

        }

        case myu::net::PacketUnion::PacketUnion_NONE: {
            spdlog::error("Packet with none from {}", client);
        }

        case myu::net::PacketUnion::PacketUnion_PlayerInfo: {
            const auto *info = msg->packet_as_PlayerInfo();

            uint32_t player_id = client; // 假设 client 是唯一的 ClientID
            std::string player_name = info->name()->c_str();
            bool is_ready = info->ready();

            auto &room = RoomContext::getInstance();

            auto it = room.players.find(player_id);
            if (it == room.players.end()) {
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



