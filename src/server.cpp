#include <iostream>
#include <thread>

#include "network/NetWorkService.h"
#include "spdlog/spdlog.h"

int main() {
    try {
        myu::NetWork network;

        network.init();
        network.startNetworkThread();

        spdlog::info("服务器启动，等待客户端连接...");

        bool running = true;

        while (running) {
            RecvPacket pkt;

            // 一帧内处理所有收到的网络事件
            while (network.popPacket(pkt)) {
                switch (pkt.type) {
                    case NetPacketType::Packet: {
                        // 这里只是测试接收
                        std::cout
                            << "[RECV] client=" << pkt.client
                            << " channel=" << static_cast<int>(pkt.channel)
                            << " size=" << pkt.payload.size()
                            << " recv_time_ns=" << pkt.recv_time_ns
                            << std::endl;

                        // 如果你发的是 uint32_t
                        if (pkt.payload.size() >= sizeof(uint32_t)) {
                            uint32_t value;
                            std::memcpy(&value, pkt.payload.data(), sizeof(uint32_t));
                            value = ENET_NET_TO_HOST_32(value);
                            std::cout << "  value=" << value << std::endl;
                        }
                        break;
                    }

                    case NetPacketType::Disconnect:
                        std::cout
                            << "[DISCONNECT] client=" << pkt.client
                            << std::endl;
                    break;
                }
            }

            // === Tick 控制（非常重要）===
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60Hz
        }

        network.stopNetworkThread();
    }
    catch (const std::exception& e) {
        spdlog::error("服务器异常退出: {}", e.what());
    }

    return 0;
}
