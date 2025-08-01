#pragma once

#include "TypeList.hpp"
#include "Storage.hpp"
#include "GroupTs.hpp"
#include <algorithm>
#include <iostream>
#include <tuple>

// === Registry ===

template <typename ComponentList>
class Registry;

template <typename... Cs>
class Registry<TypeList<Cs...>> 
{
    private:

        EntityManager _manager;
        std::tuple<ComponentStorage<Cs>...> _storages;

        template <typename ComponentList, typename Func>
        struct ApplyWithImpl;

        template <typename... Ts, typename Func>
        struct ApplyWithImpl<TypeList<Ts...>, Func>
        {
            template <typename RegistryT>
            static void apply(RegistryT& reg, Entity e, Func&& fnc)
            {
                fnc(e, reg.template get<Ts>(e)...);
            }
        };

        template <typename ComponentList, typename Func>
        void applyWithImpl(Entity e, Func&& fnc)
        {
            ApplyWithImpl<ComponentList, Func>::apply(*this, e, std::forward<Func>(fnc));
        }

        template <typename ComponentList, typename Func>
        void applyWith(Entity e, Func&& fnc)
        {
            applyWithImpl<ComponentList>(e, std::forward<Func>(fnc));
        }

    public:

        using ComponentTypes = TypeList<Cs...>;

        virtual ~Registry(void) = default;

        Entity create(void) { return _manager.create(); }

        void destroy(Entity e) 
        {
            StaticForEach<ComponentTypes>([&](auto tag) {
                using T = typename decltype(tag)::type;
                storage<T>().remove(e);
            });
            _manager.destroy(e);
        }

        std::vector<Entity> getAliveEntities(void)
        {
            return _manager.getAliveEntities();
        }

        template <typename T>
        void add(Entity e, T&& value) 
        {
            storage<T>().emplace(e, std::forward<T>(value));
        }

        template <typename T>
        bool has(Entity e) const 
        {
            return storage<T>().has(e);
        }

        template <typename T>
        T& get(Entity e) 
        {
            return storage<T>().get(e);
        }

        template <typename T>
        const T& get(Entity e) const
        {
            return storage<T>().get(e);
        }

        template <typename T>
        void remove(Entity e)
        {
            storage<T>().remove(e);
        }

        template <typename T>
        T* getIf(Entity e)
        {
            if (!storage<T>().has(e))
                return nullptr;
            return &storage<T>().get(e);
        }

        template <typename T>
        ComponentStorage<T>& storage(void) 
        {
            return std::get<ComponentStorage<T>>(_storages);
        }

        template <typename T>
        const ComponentStorage<T>& storage(void) const 
        {
            return std::get<ComponentStorage<T>>(_storages);
        }


        template <typename T>
        T* getIf(Entity e) const
        {
            if (!storage<T>().has(e))
                return nullptr;
            return &storage<T>().get(e);
        }

        template <typename ComponentList>
        bool hasAll(Entity e) const
        {
            bool result = true;
            StaticForEach<ComponentList>([&](auto tag) 
            {
                using T = typename decltype(tag)::type;
                result &= this->template has<T>(e);
            });
            return result;
        }

        void preAllocate(std::size_t count)
        {
            _manager.preAllocate(count);
        }

        void reset(void)
        {
            _manager.reset();
        }

        template <typename ComponentList, typename Func>
        void forEachEntityWith(Func&& fnc)
        {
            using First = typename Front<ComponentList>::type;
            for (Entity e : storage<First>().entities())
            {
                if (hasAll<ComponentList>(e))
                    applyWith<ComponentList>(e, std::forward<Func>(fnc));
            }
        }

        void debugEntity(Entity e) const
        {
            std::cout << "[Entity] ID = " << e.id << ", version = " << e.version << " : ";
            StaticForEach<ComponentTypes>([&](auto tag) {
                using T = typename decltype(tag)::type;
                if (this->template has<T>(e))
                    std::cout << " - Has :  " << typeid(T).name() << " ";
            });
        }

        template <typename... Ts>
        GroupTs<Ts...> group(void) const
        {
            GroupTs<Ts...> g(*this);
            g.refresh();
            return g;
        }

};