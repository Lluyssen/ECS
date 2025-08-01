#pragma once

#include "TypeList.hpp"
#include "Registry.hpp"
#include <typeindex>
#include <typeinfo>
#include <string>
#include <vector>
#include <sstream>
#include <tuple>

struct FieldInfo
{
    std::string name;
    std::string value;
};

struct ComponentInfo
{
    std::string typeName;
    std::vector<FieldInfo> fields;
};

struct EntityInfo
{
    Entity id;
    std::vector<ComponentInfo> components;
};

template <typename ComponentList>
class RunTimeInspector
{
    private:

        template <typename Tuple, typename Func, std::size_t... Is>
        static void applyImpl(const Tuple& tuple, Func&& fnc, std::index_sequence<Is...>)
        {
            (fnc(Is, std::get<Is>(tuple)), ...);
        }

        template <typename Tuple, typename Func>
        static void applyToTuple(const Tuple& tuple, Func&& fnc)
        {
            constexpr auto size = std::tuple_size<Tuple>::value;
            applyImpl(tuple, std::forward<Func>(fnc), std::make_index_sequence<size>{});
        }
    
    public:

        static EntityInfo inspectEntity(const Registry<ComponentList>& reg, Entity e)
        {
            EntityInfo info {e, {}};
            StaticForEach<ComponentList>([&](auto tag) {
                using T = typename decltype(tag)::type;
                if (reg.template has<T>(e))
                {
                    const T& comp = reg.template get<T>(e);
                    info.components.push_back(inspectComponent<T>(comp));
                }
            });
            return info;
        }

        template <typename T>
        static ComponentInfo inspectComponent(const T& comp)
        {
            ComponentInfo ci;
            ci.typeName = typeid(T).name();
            auto values = comp.tie(comp);
            auto names = comp.fieldNames();
            applyToTuple(values, [&](std::size_t i, const auto& field) {
                std::ostringstream oss;
                oss << field;
                ci.fields.push_back({names[i], oss.str()});
            });
            return ci;
        }
};