#pragma once
#include <array>
#include <cstddef>
#include <utility>

/*
 * 固定容量环形队列
 * - 超出容量时，自动覆盖最旧元素
 * - 非线程安全（适合单线程 / 已加锁 / 单生产者单消费者）
 */
template <typename T, size_t Capacity>
class RingQueue {
    static_assert(Capacity > 0, "RingQueue Capacity must be > 0");

public:
    RingQueue() = default;

    // 拷贝插入
    void push(const T& value) {
        buffer_[head_] = value;
        advance();
    }

    // 移动插入
    void push(T&& value) {
        buffer_[head_] = std::move(value);
        advance();
    }

    // 弹出最早元素
    bool pop(T& out) {
        if (size_ == 0) {
            return false;
        }

        out = std::move(buffer_[tail_]);
        tail_ = (tail_ + 1) % Capacity;
        --size_;
        return true;
    }

    // 查看最早元素（不弹出）
    T& front() {
        return buffer_[tail_];
    }

    const T& front() const {
        return buffer_[tail_];
    }

    // 查看最新元素
    T& back() {
        size_t idx = (head_ + Capacity - 1) % Capacity;
        return buffer_[idx];
    }

    const T& back() const {
        size_t idx = (head_ + Capacity - 1) % Capacity;
        return buffer_[idx];
    }

    bool empty() const {
        return size_ == 0;
    }

    bool full() const {
        return size_ == Capacity;
    }

    size_t size() const {
        return size_;
    }

    constexpr size_t capacity() const {
        return Capacity;
    }

    void clear() {
        head_ = 0;
        tail_ = 0;
        size_ = 0;
    }

    T& operator[](size_t logicalIndex) {
        size_t physicalIndex = (tail_ + logicalIndex) % Capacity;
        return buffer_[physicalIndex];
    }

    const T& operator[](size_t logicalIndex) const {
        size_t physicalIndex = (tail_ + logicalIndex) % Capacity;
        return buffer_[physicalIndex];
    }

private:
    void advance() {
        head_ = (head_ + 1) % Capacity;

        if (size_ < Capacity) {
            ++size_;
        } else {
            // 满了，挤掉最早的
            tail_ = (tail_ + 1) % Capacity;
        }
    }

private:
    std::array<T, Capacity> buffer_{};
    size_t head_ = 0; // 写入位置
    size_t tail_ = 0; // 读取位置
    size_t size_ = 0;
};
