#include "server.h"
//
// Created by Administrator on 25-12-25.
//
int main() {
    auto& server = Server::instance();
    server.start();
    server.run();
    server.stop();
}