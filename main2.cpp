#include "ECS.hpp"

struct Position { float x = 0, y = 0; };
struct Velocity { float vx = 0, vy = 0; };

class MovementSystem : public SystemTypeList<TypeList<Position, Velocity>>
{
    public:

        void update(double dt, Registry<Signature>& reg) override 
        {
            reg.template forEachEntityWith<Signature>([&](Entity, Position& pos, Velocity& vel) {
                pos.x += vel.vx * dt;
                pos.y += vel.vy * dt;
                std::cout << "  PX : " << pos.x << " PY : " << pos.y << std::endl;
            });
        }

        const char* name(void) const override
        {
            return "MovementSystem";
        }
};

struct Heal { int hp; };
struct Mana { int mp; };

class RegenSystem : public SystemTypeList<TypeList<Heal, Mana>>
{
    public:

        void update(double, Registry<Signature>& reg) override
        {
            reg.template forEachEntityWith<Signature>([](Entity, Heal& h, Mana& m) {
                h.hp +=1;
                m.mp += 2;
            });
        }

        const char* name(void) const override
        {
            return "RegenSystem";
        }
};

class PrintSystem : public SystemTypeList<TypeList<Heal, Mana>>
{
    public:

        void update(double, Registry<Signature>& reg) override
        {
            reg.template forEachEntityWith<Signature>([](Entity& e, Heal& h, Mana& m) {
                std::cout << " Entity " << e.id << " :  HP : " << h.hp << " MP: " << m.mp << " | ";
            });
            std::cout << "\n";
        }

        const char* name(void) const override
        {
            return "PrintSystem";
        }
};

struct DamageEvent 
{ 
    int amount;
    Entity target;
};

template <>
struct EventTraits<DamageEvent>
{
    static constexpr bool isTargeted = true;
    static Entity getTarget(const DamageEvent& e) { return e.target;}
};

void handleEvent(Scene<TypeList<Heal, Mana>>* scene, const DamageEvent& evt, Entity target = INVALID_ENTITY)
{
    auto& reg = scene->getRegistry();
    auto* h = reg.template getIf<Heal>(target);
    h->hp -= evt.amount;
    std::cout << "[EventDamage] on target.id " << target.id << " took " << evt.amount << " damage, rest " << h->hp << " hp" << std::endl;
}


int main() 
{
    using Components1 = TypeList<Position, Velocity>;
    using Components2 = TypeList<Heal, Mana>;
    GameManager manager;

    auto& scene1 = manager.createScene<Components1>("scene1", true);
    auto& reg1 = scene1.getRegistry();
    Entity e1 = reg1.create();
    reg1.add<Position>(e1, {0, 0});
    reg1.add<Velocity>(e1, {1, 1});
    scene1.addSystem(new MovementSystem);

    auto& scene2 = manager.createScene<Components2>("scene2", true);
    auto& reg2 = scene2.getRegistry();
    Entity e2 = reg2.create();
    Entity e22 = reg2.create();
    reg2.add<Heal>(e2, {20});
    reg2.add<Mana>(e2, {20});
    reg2.add<Heal>(e22, {10});
    reg2.add<Mana>(e22, {10});
    scene2.addSystem(new PrintSystem, 50);
    scene2.addSystem(new RegenSystem, 10);
    scene2.addEventRouter<DamageEvent, Heal>();
    scene2.bindRouter<DamageEvent>();

    GameManager manager2;
    auto& scene21 = manager2.createScene<Components1>("scene21", true);
    auto& reg21 = scene21.getRegistry();
    Entity e21 = reg21.create();
    reg21.add<Position>(e21, {0, 0});
    reg21.add<Velocity>(e21, {1, 1});
    scene21.addSystem(new MovementSystem);

    manager.run(3, 1.0);
    manager2.run(3, 1.0);

    EventBus::instance().publish(DamageEvent{5, e2});
    EventBus::instance().publish(DamageEvent{5, INVALID_ENTITY});

    std::cout << "-----" << std::endl;
    for (int i = 0; i < 5; i++)
    {
        std::cout << "GameManager 1 :";
        manager.update(0.5f);
        std::cout << "GameManager 2 :";
        manager2.update(0.5f);
    }
    return 0;
}