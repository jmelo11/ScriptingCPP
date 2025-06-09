#pragma once

//  Compile time check if a type is part of a pack
//  Use as follows:

//  Pack<T1, T2, T3, ...>::includes<T>() = true if T is part of T1, T2, T3, ... false otherwise
//  includes() is resolved at compile time

template <typename... Vs>
struct Pack;

template <typename V>
struct Pack<V>
{
    template <class T>
    static constexpr bool includes()
    {
        return std::is_same_v<T, V>;
    }
};

template <typename V, typename... Vs>
struct Pack<V, Vs...>
{
    template <class T>
    static constexpr bool includes()
    {
        return Pack<V>::template includes<T>() || Pack<Vs...>::template includes<T>();
    }
};
