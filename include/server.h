#pragma once

#include "network/NetWorkService.h"
#include <atomic>

#include "protocol/receiveGamingPacket_generated.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

class Server {
public:
    static Server& instance() {
        static Server instance;
        return instance;
    }

    void start();
    void run();
    void stop();

    void onClientConnect(ClientID client);
    void onClientDisconnect(ClientID client);
    void onClientPacket(const RecvPacket &pkt);
    void dispatchNetMessage(ClientID client, const myu::net::NetMessage *msg);

    size_t getTick() const;
    std::shared_ptr<spdlog::logger> move_logger = spdlog::basic_logger_mt("move_records", "logs/move_records.log");
    std::shared_ptr<spdlog::logger> fire_logger = spdlog::basic_logger_mt("fire_records", "logs/fire_records.log");
    std::shared_ptr<spdlog::logger> console_logger = spdlog::stdout_color_mt("console");
private:
    Server() = default;
    ~Server() = default;
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    Server(Server&&) = delete;
    Server& operator=(Server&&) = delete;


    std::atomic<bool> running{false};
    std::atomic<size_t> tick{0};
};
