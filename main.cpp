#include "ECS.hpp"

struct Task
{
    std::string description;
    using Types = TypeList<std::string>;
    static auto tie(const Task& obj) { return std::tie(obj.description);}
    std::array<const char*, 1> fieldNames(void) const {return {"Task"};}
};

struct Status
{
    bool completed = false;
    using Types = TypeList<bool>;
    static auto tie(const Status& obj) { return std::tie(obj.completed);}
    std::array<const char*, 1> fieldNames(void) const {return {"Status"};}
    
};

struct Priority 
{
    int level = 1;
    using Types = TypeList<int>;
    static auto tie(const Priority& obj) { return std::tie(obj.level);}
    std::array<const char*, 1> fieldNames(void) const {return {"Priority"};}
};

struct Deadline 
{
    int daysLeft = 0;
    using Types = TypeList<int>;
    static auto tie(const Deadline& obj) { return std::tie(obj.daysLeft);}
    std::array<const char*, 1> fieldNames(void) const {return {"Deadline"};}
};

// Système de progression
class TaskProgressSystem : public SystemTypeList<TypeList<Task, Status, Priority, Deadline>> 
{
    public:

        void update(double, Registry<Signature>& reg) override 
        {
            reg.template forEachEntityWith<Signature>([](Entity, Task& t, Status& s, Priority&, Deadline&) {
                if (!s.completed) {
                    std::cout << "[Task] " << t.description << " is still in progress" << std::endl;
                }
            });
        }
        const char* name(void) const override { return "TaskProgressSystem"; }
};

// Système de deadline
class DeadlineSystem : public SystemTypeList<TypeList<Task, Status, Priority, Deadline>> 
{
    public:

        void update(double, Registry<Signature>& reg) override 
        {
            reg.template forEachEntityWith<Signature>([](Entity e, Task&, Status&, Priority&, Deadline& d) {
                if (d.daysLeft > 0) d.daysLeft--;
                std::cout << "[Deadline] Task " << e.id << " has " << d.daysLeft << " day(s) left" << std::endl;
            });
        }
        const char* name(void) const override { return "DeadlineSystem"; }
};

// Système de print
class TaskPrintSystem : public SystemTypeList<TypeList<Task, Status, Priority, Deadline>> 
{
    public:

        void update(double, Registry<Signature>& reg) override 
        {
            reg.template forEachEntityWith<Signature>([](Entity, Task& t, Status& s, Priority& p, Deadline&) {
                std::cout << "[Task] " << t.description << " | Priority: " << p.level << " | Status: " << (s.completed ? "V" : "X") << std::endl;
            });
        }
        const char* name(void) const override { return "TaskPrintSystem"; }
};

// Event : TaskCompleted
struct TaskCompletedEvent 
{
    Entity target;
};

template<>
struct EventTraits<TaskCompletedEvent> 
{
    static constexpr bool isTargeted = true;
    static Entity getTarget(const TaskCompletedEvent& e) {return e.target;}
};

// Event handler
template <typename ComponentList>
void handleEvent(Scene<ComponentList>* scene, const TaskCompletedEvent&, Entity target) 
{
    auto& reg = scene->getRegistry();
    if (auto* status = reg.template getIf<Status>(target))
    {
        status->completed = true;
        std::cout << " Task " << target.id << " has been marked as completed!" << std::endl;
    }
}

int main(void) 
{
    using Components = TypeList<Task, Status, Priority, Deadline>;
    GameManager manager;

    // Scene 1 : Team Alpha
    auto& teamAlpha = manager.createScene<Components>("TeamAlpha", true);
    auto& reg1 = teamAlpha.getRegistry();

    Entity t1 = reg1.create();
    reg1.add<Task>(t1, {"Implement ECS"});
    reg1.add<Status>(t1, {false});
    reg1.add<Priority>(t1, {2});
    reg1.add<Deadline>(t1, {5});

    teamAlpha.addSystem(new TaskProgressSystem(), 10);
    teamAlpha.addSystem(new DeadlineSystem(), 20);
    teamAlpha.addSystem(new TaskPrintSystem(), 30);
    teamAlpha.bindRouter<TaskCompletedEvent>();

    // Scene 2 : Team Beta
    auto& teamBeta = manager.createScene<Components>("TeamBeta", true);
    auto& reg2 = teamBeta.getRegistry();

    Entity t2 = reg2.create();
    reg2.add<Task>(t2, {"Write Documentation"});
    reg2.add<Status>(t2, {false});
    reg2.add<Priority>(t2, {1});
    reg2.add<Deadline>(t2, {3});

    teamBeta.addSystem(new TaskPrintSystem(), 10);
    teamBeta.addSystem(new DeadlineSystem(), 20);
    teamBeta.bindRouter<TaskCompletedEvent>();

    manager.run(3, 1.0);

    EventBus::instance().publish(TaskCompletedEvent{t2});
    std::cout << "After completion:" << std::endl;
    manager.run(1, 0.5);

    
    std::cout << "\n Introspection Runtime :" << std::endl;
    RunTimeInspector<Components> inspector;

    for (Entity e : reg1.getAliveEntities()) 
    {
        auto info = inspector.inspectEntity(reg1, e);
        std::cout << "Entity " << info.id.id << std::endl;
        for (const auto& comp : info.components) {
            std::cout << " - " << comp.typeName << " : { ";
            for (const auto& field : comp.fields) {
                std::cout << field.name << " = " << field.value << "; ";
            }
            std::cout << "}" << std::endl;
        }
    }

    return 0;
}