#pragma once

#include <functional>
#include "Storage.hpp"

// === EventTrait

template <typename T>
class EventDispatcher
{
    private:

        int _nextId;
        std::vector<std::pair<int, std::function<void(const T&)>>> _handlers;

    public:

        int subscribe(std::function<void(const T&)> handler)
        {
            int id = _nextId++;
            _handlers.emplace_back(id, std::move(handler));
            return id;
        }

        void unsubscribe(int id)
        {
            _handlers.erase(std::remove_if(_handlers.begin(), _handlers.end(), [id](const auto& pair) {
                return pair.first == id;
            }), _handlers.end());
        }

        void publish(const T& event)
        {
            for (const auto& h : _handlers)
                h.second(event);
        }
};

class EventBus
{
    private:

        template <typename T>
        EventDispatcher<T>& getDispatcher(void)
        {
            static EventDispatcher<T> dispatcher;
            return dispatcher;
        }

    public:

        template <typename T>
        int subscribe(std::function<void(const T&)> handler)
        {
            return getDispatcher<T>().subscribe(std::move(handler));
        }

        template <typename T>
        void unsubscribe(int id)
        {
            getDispatcher<T>().unsubscribe(id);
        }

        template <typename T>
        void publish(const T& e)
        {
            getDispatcher<T>().publish(e);
        }

        static EventBus& instance(void)
        {
            static EventBus bus;
            return bus;
        }
};

template <typename SceneT, typename EventT>
struct hasHandleEvent
{
    private:

        template <typename U>
        static auto test(U*) -> decltype(handleEvent(std::declval<U*>(), std::declval<const EventT&>()), std::true_type());

        template <typename>
        static auto test(...) -> std::false_type;

    public:

        static constexpr bool value = decltype(test<SceneT>(nullptr))::value;
};

template <typename T, typename SceneT>
int bindEvent(SceneT* scene, void(*handler)(SceneT*, const T&))
{
    return EventBus::instance().subscribe<T>([scene, handler](const T& event) {
        handler(scene, event);
    });
}

template <typename T>
struct EventTraits
{
    static constexpr bool isTargeted = false;
    static Entity getTarget(const T&) { return Entity{~0u, ~0u};}
};