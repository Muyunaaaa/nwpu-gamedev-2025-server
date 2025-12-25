//
// Created by Administrator on 25-12-25.
//
#include <iostream>
#include <thread>
#include <cstring>

#include "network/NetWorkService.h"
#include "network/NetChannel.h"
#include "spdlog/spdlog.h"
#include "test/network.h"

void myu::test::testNetwork(){
    try {
        myu::NetWork network;

        network.init();
        network.startNetworkThread();

        spdlog::info("服务器启动，等待客户端连接...");

        bool running = true;
        int tick_count = 0;

        while (running) {
            RecvPacket recv_pkt;
            // 一帧内处理所有收到的网络事件
            while (network.popPacket(recv_pkt)) {
                switch (recv_pkt.type) {
                    case NetPacketType::Packet: {
                        std::cout
                                << "[RECV] client=" << recv_pkt.client
                                << " channel=" << static_cast<int>(recv_pkt.channel)
                                << " size=" << recv_pkt.payload.size()
                                << " recv_time_ns=" << recv_pkt.recv_time_ns
                                << std::endl;

                        if (recv_pkt.payload.size() >= sizeof(uint32_t)) {
                            uint32_t value;
                            std::memcpy(&value, recv_pkt.payload.data(), sizeof(uint32_t));
                            value = ENET_NET_TO_HOST_32(value);
                            std::cout << "  value=" << value << std::endl;
                        }

                        SendPacket pkt{
                            0, // 占位，会被 broadcast 覆盖
                            NetChannel::CH_RELIABLE,
                            std::span<const uint8_t>(
                                recv_pkt.payload.data(),
                                recv_pkt.payload.size()
                            ),
                            true
                        };
                        network.broadcast(pkt);
                        break;
                    }

                    case NetPacketType::Disconnect: {
                        std::cout
                                << "[DISCONNECT] client=" << recv_pkt.client
                                << std::endl;
                        break;
                    }
                }
            }
            spdlog::info("tick : {}", tick_count);
            // Tick（~60Hz）
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
            tick_count++;
        }

        network.stopNetworkThread();
    } catch (const std::exception &e) {
        spdlog::error("服务器异常退出: {}", e.what());
    }
}