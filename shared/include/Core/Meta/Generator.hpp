#pragma once

#include "Core/Common.hpp"
#include "Core/Meta/TypeTraits.hpp"

MOE_BEGIN_NAMESPACE

namespace Meta {
    template<typename T>
    struct HasGeneratorMethods {
    private:
        template<typename U>
        static auto test(int)
                -> decltype(Meta::DeclareValue<U>().generate(),   // -> value_type
                            Meta::DeclareValue<U>().hashCode(),   // -> uint64_t
                            Meta::DeclareValue<U>().paramString(),// -> String
                            Meta::TrueType{});

        template<typename U>
        static auto test(...) -> Meta::FalseType;

    public:
        static constexpr bool value = decltype(test<T>(0))::value;
    };

    template<typename T>
    constexpr bool HasGenerateMethodV = HasGeneratorMethods<T>::value;

    template<typename T>
    constexpr bool IsGeneratorV =
            HasGenerateMethodV<T> && Meta::HasValueTypeV<T>;

    template<typename T>
    struct IsBinarySerializable {
    private:
        template<typename U>
        static auto test(int)
                -> decltype(
                        // static method -> Optional<typename U::value_type>
                        U::deserialize(Meta::DeclareValue<Span<const uint8_t>>()),
                        U::serialize(),// -> Vector<uint8_t>
                        Meta::TrueType{});

        template<typename U>
        static auto test(...) -> Meta::FalseType;

    public:
        static constexpr bool value = decltype(test<T>(0))::value;
    };

    template<typename T>
    constexpr bool IsBinarySerializableV = IsBinarySerializable<T>::value;

}// namespace Meta

MOE_END_NAMESPACE