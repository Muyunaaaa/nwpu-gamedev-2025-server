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

#include "core/HandlePacket.h"
#include "protocol/Main_generated.h"
#include "util/getTime.h"

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
            HandlePacket::handleFire(client, msg->packet_as_FirePacket());
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

void Server::playerSync() {
    flatbuffers::FlatBufferBuilder fbb;

    moe::net::Vec3 pos{0.0f,0.0f,0.0f};
    moe::net::Vec3 vel{0.0f,0.0f,0.0f};
    moe::net::Vec3 head{0.0f,0.0f,0.0f};
    auto my_update = moe::net::CreatePlayerUpdate(
        fbb,
        -1,
        &pos,
        &vel,
        &head,
        moe::net::PlayerMotionState::PlayerMotionState_NORMAL,
        -1
        );
    auto player_updates = std::vector<flatbuffers::Offset<moe::net::PlayerUpdate>>();
    player_updates.push_back(my_update);
    auto event = moe::net::CreateAllPlayerUpdate(
        fbb,
            fbb.CreateVector(player_updates),
            my_update
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
    SendPacket player_update_packet = SendPacket(-1, CH_RELIABLE, fbb.GetBufferSpan(), true);
    myu::NetWork::getInstance().broadcast(player_update_packet);
}


