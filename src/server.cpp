#include <cstdint>
#include <iostream>
#include <ostream>
#include <cstring>

#include "enet/enet.h"
#include "network/common.h"
//
// Created by 冯于洋 on 25-12-20.
//
int main() {
    //initialize
    if (enet_initialize() != 0) {
        fprintf(stderr, "无法初始化 ENet！\n");
        return EXIT_FAILURE;
    }

    //临时
    bool running = true;
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 1234;
    ENetHost *host = enet_host_create(&address, 32, 2, 0, 0);

    while (running) {
        // std::cout<<"第一次运行"<<std::endl;
        PacketQueue queue = PollNetwork(host);
        NetPacket net_packet;
        while (!queue.getPackets().empty()) {
            net_packet = queue.getPackets().front();
            if (net_packet.data.size() < sizeof(uint32_t)) {
                std::cout << "收到一个无效的短包" << std::endl;
            }

            uint32_t received_value;

            // 2. 将字节拷贝到变量中
            // 假设客户端发送的是原始内存字节
            std::memcpy(&received_value, net_packet.data.data(), sizeof(uint32_t));

            // 3. 字节序转换 (非常重要)
            // 如果客户端按照网络标准(大端)发送，使用 ENET_NET_TO_HOST_32
            // 如果客户端和你都在同一台电脑测试且没写转换逻辑，可以直接打印，但在正式环境必须转换
            uint32_t final_value = ENET_NET_TO_HOST_32(received_value);

            // 4. 打印结果
            std::cout << "[网络测试] 收到来自 Peer: " << net_packet.peer
                    << " 的数据: " << std::string(net_packet.data.begin(), net_packet.data.end())
                    << " (服务器接收时间: " << net_packet.server_time << ")" << std::endl;

            queue.getPackets().pop();
        }
    }
}
