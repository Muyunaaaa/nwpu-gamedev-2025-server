#pragma once

#include "Core/Common.hpp"
#include "Core/Meta/Feature.hpp"
#include "Core/Meta/TypeTraits.hpp"


#include <mutex>
#include <shared_mutex>

MOE_BEGIN_NAMESPACE

template<typename Derived>
struct MutexSynchronized {
public:
    MutexSynchronized() = default;
    ~MutexSynchronized() = default;

    MutexSynchronized(MutexSynchronized&&) {}
    MutexSynchronized& operator=(MutexSynchronized&&) { return *this; }

    struct LockGuard : Meta::NonCopyable<LockGuard> {
    public:
        LockGuard(Derived& obj, std::mutex& mutex) noexcept
            : m_obj(obj), m_lock(mutex) {}

        LockGuard(LockGuard&&) = default;
        LockGuard& operator=(LockGuard&&) = default;

        Derived& operator*() {
            return m_obj;
        }

        Derived* operator->() {
            return &m_obj;
        }

    private:
        Derived& m_obj;
        std::unique_lock<std::mutex> m_lock;
    };

    struct ConstLockGuard : Meta::NonCopyable<ConstLockGuard> {
    public:
        ConstLockGuard(const Derived& obj, std::mutex& mutex) noexcept
            : m_obj(obj), m_lock(mutex) {}

        const Derived& operator*() const {
            return m_obj;
        }

        const Derived* operator->() const {
            return &m_obj;
        }

    private:
        const Derived& m_obj;
        std::unique_lock<std::mutex> m_lock;
    };

    template<typename F, typename... Args>
    auto withLock(F&& f, Args&&... args)
            -> decltype(f(std::forward<Args>(args)...)) {
        std::lock_guard<std::mutex> lk(m_mutex);
        return f(std::forward<Args>(args)...);
    }

    template<typename F, typename... Args>
    auto withLock(F&& f, Args&&... args) const
            -> decltype(f(std::forward<Args>(args)...)) {
        std::lock_guard<std::mutex> lk(m_mutex);
        return f(std::forward<Args>(args)...);
    }

    LockGuard lock() {
        return LockGuard(static_cast<Derived&>(*this), m_mutex);
    }

    ConstLockGuard lock() const {
        return ConstLockGuard(static_cast<const Derived&>(*this), m_mutex);
    }

private:
    mutable std::mutex m_mutex;
};

template<typename Derived>
struct RwSynchronized {
public:
    RwSynchronized() = default;
    ~RwSynchronized() = default;

    RwSynchronized(RwSynchronized&&) {}
    RwSynchronized& operator=(RwSynchronized&&) { return *this; }

    struct ReadGuard : Meta::NonCopyable<ReadGuard> {
    public:
        ReadGuard(const Derived& obj, std::shared_mutex& mutex) noexcept
            : m_obj(obj), m_lock(mutex) {}

        const Derived& operator*() const {
            return m_obj;
        }

        const Derived* operator->() const {
            return &m_obj;
        }

    private:
        const Derived& m_obj;
        std::shared_lock<std::shared_mutex> m_lock;
    };

    struct WriteGuard : Meta::NonCopyable<WriteGuard> {
    public:
        WriteGuard(Derived& obj, std::shared_mutex& mutex) noexcept
            : m_obj(obj), m_lock(mutex) {}

        Derived& operator*() {
            return m_obj;
        }

        Derived* operator->() {
            return &m_obj;
        }

    private:
        Derived& m_obj;
        std::unique_lock<std::shared_mutex> m_lock;
    };

    template<typename F, typename... Args>
    auto withReadLock(F&& f, Args&&... args) const
            -> decltype(f(std::forward<Args>(args)...)) {
        std::shared_lock<std::shared_mutex> lk(m_mutex);
        return f(std::forward<Args>(args)...);
    }

    template<typename F, typename... Args>
    auto withWriteLock(F&& f, Args&&... args)
            -> decltype(f(std::forward<Args>(args)...)) {
        std::unique_lock<std::shared_mutex> lk(m_mutex);
        return f(std::forward<Args>(args)...);
    }

    ReadGuard read() {
        return ReadGuard(static_cast<Derived&>(*this), m_mutex);
    }

    ReadGuard read() const {
        return ReadGuard(static_cast<const Derived&>(*this), m_mutex);
    }

    WriteGuard write() {
        return WriteGuard(static_cast<Derived&>(*this), m_mutex);
    }

private:
    mutable std::shared_mutex m_mutex;
};

MOE_END_NAMESPACE