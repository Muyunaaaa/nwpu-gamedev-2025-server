#include "server.h"
//
// Created by Administrator on 25-12-25.
//
#include "Core/FileReader.hpp"
int main() {
    moe::FileReader::initReader(new moe::DefaultFileReader());
    auto& server = Server::instance();
    server.start();
    server.run();
    server.stop();
    moe::FileReader::destroyReader();
}