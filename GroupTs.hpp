#pragma once

#include "Registry.hpp"
#include <tuple>
#include <vector>

template <typename ComponentList>
class Registry;

template <typename... Ts>
class GroupTs
{
    private:

        const Registry<TypeList<Ts...>>& _registry;
        std::vector<Entity> _entities;

    public:

        using ComponentTuple = std::tuple<Ts&...>;

        GroupTs(const Registry<TypeList<Ts...>>& reg) : _registry(reg) {}
        virtual ~GroupTs(void) = default;

        struct Iterator
        {
            Registry<TypeList<Ts...>>& reg;
            const std::vector<Entity>& entities;
            std::size_t index = 0;

            Iterator(const Registry<TypeList<Ts...>>& r, const std::vector<Entity>& e, std::size_t i) : reg(r), entities(e), index(i) {}
            Iterator& operator++(void) {index++; return *this;}
            bool operator!=(const Iterator& other) const {return index != other.index;}
            auto operator*(void) const 
            {
                Entity e = entities[index];
                return std::tuple_cat(std::make_tuple(e), reg.template get<Ts>(e)...);
            }
        };

        Iterator begin(void) const {return {_registry, _entities, 0};}
        Iterator end(void) const {return {_registry, _entities, _entities.size()};}
        void refresh(void)
        {
            _entities.clear();
            auto& pool = _registry.template storage<Ts...>().template entities();
            for (Entity e : pool)
            {
                if (_registry.template hasAll<TypeList<Ts...>>(e))
                    _entities.push_back(e);
            }
        }        
};