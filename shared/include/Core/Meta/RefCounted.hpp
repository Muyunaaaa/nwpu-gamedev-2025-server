#pragma once

#include "Core/Common.hpp"
#include "Core/Meta/TypeTraits.hpp"

MOE_BEGIN_NAMESPACE

namespace Meta {
    template<typename T>
    struct IsRefCounted {
        template<typename U>
        static auto test(int)
                -> decltype(Meta::DeclareValue<U>().retain(),
                            Meta::DeclareValue<U>().release(),
                            Meta::TrueType{});

        template<typename U>
        static auto test(...) -> Meta::FalseType;

        static constexpr bool value = decltype(test<T>(0))::value;
    };

    template<typename T>
    constexpr bool IsRefCountedV = IsRefCounted<T>::value;
}// namespace Meta

MOE_END_NAMESPACE
