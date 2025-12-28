#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <cstdint>
#include "util/RingQueue.h" // 假设你的头文件名为 RingQueue.h

// 模拟你的 PlayerUpdate 结构
struct PlayerUpdate {
    uint32_t tick;
    float x;

    bool operator==(const PlayerUpdate& other) const {
        return tick == other.tick && x == other.x;
    }
};

void test_basic_ops() {
    std::cout << "Running Basic Operations Test..." << std::endl;
    RingQueue<int, 3> q;

    assert(q.empty());
    assert(q.size() == 0);

    q.push(10);
    q.push(20);
    assert(q.size() == 2);
    assert(q.front() == 10);
    assert(q.back() == 20);

    int out;
    bool success = q.pop(out);
    assert(success && out == 10);
    assert(q.size() == 1);
    assert(q.front() == 20);

    std::cout << "-> Basic Operations Passed!" << std::endl;
}

void test_overflow_and_indexing() {
    std::cout << "Running Overflow and Indexing Test..." << std::endl;
    // 容量为 3
    RingQueue<int, 3> q;

    q.push(1); // index 0 (logical)
    q.push(2); // index 1
    q.push(3); // index 2
    assert(q.full());

    // 此时缓冲区：[1, 2, 3], tail=0, head=0 (wrap)
    assert(q[0] == 1);
    assert(q[2] == 3);

    // 关键点：触发覆盖
    q.push(4);
    // 预期：1 被挤掉，tail 移向 2。逻辑索引 0 现在应该是 2
    // 此时缓冲区物理可能：[4, 2, 3], tail=1, head=1
    assert(q.size() == 3);
    assert(q.front() == 2); // 最旧的是 2
    assert(q.back() == 4);  // 最新的是 4

    assert(q[0] == 2); // 逻辑 index 0 映射到物理 index 1
    assert(q[1] == 3); // 逻辑 index 1 映射到物理 index 2
    assert(q[2] == 4); // 逻辑 index 2 映射到物理 index 0

    std::cout << "-> Overflow and Indexing Passed!" << std::endl;
}

void test_player_history_lookup() {
    std::cout << "Running Player History Lookup Test..." << std::endl;
    // 模拟 300 帧容量，但我们用小的测试
    RingQueue<PlayerUpdate, 5> history;

    // 模拟服务器步进 Tick 100 到 104
    for (uint32_t t = 100; t < 105; ++t) {
        history.push({t, (float)t * 1.1f});
    }

    // 模拟回溯查找函数 (手动实现逻辑)
    auto getHistory = [&](uint32_t target) -> const PlayerUpdate* {
        for (int i = (int)history.size() - 1; i >= 0; --i) {
            if (history[i].tick == target) return &history[i];
            if (history[i].tick < target) break;
        }
        return nullptr;
    };

    // 1. 查找存在的
    const auto* res1 = getHistory(102);
    assert(res1 != nullptr && res1->tick == 102);

    // 2. 覆盖后查找
    history.push({105, 105.5f}); // 挤掉 Tick 100
    const auto* res2 = getHistory(100);
    assert(res2 == nullptr); // 100 应该已经不在了

    const auto* res3 = getHistory(105);
    assert(res3 != nullptr && res3->tick == 105);

    std::cout << "-> Player History Lookup Passed!" << std::endl;
}

void test_move_semantics() {
    std::cout << "Running Move Semantics Test..." << std::endl;
    RingQueue<std::string, 2> q;

    std::string s = "Long string to avoid SSO";
    q.push(std::move(s));
    assert(s.empty()); // 原字符串应被移动
    assert(q.front() == "Long string to avoid SSO");

    std::cout << "-> Move Semantics Passed!" << std::endl;
}

// int main() {
//     try {
//         test_basic_ops();
//         test_overflow_and_indexing();
//         test_player_history_lookup();
//         test_move_semantics();
//         std::cout << "\nALL TESTS PASSED SUCCESSFULLY!" << std::endl;
//     } catch (const std::exception& e) {
//         std::cerr << "Test failed with exception: " << e.what() << std::endl;
//         return 1;
//     }
//     return 0;
// }
