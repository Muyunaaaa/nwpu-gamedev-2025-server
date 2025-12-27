#pragma once

#include "Core/Common.hpp"
#include "Core/Meta/TypeTraits.hpp"

MOE_BEGIN_NAMESPACE

namespace Detail {
    struct Infallible {};

    template<typename T>
    struct OkWrapper {
        T value;
    };

    template<typename E>
    struct ErrWrapper {
        E error;
    };

    template<typename E>
    constexpr bool IsInfallible = Meta::IsSameV<E, Infallible>;
}// namespace Detail

using Infallible = Detail::Infallible;

template<
        typename T,
        typename E = Detail::Infallible,
        typename = Meta::EnableIf<!Meta::IsVoidV<T>>>
struct Expected {
public:
    using value_type = T;
    using error_type = E;

    Expected(const Detail::OkWrapper<T>& ok)
        : m_isOk(true) {
        if constexpr (Detail::IsInfallible<E>) {
            m_valueOrError = ok.value;
        } else {
            m_valueOrError = Variant<T, E>(std::in_place_index<0>, ok.value);
        }
    }

    Expected(const Detail::ErrWrapper<E>& err)
        : m_isOk(false) {
        static_assert(!Detail::IsInfallible<E>, "Cannot construct Expected with error when E is Detail::Infallible");
        m_valueOrError = Variant<T, E>(std::in_place_index<1>, err.error);
    }

    Expected(Detail::OkWrapper<T>&& ok)
        : m_isOk(true) {
        if constexpr (Detail::IsInfallible<E>) {
            m_valueOrError = std::move(ok.value);
        } else {
            m_valueOrError = Variant<T, E>(std::in_place_index<0>, std::move(ok.value));
        }
    }

    Expected(Detail::ErrWrapper<E>&& err)
        : m_isOk(false) {
        if constexpr (!Detail::IsInfallible<E>) {
            m_valueOrError = Variant<T, E>(std::in_place_index<1>, std::move(err.error));
        } else {
            MOE_ASSERT(false, "Cannot construct Expected with error when E is Detail::Infallible");
        }
    }

    bool isOk() const {
        return m_isOk;
    }

    bool isErr() const {
        return !m_isOk;
    }

    explicit operator bool() const { return m_isOk; }

    T unwrap() const {
        MOE_ASSERT(m_isOk, "Called unwrap on an Err value");
        if constexpr (Detail::IsInfallible<E>) {
            return m_valueOrError;
        } else {
            return std::get<0>(m_valueOrError);
        }
    }

    E unwrapErr() const {
        MOE_ASSERT(!m_isOk, "Called unwrapErr on an Ok value");
        if constexpr (!Detail::IsInfallible<E>) {
            return std::get<1>(m_valueOrError);
        }
    }

    template<typename F, typename U = Meta::InvokeResultT<F, T>,
             typename = Meta::EnableIfT<!Meta::IsVoidV<U>>>
    Expected<U, E> andThen(F func) const {
        if (!Detail::IsInfallible<E>) {
            if (m_isOk) {
                return Detail::OkWrapper<U>{func(unwrap())};
            } else {
                return Detail::ErrWrapper<E>{unwrapErr()};
            }
        }
        return Detail::OkWrapper<U>{func(unwrap())};
    }

    template<typename F, typename X = Meta::InvokeResultT<F, E>,
             typename = Meta::EnableIfT<!Meta::IsVoidV<X>>>
    Expected<T, X> orElse(F func) const {
        if constexpr (!Detail::IsInfallible<E>) {
            if (m_isOk) {
                return Detail::OkWrapper<T>{unwrap()};
            } else {
                return Detail::ErrWrapper<X>{func(unwrapErr())};
            }
        }
        return Detail::OkWrapper<T>{unwrap()};
    }

    T unwrapOrDefault(T defaultValue) const {
        if (m_isOk) {
            return unwrap();
        } else {
            return defaultValue;
        }
    }

    Optional<T> transpose() const {
        if (m_isOk) {
            return unwrap();
        } else {
            return std::nullopt;
        }
    }

    static auto Ok(T value) {
        return Expected<T, E>{Detail::OkWrapper<T>{std::move(value)}};
    }

    static auto Err(E error) {
        static_assert(!Detail::IsInfallible<E>, "Cannot create Err Expected when E is Detail::Infallible");
        return Expected<T, E>{Detail::ErrWrapper<E>{std::move(error)}};
    }

private:
    bool m_isOk;
    Meta::ConditionalT<
            Detail::IsInfallible<E>,
            T, Variant<T, E>>
            m_valueOrError;
};

template<typename T, typename E = Detail::Infallible>
Expected<T, E> Ok(T value) {
    return Detail::OkWrapper<T>{std::move(value)};
}

template<typename E>
Detail::ErrWrapper<E> Err(E error) {
    return Detail::ErrWrapper<E>{std::move(error)};
}

template<
        typename T, typename E,
        template<typename...> class ContainerT,
        typename = Meta::EnableIfT<Meta::IsIterableV<ContainerT<Expected<T, E>>>>>
Expected<Vector<T>, E> collectExpected(ContainerT<Expected<T, E>>&& expecteds) {
    using ReturnT = Expected<Vector<T>, E>;

    Vector<T> results;
    results.reserve(expecteds.size());

    for (const auto& expected: expecteds) {
        if constexpr (!Detail::IsInfallible<E>) {
            if (expected.isErr()) {
                return ReturnT::Err(expected.unwrapErr());
            }
        }
        results.push_back(expected.unwrap());
    }

    return ReturnT::Ok(std::move(results));
}

template<typename T, typename E>
auto tryFromOptional(const Optional<T>& opt, E error) {
    using ReturnT = Expected<T, E>;
    if (opt.has_value()) {
        return ReturnT::Ok(opt.value());
    } else {
        return ReturnT::Err(std::move(error));
    }
}

MOE_END_NAMESPACE