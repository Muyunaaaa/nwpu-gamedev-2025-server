#pragma once

#include <atomic>

template<typename T>
struct Dirty {
public:
    T* get() {
        m_dirty.store(true, std::memory_order_acq_rel);
        return &value;
    }

    const T* get() const {
        return &value;
    }

    T* operator->() {
        m_dirty.store(true, std::memory_order_acq_rel);
        return &value;
    }

    const T* operator->() const {
        return &value;
    }

    T& operator*() {
        m_dirty.store(true, std::memory_order_acq_rel);
        return &value;
    }

    const T& operator*() const {
        return &value;
    }

    bool isDirty() const {
        return m_dirty.load(std::memory_order_relaxed);
    }

    void clear() { m_dirty.store(false, std::memory_order_relaxed); }

    void set(const T& v) {
        value = v;
        m_dirty.store(true, std::memory_order_acq_rel);
    }

    explicit operator bool() const { return isDirty(); }

private:
    std::atomic_bool m_dirty{false};
    T value;
};