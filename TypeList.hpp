#pragma once

#include <utility>

// === Méta basé sur TypeList voir autre répo  ===

template <typename T>
struct TypeTag { using type = T; };

template <typename... Ts>
struct TypeList { using type = TypeList<Ts...>; };

template <typename List> 
struct Front;

template <typename T, typename... Ts>
struct Front<TypeList<T, Ts...>> 
{ 
    using type = T; 
};

template <typename TypeList, typename Func>
struct StaticForEachImpl;

template <typename Func>
struct StaticForEachImpl<TypeList<>, Func>
{
    static void apply(Func&&) {}
};

template <typename T, typename... Ts, typename Func>
struct StaticForEachImpl<TypeList<T, Ts...>, Func>
{
    static void apply(Func&& fnc)
    {
        fnc(TypeTag<T>{});
        StaticForEachImpl<TypeList<Ts...>, Func>::apply(std::forward<Func>(fnc));
    }
};

template <typename List, typename Func>
void StaticForEach(Func&& fnc) 
{
    StaticForEachImpl<List, Func>::apply(std::forward<Func>(fnc));
}