#pragma once

#include "TypeList.hpp"
#include <memory>

// === Systèmes ===

template <typename... Ts>
struct System
{
    using Signature = TypeList<Ts...>;
    virtual ~System(void) = default;
    virtual void update(double, Registry<Signature>&) = 0;
    virtual const char* name(void) const = 0;
};

template <typename T>
struct SystemTypeList;

template <typename... Ts>
struct SystemTypeList<TypeList<Ts...>> : public System<Ts...>
{
    using Signature = TypeList<Ts...>;
};

struct ISystem
{
    virtual ~ISystem(void) = default;
    virtual void update(double) = 0;
    virtual const char* name(void) const = 0;
    int priority = 0;
};

template <typename SystemT, typename ComponentList>
struct SystemWrapper : public ISystem
{
    SystemT* _system;
    Registry<ComponentList>& _registry;

    SystemWrapper(SystemT* sys, Registry<ComponentList>& reg) : _system(sys), _registry(reg) {}

    void update(double dt) override
    {
        _system->update(dt, _registry);
    }

    const char* name(void) const override
    {
        return _system->name();
    }
};


template <typename ComponentList>
class SystemManager
{
    private:

        std::vector<std::unique_ptr<ISystem>> _systems;

    public:

        SystemManager(void) = default;
        virtual ~SystemManager(void) = default;

        template <typename SystemT>
        void addSystem(SystemT* sys, Registry<ComponentList>& reg, int priority = 0)
        {
            auto ptr = std::make_unique<SystemWrapper<SystemT, ComponentList>>(sys, reg);
            ptr->priority = priority;
            _systems.push_back(std::move(ptr));
            std::sort(_systems.begin(), _systems.end(), [](const auto& a, const auto& b) {
                return a->priority < b->priority;
            });
        }

        template <typename SystemT, typename... Args>
        void addWithPriority(int priority, Args&&... args)
        {
            auto sys = new SystemT(std::forward<Args>(args)...);
            addSystem(sys, priority);
        }

        void update(double dt, Registry<ComponentList>&)
        {
            for (auto& s : _systems)
            {
                std::cout << " || System[" << s->name() << "] ";
                s->update(dt);
            }
        }

        const std::vector<std::unique_ptr<System<ComponentList>>>& systems(void) const
        {
            return reinterpret_cast<const std::vector<std::unique_ptr<System<ComponentList>>>&>(_systems);
        }
};