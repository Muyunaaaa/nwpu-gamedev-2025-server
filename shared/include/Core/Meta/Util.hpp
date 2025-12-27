#pragma once

#include "Core/Common.hpp"
#include "Core/Meta/TypeTraits.hpp"

MOE_BEGIN_NAMESPACE


namespace Detail {
    template<typename F, typename Tuple, size_t... I>
    void applyToTupleImpl(F&& f, Tuple&& t, std::index_sequence<I...>) {
        (f(std::get<I>(std::forward<Tuple>(t)), Meta::IntegralConstant<size_t, I>{}), ...);
    }
}// namespace Detail

template<typename F, typename... Args>
void applyToTuple(F&& f, std::tuple<Args...>&& t) {
    constexpr size_t tupleSize = sizeof...(Args);
    Detail::applyToTupleImpl(
            std::forward<F>(f),
            std::forward<std::tuple<Args...>>(t),
            std::make_index_sequence<tupleSize>{});
}

template<
        typename FirstElem,
        typename... RestElems,
        typename = Meta::EnableIfT<Meta::ConjunctionV<
                Meta::IsSameV<FirstElem, RestElems>...>>>
Vector<FirstElem> flatTuple(std::tuple<FirstElem, RestElems...>&& tup) {
    Vector<FirstElem> result;
    result.reserve(sizeof...(RestElems) + 1);

    auto addToResult = [&result](auto&& elem, auto /*idx*/) {
        result.push_back(std::forward<decltype(elem)>(elem));
    };

    applyToTuple(addToResult, std::move(tup));
    return result;
}

MOE_END_NAMESPACE