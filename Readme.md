# ⚙️ ECS Pro – Moteur ECS en C++17 orienté Métaprogrammation

---
Moteur ECS (Entity Component System) modulaire, extensible et 100 % typé
il repose sur une architecture à base de **TypeList** (voir répo TypeList Métaprog)
Compile time et runTime
---

## ✨ Fonctionnalités principales

- ✅ EntityManager versionné, sécurisé, performant
- 📦 ComponentStorage<T> avec accès O(1) via `SparseSet`
- 🧱 Registry typé par `TypeList<Ts...>` (aucun cast, aucun RTTI)
- 🧠 System<Ts...> priorisés, avec signature typée compile-time
- 🌀 Scenes indépendantes orchestrées par GameManager
- 📬 EventBus / EventRouter avec dispatch ciblé (via EventTraits)
- 🔍 Introspection runtime minimale via `tie()` et `fieldNames()`
- ⚡ Group<Ts...> pour itération directe sans has<T>() inutile
- 🔁 Multi-scenes, multi-contextes isolés et parallèles

---

## 🧠 Métaprogrammation en C++17

ECS Pro s'appuie sur un système compile-time de `TypeList` :

```cpp
template <typename... Ts>
struct TypeList {};
```

Cela permet :

- La génération statique de signatures système : `System<Plant, Hunger>`
- L’itération statique sur les types : `StaticForEach<TypeList<...>>`
- L’application d’une fonction sur tous les composants d’une entité : `ApplyWith`
- Une réflexion minimaliste grâce à `tie()` et `fieldNames()`

### 🔧 Exemple : StaticForEach

```cpp
StaticForEach<TypeList<Plant, Insect>>([](auto tag) {
    using T = typename decltype(tag)::type;
    std::cout << "Component: " << typeid(T).name() << "\n";
});
```

### 🧠 Exemple : introspection statique

```cpp
struct Plant 
{
    float growth;
    int age;

    auto tie(void) { return std::tie(growth, age); }
    static std::array<const char*, 2> fieldNames(void) const { return { "growth", "age" }; }
};
```

---

## 📊 Statistiques du code

Benchmark typique sur 100 000 entités
Opération   Durée estimée (release, GCC/Clang, -O2)   

| Opération                      | Durée estimée               |
|--------------------------------|-----------------------------|
| Création de 100k entités       | ~2-5 ms                     |
| Ajout de 3 composants          | ~5-10 ms                    |
| Boucle forEachEntityWith<Ts>   | ~1-3 ms                     |
| Dispatch d’événement ciblé     | ~0.01 ms                    |
| Dispatch d’événement broadcast | ~0.1 ms                     |
| Inspection runtime             | ~5-20 µs                    |
| update() complet 100k entités  | ~5-15 ms                    |

---

## 🔬 Exemples d’utilisation

### 📦 Group<Ts...>

```cpp
auto group = registry.group<Plant, Growth>();
for (auto&& [e, plant, growth] : group)
    plant.size += growth.rate;
```

### 🧠 Introspection runtime

```cpp
auto info = RuntimeInspector<Components>::inspectEntity(registry, entity);
for (auto& comp : info.components)
 {
    std::cout << comp.typeName << ":";
    for (auto& field : comp.fields)
        std::cout << "  " << field.name << " = " << field.value << "";
}
```

### 📬 Événements ciblés

```cpp
struct FireEvent { Entity target; };
template<> struct EventTraits<FireEvent> 
{
    static constexpr bool isTargeted = true;
    static Entity getTarget(const FireEvent& e) { return e.target; }
};
```

---

## 🏗️ Structure du projet

```
ecs_pro/
├── core/
│   ├── Entity.hpp
│   ├── TypeList.hpp
│   ├── Registry.hpp
├── system/
│   ├── System.hpp
│   ├── SystemManager.hpp
│   ├── Group.hpp
├── events/
│   ├── EventBus.hpp
│   ├── EventTraits.hpp
│   ├── EventRouter.hpp
├── scene/
│   ├── Scene.hpp
│   ├── GameManager.hpp
│   ├── RuntimeInspector.hpp
├── main.cpp
└── README.md
```

## PS

Contient actuellement deux mains explications montrant l'ensemble des possibilitées de cette Ecs
Projet perso n'ayant pas de but précis en dehors de trouver un cas d'utilisation au repo TypeList
