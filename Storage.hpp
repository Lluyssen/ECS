#pragma once

#include <vector>
#include <cstdint>
#include <unordered_map>

// === Entity / Manager  ===

struct Entity 
{
    std::uint32_t id = 0;
    std::uint32_t version = 1;
    bool operator==(const Entity& other) const
    {
        return (id == other.id && version == other.version);
    }
    bool operator!=(const Entity& other) const
    {
        return (id != other.id && version != other.version);
    }
};

constexpr Entity INVALID_ENTITY {~0u, ~0u};

class EntityManager 
{
    private:

        std::uint32_t nextId = 0;
        std::vector<Entity> free;
        std::unordered_map<std::uint32_t, Entity> alive;

    public:

        virtual ~EntityManager(void) = default;

        Entity create(void) 
        {
            if (!free.empty()) 
            {
                Entity e = free.back();
                free.pop_back();
                e.version++;
                alive[e.id] = e;
                return e;
            }
            Entity e {nextId++, 1};
            alive[e.id] = e;
            return e;
        }

        void destroy(Entity e) 
        {
            if (e.id >= nextId)
                return;
            free.push_back(e);
            alive.erase(e.id);
        }

        void preAllocate(std::size_t count)
        {
            for (std::size_t i = 0; i < count; i++)
                free.push_back(Entity{nextId++, 1});
        }

        void reset(void)
        {
            nextId = 0;
            free.clear();
        }

        std::size_t freeCount(void) const
        {
            return free.size();
        }

        std::size_t allocated(void) const
        {
            return nextId;
        }

        std::vector<Entity> getAliveEntities(void)
        {
            std::vector<Entity> v;
            for (const auto& pair : alive)
                v.push_back(pair.second);
            return v;
        }
};

// === SparseSet Storage ===

template <typename T>
class ComponentStorage 
{
    private:

        static constexpr std::size_t INVALID = static_cast<std::size_t>(-1);
        std::vector<std::size_t> sparse;
        std::vector<Entity> denseEntities;
        std::vector<T> denseData;

        void ensure(const std::size_t& id) 
        {
            if (id >= sparse.size()) 
                sparse.resize(id + 1, INVALID);
        }

    public:

        virtual ~ComponentStorage(void) = default;

        bool has(Entity e) const
        {
            return e.id < sparse.size() && sparse[e.id] != INVALID && denseEntities[sparse[e.id]].id == e.id;
        }

        T& emplace(Entity e, const T& value)
        {
            ensure(e.id);
            if (!has(e))
            {
                std::size_t i = denseData.size();
                sparse[e.id] = i;
                denseEntities.push_back(e);
                denseData.push_back(value);
                return denseData.back();
            }
            denseData[sparse[e.id]] = value;
            return get(e);
        }

        void remove(Entity e) 
        {
            if (!has(e))
                return;
            std::size_t index = sparse[e.id];
            std::size_t last = denseData.size() - 1;
            if (index != last) 
            {
                denseEntities[index] = denseEntities[last];
                denseData[index] = denseData[last];
                sparse[denseEntities[index].id] = index;
            }
            denseEntities.pop_back();
            denseData.pop_back();
            sparse[e.id] = INVALID;
        }

        T& get(Entity e) 
        {
            return denseData[sparse[e.id]];
        }

        const T& get(Entity e) const
        {
            return denseData[sparse[e.id]];
        }

        std::vector<Entity> entities(void) const
        {
            return denseEntities;
        }
};