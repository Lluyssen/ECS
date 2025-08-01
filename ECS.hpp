
#include "Registry.hpp"
#include "System.hpp"
#include "RunTimeInspector.hpp"
#include "Bus.hpp"
#include <unordered_map>
#include <typeindex>
#include <functional>

// === Scene ===

class IScene
{
    public:

        virtual ~IScene(void) = default;
        virtual void update(double) = 0;
        virtual const std::string& name(void) = 0;
};

template <typename ComponentList>
class Scene : public IScene
{
    private:
        
        std::string _sceneName;
        Registry<ComponentList> _registry;
        SystemManager<ComponentList> _systems;
        std::unordered_map<std::type_index, std::function<void(const void*)>> _routers;

    public:

        Scene(const std::string& name) : _sceneName(name) {}
        virtual ~Scene(void) = default;

        template <typename T>
        void addSystem(T* sys, int priority = 0)
        {
            _systems.addSystem(sys, _registry, priority);
        }

        Registry<ComponentList>& getRegistry(void)
        {
            return _registry;
        }

        void update(double dt) override
        {
            std::cout << "Scene [" << _sceneName << "] ";
            _systems.update(dt, _registry);
        }

        const std::string& name(void) override
        {
            return _sceneName;
        }

        const std::vector<std::unique_ptr<System<ComponentList>>>& systems(void) const
        {
            return _systems.systems();
        }

        template <typename Event, typename T>
        void routeEvent(const Event& evt)
        {
            auto& reg = getRegistry();
            if constexpr (EventTraits<Event>::isTargeted)
            {
                Entity tgt = EventTraits<Event>::getTarget(evt);
                if (reg.template has<T>(tgt))
                    handleEvent(this, evt, tgt);
            }
            else
            {
                reg.template forEachEntityWith<TypeList<T>>([&](Entity e, T& comp) {
                    (void)comp;
                    handleEvent(this, evt, e);
                });
            }
        }

        template <typename Event, typename T>
        void addEventRouter(void)
        {
            _routers[typeid(Event)] = [this](const void* e){
                const Event& evt = *static_cast<const Event*>(e);
                this->routeEvent<Event, T>(evt);
            };
        }

        template <typename Event>
        void bindRouter(void)
        {
            EventBus::instance().subscribe<Event>([this](const Event& evt) {
                auto it = _routers.find(typeid(Event));
                if (it != _routers.end())
                    it->second(&evt);
            });
        }

        std::vector<std::string> listActiveSystem(void) const
        {
            std::vector<std::string> out;
            for (const auto& sys : _systems.systems())
                out.push_back(sys->name());
            return out;
        }
};

// === GameManager ===

class GameManager
{
    private:

        std::unordered_map<std::string, std::pair<std::unique_ptr<IScene>, bool>> _scenes;

    public:

        GameManager(void) = default;
        virtual ~GameManager(void) = default;

        template <typename ComponentList>
        Scene<ComponentList>& createScene(const std::string& name, bool active = false, std::size_t alloc = 100)
        {
            auto scene = std::make_unique<Scene<ComponentList>>(name);
            scene->getRegistry().preAllocate(alloc);
            Scene<ComponentList>* ptr = scene.get();
            _scenes[name] = {std::move(scene), active};
            return *ptr;
        }

        void setActiveScene(const std::string& name, bool b)
        {
            auto it = _scenes.find(name);
            if (it != _scenes.end())
                it->second.second = b;
        }

        IScene* getScene(const std::string& name)
        {
            auto it = _scenes.find(name);
            return it != _scenes.end() ? it->second.first.get() : nullptr;
        }

        void run(int frames = 3, double dt = 1.0)
        {
            for (int i = 0; i < frames; i++)
            {
                std::cout << "\nFrame " << i << std::endl;
                for (auto& [name, scene] : _scenes)
                {
                    if (scene.second)
                    {
                        std::cout << "Scene[" << name << "] ";
                        scene.first->update(dt);
                    }
                }
            }
        }

        void update(double dt)
        {
            for (auto& [name, scene] : _scenes)
            {
                if (scene.second)
                {
                    std::cout << "Scene[" << name << "] ";
                    scene.first->update(dt);
                }
            }
        }
};