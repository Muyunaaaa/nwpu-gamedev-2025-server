//
// Created by 冯于洋 on 25-12-20.
//

#pragma once
#include <queue>

namespace myu {
    namespace net {
        class PackageQueue {
        private:
            std::queue<myu::net::InputDataPackage> packages;

        public:
            PackageQueue();

            ~PackageQueue();

            void updateAndClear();
        };
    }
}
