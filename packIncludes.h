#pragma once

#include <type_traits>

// Compile-time check if a type is part of a pack
// Usage:
// Pack<T1,T2,...>::includes<T>() yields true if T is among T1,T2,...

template<typename... Vs>
struct Pack;

template<typename V>
struct Pack<V>
{
    template<class T>
    static constexpr bool includes()
    {
        return std::is_same<T,V>::value;
    }
};

template<typename V, typename... Vs>
struct Pack<V, Vs...>
{
    template<class T>
    static constexpr bool includes()
    {
        return Pack<V>::template includes<T>() || Pack<Vs...>::template includes<T>();
    }
};

