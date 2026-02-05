//
//  Entity.hpp
//  SaplingEngine
//

#pragma once

#include "ECS/Component.hpp"
#include "Utility/Debug.hpp"

#include <string>
#include <tuple>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <vector>
#include <functional>
#include <any>
#include <iostream>
#include <algorithm>
#include <type_traits>
#include <utility>
#include <memory>


class Component;
class EntityManager;

typedef std::vector<std::string> TagList;
typedef std::shared_ptr<Entity> Inst;

class Entity : public std::enable_shared_from_this<Entity>
{
    private:
        std::string m_name = "";
        TagList m_tags;
        size_t m_id = 0;
        std::shared_ptr<EntityManager> m_owner;
        bool m_active = true;
        std::unordered_map<std::type_index, std::shared_ptr<Comp::Component>> m_components;
        
        // events
        std::unordered_map<std::string, std::vector<std::function<void(const std::vector<std::any>&)>>> m_eventCallbacks;
    
        template<size_t I = 0, typename... Tp>
        static void UnpackArgs(std::tuple<Tp...>& tup, const std::vector<std::any>& args)
        {
            if constexpr (I < sizeof...(Tp))
            {
                std::get<I>(tup) = std::any_cast<std::tuple_element_t<I, std::tuple<Tp...>>>(args[I]);
                UnpackArgs<I + 1, Tp...>(tup, args);
            }
        }
        
        // only the EntityManager can create entities
        friend class EntityManager;
        Entity(TagList tags, size_t id, std::shared_ptr<EntityManager> owner);
        
        /*
            * Adds a tag to the entity, only accessible by the EntityManager
            * @param tag The tag to add
        */
        void addTag(const std::string& tag);
        
        /*
            * Removes a tag from the entity, only accessible by the EntityManager
            * @param tag The tag to remove
        */
        void removeTag(const std::string& tag);

        
    public:
    
        void setName(const std::string& name);
        std::string& getName();
    
        /* 
            * Requests the addition of a tag to the entity through the entity manager.
            * @param tag The tag to add
        */
        void requestAddTag(const std::string& tag);
        
        /* 
            * Requests the removal of a tag from the entity through the entity manager.
            * @param tag The tag to remove
        */
        void requestRemoveTag(const std::string& tag);
    
        /*
            * Gets the id of the entity
            * @return The id of the entity
        */
        auto getId() const -> size_t;
        
        /*
            * Gets the tags of the entity
            * @return The tags of the entity
        */
        auto getTags() -> const TagList&;
        
        /*
            * Checks if the entity has a tag
            * @param tag The tag to check for
            * @return True if the entity has the tag
        */
        auto hasTag(const std::string& tag) -> bool;
        
        /*
            * Checks if the entity is active
            * @return True if the entity is active
        */
        auto isActive() const -> bool;
        
        /*
            * Marks entity for destruction by the EntityManager
        */
        void destroy();
    
        /*
            * Adds a component to the entity
            * @tparam T The type of the component
            * @tparam Args The arguments to pass to the component constructor
            * @param args The arguments to pass to the component constructor
            * @return The added component
        */
        template <typename T, typename... Args>
        auto addComponent(Args&&... args) -> T& {
            std::shared_ptr<T> component = std::make_shared<T>(shared_from_this(), std::forward<Args>(args)...);
            m_components[typeid(T)] = std::move(component);
            m_components[typeid(T)]->OnAddToEntity();
            return *static_cast<T*>(m_components[typeid(T)].get());
        }
    
        /*
            * Removes a component from the entity
            * @tparam T The type of the component
        */
        template <typename T>
        void removeComponent() {
            auto it = m_components.find(typeid(T));
            if (it != m_components.end()) {
                it->second->OnRemoveFromEntity();
                m_components.erase(it);
            } else {
                Debug::log("Trying to remove a component that doesn't exist!");
            }
        }
    
        /*
            * Gets a component from the entity
            * @tparam T The type of the component
            * @return The component
        */
        template <typename T>
        auto getComponent() const -> T& {
            const auto it = m_components.find(typeid(T));
            return *dynamic_cast<T*>(it->second.get());
        }
    
        /*
            * Checks if the entity has a component
            * @tparam T The type of the component
            * @return True if the entity has the component
        */
        template <typename T>
        auto hasComponent() const -> bool {
            return m_components.find(typeid(T)) != m_components.end();
        }
        
        /*
            * Checks if the entity has a component and if it is enabled
            * @tparam T The type of the component
            * @return True if the entity has the component
        */
        template <typename T>
        auto hasComponentEnabled() const -> bool {
            bool has = m_components.find(typeid(T)) != m_components.end();
            bool hasEnabled = has && m_components.at(typeid(T))->enabled;
            return hasEnabled;
        }
        
        // events
        
        /*
            * Listens for an event on the entity
            * @param event The event to listen for
            * @param callback The callback to call when the event is received
        */
        template<typename... Args>
        void ListenForEvent(const std::string& event, std::function<void(Args...)> callback)
        {
            auto wrapper = [callback](const std::vector<std::any>& args) {
                std::tuple<std::decay_t<Args>...> tuple_args;
                UnpackArgs(tuple_args, args);
                std::apply(callback, tuple_args);
            };
    
            m_eventCallbacks[event].push_back(wrapper);
        }
        
        /*
            * Removes an event callback from the entity
            * @param event The event to remove the callback from
            * @param callback The callback to remove
        */
        template <typename... Args>
        void RemoveEventCallback(const std::string& event, const std::function<void(Inst, Args...)>& callback)
        {
            if (m_eventCallbacks.find(event) != m_eventCallbacks.end())
            {
                auto& callbacks = m_eventCallbacks[event];
                callbacks.erase(std::remove_if(callbacks.begin(), callbacks.end(), 
                    [&callback](const auto& cb) { return cb.template target<void(Inst, Args...)>() == callback.template target<void(Inst, Args...)>(); }), 
                    callbacks.end());
            }
        }

        /*
            * Pushes an event to the entity
            * @param event The event to push
            * @param args The arguments to pass to the event
        */
        template<typename... Args>
        void PushEvent(const std::string& event, Args&&... args)
        {
            if (m_eventCallbacks.find(event) != m_eventCallbacks.end())
            {
                std::vector<std::any> arg_vector{std::forward<Args>(args)...};
                for (const auto& callback : m_eventCallbacks[event])
                {
                    callback(arg_vector);
                }
            }
        }
};