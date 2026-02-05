//
//  Entity.cpp
//  SaplingEngine
//

#include "ECS/Entity.hpp"
#include "ECS/EntityManager.hpp"

#include <any>
#include <memory>



Entity::Entity(TagList  tags, const size_t id, std::shared_ptr<EntityManager> owner) 
    :   m_tags(std::move(tags)), 
        m_id(id),
        m_owner(std::move(owner))
    {}
    
void Entity::setName(const std::string& name) { m_name = name; }

std::string& Entity::getName() { return m_name; }

auto Entity::getId() const -> size_t { return m_id; }

auto Entity::getTags() -> const TagList& { return m_tags; }

auto Entity::hasTag(const std::string& tag) -> bool
{
    return std::find(m_tags.begin(), m_tags.end(), tag) != m_tags.end();
}

auto Entity::isActive() const -> bool { return m_active; }

void Entity::destroy() { m_active = false; }

void Entity::addTag(const std::string& tag)
{
    if (std::find(m_tags.begin(), m_tags.end(), tag) == m_tags.end())
    {
        m_tags.push_back(tag);
    }
}

void Entity::removeTag(const std::string& tag)
{
    if (const auto it = std::find(m_tags.begin(), m_tags.end(), tag); it != m_tags.end())
    {
        m_tags.erase(it);
    }
}

void Entity::requestAddTag(const std::string& tag)
{
    m_owner->addTagToEntity(shared_from_this(), tag);
}

void Entity::requestRemoveTag(const std::string& tag)
{
    m_owner->removeTagFromEntity(shared_from_this(), tag);
}

