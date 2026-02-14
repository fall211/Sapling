//
//  ComponentReflection.hpp
//  SaplingEngine
//

#pragma once

#include "Renderer/Material.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Skeleton.hpp"
#include "Renderer/Animation.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/Sprout.hpp"

#include "glm/glm.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <type_traits>

class PrefabLoader;
class Entity;

namespace Comp {

class Component;
enum class Layer : std::uint8_t;

enum class FieldType : uint8_t {
    Float, Int, Int8, UInt8, SizeT, Bool, String,
    Vec2, Vec3, Vec4,
    Color3, Color4,
    Layer, Pivot, TextJustify, ShaderType,
    TextureRef,
    ImageTextureRef,
    MeshRef,
    SkeletonRef,
    AnimClipRef,
    MaterialRef,
};

struct FieldInfo {
    const char* jsonKey;
    const char* displayName;
    FieldType type;
    void* (*getMutablePtr)(Component&);
    const void* (*getConstPtr)(const Component&);

    template<typename T>
    T& get(Component& comp) const {
        return *static_cast<T*>(getMutablePtr(comp));
    }

    template<typename T>
    const T& getConst(const Component& comp) const {
        return *static_cast<const T*>(getConstPtr(comp));
    }
};

template<typename C, typename T, T C::*Member>
inline void* fieldMutablePtr(Component& comp) {
    return &(static_cast<C&>(comp).*Member);
}

template<typename C, typename T, T C::*Member>
inline const void* fieldConstPtr(const Component& comp) {
    return &(static_cast<const C&>(comp).*Member);
}

template<typename T> struct FieldTypeTag;

template<> struct FieldTypeTag<float>              { static constexpr FieldType value = FieldType::Float; };
template<> struct FieldTypeTag<int>                { static constexpr FieldType value = FieldType::Int; };
template<> struct FieldTypeTag<int8_t>             { static constexpr FieldType value = FieldType::Int8; };
template<> struct FieldTypeTag<uint8_t>            { static constexpr FieldType value = FieldType::UInt8; };
template<> struct FieldTypeTag<size_t>             { static constexpr FieldType value = FieldType::SizeT; };
template<> struct FieldTypeTag<bool>               { static constexpr FieldType value = FieldType::Bool; };
template<> struct FieldTypeTag<std::string>        { static constexpr FieldType value = FieldType::String; };
template<> struct FieldTypeTag<glm::vec2>          { static constexpr FieldType value = FieldType::Vec2; };
template<> struct FieldTypeTag<glm::vec3>          { static constexpr FieldType value = FieldType::Vec3; };
template<> struct FieldTypeTag<glm::vec4>          { static constexpr FieldType value = FieldType::Vec4; };
template<> struct FieldTypeTag<Comp::Layer>        { static constexpr FieldType value = FieldType::Layer; };
template<> struct FieldTypeTag<Sprout::Pivot>      { static constexpr FieldType value = FieldType::Pivot; };
template<> struct FieldTypeTag<Sprout::TextJustify>{ static constexpr FieldType value = FieldType::TextJustify; };
template<> struct FieldTypeTag<Sprout::ShaderType> { static constexpr FieldType value = FieldType::ShaderType; };

template<> struct FieldTypeTag<std::shared_ptr<Sprout::Texture>>       { static constexpr FieldType value = FieldType::TextureRef; };
template<> struct FieldTypeTag<std::shared_ptr<Sprout::Mesh>>          { static constexpr FieldType value = FieldType::MeshRef; };
template<> struct FieldTypeTag<std::shared_ptr<Sprout::Material>>      { static constexpr FieldType value = FieldType::MaterialRef; };
template<> struct FieldTypeTag<std::shared_ptr<Sprout::Skeleton>>      { static constexpr FieldType value = FieldType::SkeletonRef; };
template<> struct FieldTypeTag<std::shared_ptr<Sprout::AnimationClip>> { static constexpr FieldType value = FieldType::AnimClipRef; };

template<typename T>
constexpr FieldType deduceFieldType() { return FieldTypeTag<T>::value; }

} // namespace Comp


#define SAPLING_FIELDS(CompType, ...) \
    using Self = CompType; \
    std::span<const Comp::FieldInfo> getFields() const override { \
        static const Comp::FieldInfo fields[] = { __VA_ARGS__ }; \
        return std::span<const Comp::FieldInfo>(fields); \
    }

#define FIELD(member, jsonKey, display) \
    Comp::FieldInfo{ \
        jsonKey, \
        display, \
        Comp::deduceFieldType<std::remove_cvref_t<decltype(Self::member)>>(), \
        &Comp::fieldMutablePtr<Self, std::remove_cvref_t<decltype(Self::member)>, &Self::member>, \
        &Comp::fieldConstPtr<Self, std::remove_cvref_t<decltype(Self::member)>, &Self::member> \
    }

#define FIELD_AS(member, jsonKey, display, fieldType) \
    Comp::FieldInfo{ \
        jsonKey, \
        display, \
        fieldType, \
        &Comp::fieldMutablePtr<Self, std::remove_cvref_t<decltype(Self::member)>, &Self::member>, \
        &Comp::fieldConstPtr<Self, std::remove_cvref_t<decltype(Self::member)>, &Self::member> \
    }

#define REGISTER_COMPONENT(CompType, compName) \
    const char* componentName() const override { return compName; } \
    static bool _doRegister(); \
    static inline const bool _registered = _doRegister();

#define REGISTER_COMPONENT_IMPL(CompType, compName) \
    bool Comp::CompType::_doRegister() { \
        PrefabLoader::getInstance().registerFactory(compName, \
            [](const std::shared_ptr<Entity>& entity, const nlohmann::json& data) { \
                auto& comp = entity->addComponent<Comp::CompType>(); \
                comp.deserialize(data); \
            }); \
        return true; \
    }
