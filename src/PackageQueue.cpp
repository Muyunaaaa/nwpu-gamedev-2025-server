//
// Created by 冯于洋 on 25-12-20.
//
#include "network/PackageQueue.h"
namespace myu {
    namespace net {
        PackageQueue::PackageQueue() {}

        PackageQueue::~PackageQueue() {}

        void PackageQueue::updateAndClear() {
            while (!packages.empty()) {
                auto pkg = packages.front();
                // 处理数据包 pkg
                // ...

                // 处理完毕后弹出数据包
                packages.pop();
            }
        }
    }
}