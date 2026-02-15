//
//  Component.cpp
//  SaplingEngine
//

#include "ECS/Component.hpp"
#include "ECS/Entity.hpp"
#include "Core/AssetManager.hpp"
#include "Core/PrefabLoader.hpp"
#include "Core/Logger.hpp"
#include "Renderer/Texture.hpp"

#include <nlohmann/json.hpp>

#include <cmath>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Comp
{
    static Sprout::Pivot stringToPivot(const std::string& str)
    {
        static const std::unordered_map<std::string, Sprout::Pivot> map =
        {
            {"TOP_LEFT", Sprout::Pivot::TOP_LEFT}, {"TOP_CENTER", Sprout::Pivot::TOP_CENTER},
            {"TOP_RIGHT", Sprout::Pivot::TOP_RIGHT}, {"CENTER_LEFT", Sprout::Pivot::CENTER_LEFT},
            {"CENTER", Sprout::Pivot::CENTER}, {"CENTER_RIGHT", Sprout::Pivot::CENTER_RIGHT},
            {"BOTTOM_LEFT", Sprout::Pivot::BOTTOM_LEFT}, {"BOTTOM_CENTER", Sprout::Pivot::BOTTOM_CENTER},
            {"BOTTOM_RIGHT", Sprout::Pivot::BOTTOM_RIGHT}
        };
        auto it = map.find(str);
        return (it != map.end()) ? it->second : Sprout::Pivot::TOP_LEFT;
    }

    static Layer stringToLayer(const std::string& str)
    {
        static const std::unordered_map<std::string, Layer> map =
        {
            {"Background", Layer::Background}, {"Midground", Layer::Midground},
            {"Player", Layer::Player}, {"Foreground", Layer::Foreground},
            {"UserInterface", Layer::UserInterface}
        };
        auto it = map.find(str);
        return (it != map.end()) ? it->second : Layer::Midground;
    }

    static Sprout::TextJustify stringToTextJustify(const std::string& str)
    {
        static const std::unordered_map<std::string, Sprout::TextJustify> map =
        {
            {"LEFT", Sprout::TextJustify::LEFT}, {"CENTER", Sprout::TextJustify::CENTER},
            {"RIGHT", Sprout::TextJustify::RIGHT}
        };
        auto it = map.find(str);
        return (it != map.end()) ? it->second : Sprout::TextJustify::LEFT;
    }

    static Sprout::ShaderType stringToShaderType(const std::string& str)
    {
        static const std::unordered_map<std::string, Sprout::ShaderType> map =
        {
            {"Mesh3D", Sprout::ShaderType::Mesh3D}, {"Mesh3DSkinned", Sprout::ShaderType::Mesh3D},
            {"Custom", Sprout::ShaderType::Custom}
        };
        auto it = map.find(str);
        return (it != map.end()) ? it->second : Sprout::ShaderType::Mesh3D;
    }

    static std::string pivotToString(Sprout::Pivot p)
    {
        switch (p)
        {
            case Sprout::Pivot::TOP_LEFT: return "TOP_LEFT";
            case Sprout::Pivot::TOP_CENTER: return "TOP_CENTER";
            case Sprout::Pivot::TOP_RIGHT: return "TOP_RIGHT";
            case Sprout::Pivot::CENTER_LEFT: return "CENTER_LEFT";
            case Sprout::Pivot::CENTER: return "CENTER";
            case Sprout::Pivot::CENTER_RIGHT: return "CENTER_RIGHT";
            case Sprout::Pivot::BOTTOM_LEFT: return "BOTTOM_LEFT";
            case Sprout::Pivot::BOTTOM_CENTER: return "BOTTOM_CENTER";
            case Sprout::Pivot::BOTTOM_RIGHT: return "BOTTOM_RIGHT";
            default: return "TOP_LEFT";
        }
    }

    static std::string layerToString(Layer l)
    {
        switch (l)
        {
            case Layer::Background: return "Background";
            case Layer::Midground: return "Midground";
            case Layer::Player: return "Player";
            case Layer::Foreground: return "Foreground";
            case Layer::UserInterface: return "UserInterface";
            default: return "Midground";
        }
    }

    static std::string textJustifyToString(Sprout::TextJustify j)
    {
        switch (j)
        {
            case Sprout::TextJustify::LEFT: return "LEFT";
            case Sprout::TextJustify::CENTER: return "CENTER";
            case Sprout::TextJustify::RIGHT: return "RIGHT";
            default: return "LEFT";
        }
    }

    static std::string shaderTypeToString(Sprout::ShaderType s)
    {
        switch (s)
        {
            case Sprout::ShaderType::Mesh3D: return "Mesh3D";
            case Sprout::ShaderType::Custom: return "Custom";
            default: return "Mesh3D";
        }
    }

    static nlohmann::json vec2ToJson(const glm::vec2& v) { return {v.x, v.y}; }
    static nlohmann::json vec3ToJson(const glm::vec3& v) { return {v.x, v.y, v.z}; }
    static nlohmann::json vec4ToJson(const glm::vec4& v) { return {v.x, v.y, v.z, v.w}; }

    static glm::vec2 jsonToVec2(const nlohmann::json& j, const std::string& key, const glm::vec2& def)
    {
        if (j.contains(key) && j[key].is_array() && j[key].size() >= 2)
            return glm::vec2(j[key][0].get<float>(), j[key][1].get<float>());
        return def;
    }

    static glm::vec3 jsonToVec3(const nlohmann::json& j, const std::string& key, const glm::vec3& def)
    {
        if (j.contains(key) && j[key].is_array() && j[key].size() >= 3)
            return glm::vec3(j[key][0].get<float>(), j[key][1].get<float>(), j[key][2].get<float>());
        return def;
    }

    static glm::vec4 jsonToVec4(const nlohmann::json& j, const std::string& key, const glm::vec4& def)
    {
        if (j.contains(key) && j[key].is_array() && j[key].size() >= 4)
            return glm::vec4(j[key][0].get<float>(), j[key][1].get<float>(), j[key][2].get<float>(), j[key][3].get<float>());
        return def;
    }

    struct AssetRef
    {
        std::string name;
        std::string path;
        std::string skeleton;
        std::string clipName;
    };

    static AssetRef parseAssetRef(const nlohmann::json& j, const std::string& key)
    {
        AssetRef ref;
        if (!j.contains(key)) return ref;
        const auto& val = j[key];
        if (val.is_string())
        {
            ref.name = val.get<std::string>();
        }
        else if (val.is_object())
        {
            ref.path = val.value("path", std::string(""));
            ref.name = val.value("name", std::string(""));
            ref.skeleton = val.value("skeleton", std::string(""));
            ref.clipName = val.value("clipName", std::string(""));
            if (ref.name.empty() && !ref.path.empty())
            {
                std::filesystem::path p(ref.path);
                ref.name = p.stem().string();
            }
        }
        return ref;
    }

    static nlohmann::json serializeAssetRef(const std::string& name, const std::string& path)
    {
        if (path.empty()) return name;
        nlohmann::json ref;
        ref["name"] = name;
        ref["path"] = path;
        return ref;
    }

    static void deserializeField(const FieldInfo& f, Component& comp, const nlohmann::json& j)
    {
        const std::string key = f.jsonKey;
        switch (f.type)
        {
            case FieldType::Float:
                f.get<float>(comp) = j.value(key, f.getConst<float>(comp));
                break;
            case FieldType::Int:
                f.get<int>(comp) = j.value(key, f.getConst<int>(comp));
                break;
            case FieldType::Int8:
                f.get<int8_t>(comp) = j.value(key, f.getConst<int8_t>(comp));
                break;
            case FieldType::UInt8:
                f.get<uint8_t>(comp) = j.value(key, f.getConst<uint8_t>(comp));
                break;
            case FieldType::SizeT:
                f.get<size_t>(comp) = j.value(key, f.getConst<size_t>(comp));
                break;
            case FieldType::Bool:
                f.get<bool>(comp) = j.value(key, f.getConst<bool>(comp));
                break;
            case FieldType::String:
                f.get<std::string>(comp) = j.value(key, f.getConst<std::string>(comp));
                break;
            case FieldType::Vec2:
                f.get<glm::vec2>(comp) = jsonToVec2(j, key, f.getConst<glm::vec2>(comp));
                break;
            case FieldType::Vec3:
                f.get<glm::vec3>(comp) = jsonToVec3(j, key, f.getConst<glm::vec3>(comp));
                break;
            case FieldType::Vec4:
                f.get<glm::vec4>(comp) = jsonToVec4(j, key, f.getConst<glm::vec4>(comp));
                break;
            case FieldType::Color4:
                f.get<glm::vec4>(comp) = jsonToVec4(j, key, f.getConst<glm::vec4>(comp));
                break;
            case FieldType::Color3:
                f.get<glm::vec3>(comp) = jsonToVec3(j, key, f.getConst<glm::vec3>(comp));
                break;
            case FieldType::Layer:
                if (j.contains(key)) f.get<Layer>(comp) = stringToLayer(j[key].get<std::string>());
                break;
            case FieldType::Pivot:
                if (j.contains(key)) f.get<Sprout::Pivot>(comp) = stringToPivot(j[key].get<std::string>());
                break;
            case FieldType::TextJustify:
                if (j.contains(key)) f.get<Sprout::TextJustify>(comp) = stringToTextJustify(j[key].get<std::string>());
                break;
            case FieldType::ShaderType:
                if (j.contains(key)) f.get<Sprout::ShaderType>(comp) = stringToShaderType(j[key].get<std::string>());
                break;
            case FieldType::TextureRef:
            {
                auto ref = parseAssetRef(j, key);
                if (!ref.name.empty())
                {
                    auto& tex = f.get<std::shared_ptr<Sprout::Texture>>(comp);
                    try
                    {
                        if (!ref.path.empty())
                        {
                            tex = AssetManager::ensureImageTexture(ref.name, ref.path);
                        }
                        else if (AssetManager::hasTexture(ref.name))
                        {
                            tex = AssetManager::getTexture(ref.name);
                        }
                        else if (AssetManager::hasImageTexture(ref.name))
                        {
                            tex = AssetManager::getImageTexture(ref.name);
                        }
                        else
                        {
                            tex = AssetManager::getTexture(ref.name);
                        }
                    }
                    catch (const std::exception&)
                    {
                        Logger::warn(std::string(comp.componentName()) + ": texture not found: " + ref.name);
                    }
                }
                break;
            }
            case FieldType::ImageTextureRef:
            {
                auto ref = parseAssetRef(j, key);
                if (!ref.name.empty())
                {
                    auto& tex = f.get<std::shared_ptr<Sprout::Texture>>(comp);
                    try
                    {
                        if (!ref.path.empty())
                        {
                            tex = AssetManager::ensureImageTexture(ref.name, ref.path);
                        }
                        else
                        {
                            tex = AssetManager::getImageTexture(ref.name);
                        }
                    }
                    catch (const std::exception&)
                    {
                        Logger::warn(std::string(comp.componentName()) + ": texture not found: " + ref.name);
                    }
                }
                break;
            }
            case FieldType::MeshRef:
            {
                auto ref = parseAssetRef(j, key);
                if (!ref.name.empty())
                {
                    auto& mesh = f.get<std::shared_ptr<Sprout::Mesh>>(comp);
                    try
                    {
                        if (!ref.path.empty())
                        {
                            mesh = AssetManager::ensureMesh(ref.name, ref.path);
                        }
                        else
                        {
                            mesh = AssetManager::getMesh(ref.name);
                        }
                    }
                    catch (const std::exception&)
                    {
                        Logger::warn(std::string(comp.componentName()) + ": mesh not found: " + ref.name);
                    }
                }
                break;
            }
            case FieldType::SkeletonRef:
            {
                auto ref = parseAssetRef(j, key);
                if (!ref.name.empty())
                {
                    auto& skel = f.get<std::shared_ptr<Sprout::Skeleton>>(comp);
                    try
                    {
                        if (!ref.path.empty())
                        {
                            skel = AssetManager::ensureSkeleton(ref.name, ref.path);
                        }
                        else
                        {
                            skel = AssetManager::getSkeleton(ref.name);
                        }
                    }
                    catch (const std::exception&)
                    {
                        Logger::warn(std::string(comp.componentName()) + ": skeleton not found: " + ref.name);
                    }
                }
                break;
            }
            case FieldType::AnimClipRef:
                // handled in component-specific postDeserialize().
                break;
            case FieldType::MaterialRef:
            {
                if (j.contains(key))
                {
                    const auto& matData = j[key];
                    auto& mat = f.get<std::shared_ptr<Sprout::Material>>(comp);
                    mat = std::make_shared<Sprout::Material>();
                    Sprout::ShaderType shaderType = Sprout::ShaderType::Mesh3D;
                    if (matData.contains("shaderType"))
                        shaderType = stringToShaderType(matData["shaderType"].get<std::string>());
                    auto diffuseRef = parseAssetRef(matData, "diffuseTexture");
                    if (!diffuseRef.path.empty())
                    {
                        auto tex = AssetManager::ensureImageTexture(diffuseRef.name, diffuseRef.path);
                        mat->create(shaderType);
                        mat->diffuseTexture = tex;
                    }
                    else if (!diffuseRef.name.empty())
                    {
                        mat->create(shaderType, diffuseRef.name);
                    }
                    else
                    {
                        mat->create(shaderType);
                    }
                    mat->properties.baseColor = jsonToVec4(matData, "baseColor", mat->properties.baseColor);
                    mat->properties.specularStrength = matData.value("specularStrength", mat->properties.specularStrength);
                }
                break;
            }
        }
    }

    static nlohmann::json serializeField(const FieldInfo& f, const Component& comp)
    {
        switch (f.type)
        {
            case FieldType::Float:   return f.getConst<float>(comp);
            case FieldType::Int:     return f.getConst<int>(comp);
            case FieldType::Int8:    return f.getConst<int8_t>(comp);
            case FieldType::UInt8:   return f.getConst<uint8_t>(comp);
            case FieldType::SizeT:   return static_cast<uint64_t>(f.getConst<size_t>(comp));
            case FieldType::Bool:    return f.getConst<bool>(comp);
            case FieldType::String:  return f.getConst<std::string>(comp);
            case FieldType::Vec2:    return vec2ToJson(f.getConst<glm::vec2>(comp));
            case FieldType::Vec3:
            case FieldType::Color3:  return vec3ToJson(f.getConst<glm::vec3>(comp));
            case FieldType::Vec4:
            case FieldType::Color4:  return vec4ToJson(f.getConst<glm::vec4>(comp));
            case FieldType::Layer:   return layerToString(f.getConst<Layer>(comp));
            case FieldType::Pivot:   return pivotToString(f.getConst<Sprout::Pivot>(comp));
            case FieldType::TextJustify: return textJustifyToString(f.getConst<Sprout::TextJustify>(comp));
            case FieldType::ShaderType:  return shaderTypeToString(f.getConst<Sprout::ShaderType>(comp));
            case FieldType::TextureRef:
            {
                auto& tex = f.getConst<std::shared_ptr<Sprout::Texture>>(comp);
                if (!tex) return nullptr;
                std::string name = AssetManager::getTextureName(tex);
                std::string path;
                if (!name.empty())
                {
                    path = AssetManager::getTexturePath(name);
                }
                else
                {
                    name = AssetManager::getImageTextureName(tex);
                    if (!name.empty()) path = AssetManager::getImageTexturePath(name);
                }
                if (!name.empty()) return serializeAssetRef(name, path);
                return nullptr;
            }
            case FieldType::ImageTextureRef:
            {
                auto& tex = f.getConst<std::shared_ptr<Sprout::Texture>>(comp);
                if (!tex) return nullptr;
                std::string name = AssetManager::getImageTextureName(tex);
                std::string path = name.empty() ? "" : AssetManager::getImageTexturePath(name);
                if (!name.empty()) return serializeAssetRef(name, path);
                return nullptr;
            }
            case FieldType::MeshRef:
            {
                auto& mesh = f.getConst<std::shared_ptr<Sprout::Mesh>>(comp);
                if (!mesh) return nullptr;
                std::string name = AssetManager::getMeshName(mesh);
                if (!name.empty())
                {
                    std::string path = AssetManager::getMeshPath(name);
                    return serializeAssetRef(name, path);
                }
                return nullptr;
            }
            case FieldType::SkeletonRef:
            {
                auto& skel = f.getConst<std::shared_ptr<Sprout::Skeleton>>(comp);
                if (!skel) return nullptr;
                std::string name = AssetManager::getSkeletonName(skel);
                if (!name.empty())
                {
                    std::string path = AssetManager::getSkeletonPath(name);
                    return serializeAssetRef(name, path);
                }
                return nullptr;
            }
            case FieldType::AnimClipRef:
            {
                auto& clip = f.getConst<std::shared_ptr<Sprout::AnimationClip>>(comp);
                if (!clip) return nullptr;
                std::string name = AssetManager::getAnimationClipName(clip);
                if (!name.empty())
                {
                    std::string path = AssetManager::getAnimationClipPath(name);
                    return serializeAssetRef(name, path);
                }
                return nullptr;
            }
            case FieldType::MaterialRef:
            {
                auto& mat = f.getConst<std::shared_ptr<Sprout::Material>>(comp);
                if (!mat) return nullptr;
                nlohmann::json matJson;
                matJson["shaderType"] = shaderTypeToString(mat->getShaderType());
                matJson["baseColor"] = vec4ToJson(mat->properties.baseColor);
                matJson["specularStrength"] = mat->properties.specularStrength;
                if (mat->diffuseTexture)
                {
                    std::string texName = AssetManager::getImageTextureName(mat->diffuseTexture);
                    if (!texName.empty())
                    {
                        std::string texPath = AssetManager::getImageTexturePath(texName);
                        matJson["diffuseTexture"] = serializeAssetRef(texName, texPath);
                    }
                }
                return matJson;
            }
        }
        return nullptr;
    }

    void Component::deserialize(const nlohmann::json& j)
    {
        for (const auto& f : getFields())
            deserializeField(f, *this, j);
        postDeserialize();
    }

    nlohmann::json Component::serialize() const
    {
        nlohmann::json j;
        for (const auto& f : getFields())
        {
            auto val = serializeField(f, *this);
            if (!val.is_null())
                j[f.jsonKey] = std::move(val);
        }
        return j;
    }

    REGISTER_COMPONENT_IMPL(Transform, "Transform")
    REGISTER_COMPONENT_IMPL(GridTransform, "GridTransform")
    REGISTER_COMPONENT_IMPL(BBox, "BBox")
    REGISTER_COMPONENT_IMPL(BCircle, "BCircle")
    REGISTER_COMPONENT_IMPL(Sprite, "Sprite")

    REGISTER_COMPONENT_IMPL(Text, "Text")
    REGISTER_COMPONENT_IMPL(MeshRenderer, "MeshRenderer")
    REGISTER_COMPONENT_IMPL(Camera, "Camera")
    REGISTER_COMPONENT_IMPL(Light, "Light")
    REGISTER_COMPONENT_IMPL(BSphere, "BSphere")
    REGISTER_COMPONENT_IMPL(BBox3D, "BBox3D")
    REGISTER_COMPONENT_IMPL(Animator, "Animator")

    Transform::Transform(Inst inst) : Component(std::move(inst)) {}

    Transform::Transform(Inst inst, const glm::vec2& positionIn)
        : Component(std::move(inst)), position(glm::vec3(positionIn, 0.0f)) {}

    Transform::Transform(Inst inst, const glm::vec2& positionIn, const glm::vec2& velocityIn)
        : Component(std::move(inst)), position(glm::vec3(positionIn, 0.0f)), velocity(glm::vec3(velocityIn, 0.0f)) {}

    Transform::Transform(Inst inst, const glm::vec3& positionIn)
        : Component(std::move(inst)), position(positionIn) {}

    Transform::Transform(Inst inst, const glm::vec3& positionIn, const glm::vec3& rotationIn)
        : Component(std::move(inst)), position(positionIn), rotation(rotationIn) {}

    Transform::Transform(Inst inst, const glm::vec2& screenPos, Sprout::Pivot pivotIn, bool screenSpaceIn)
        : Component(std::move(inst)), position(glm::vec3(screenPos, 0.0f)), pivot(pivotIn), screenSpace(screenSpaceIn) {}

    glm::mat4 Transform::getModelMatrix() const
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);
        return model;
    }

    glm::vec3 Transform::forward() const
    {
        float cy = std::cos(rotation.y), sy = std::sin(rotation.y);
        float cx = std::cos(rotation.x), sx = std::sin(rotation.x);
        return glm::normalize(glm::vec3(-sy * cx, sx, -cy * cx));
    }

    glm::vec3 Transform::right() const
    {
        float cy = std::cos(rotation.y), sy = std::sin(rotation.y);
        return glm::normalize(glm::vec3(cy, 0.0f, -sy));
    }

    glm::vec3 Transform::up() const
    {
        return glm::normalize(glm::cross(right(), forward()));
    }

    void Transform::setParent(Inst newParent)
    {
        if (parent != nullptr) {
            auto& siblings = parent->getComponent<Transform>().children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), inst), siblings.end());
        }
        parent = std::move(newParent);
        if (parent != nullptr)
            parent->getComponent<Transform>().children.push_back(inst);
    }

    void Transform::removeParent()
    {
        if (parent != nullptr) {
            auto& siblings = parent->getComponent<Transform>().children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), inst), siblings.end());
            parent = nullptr;
        }
    }

    void Transform::addChild(const Inst& child)
    {
        auto& childTransform = child->getComponent<Transform>();
        if (childTransform.parent != nullptr)
            childTransform.removeParent();
        children.push_back(child);
        childTransform.parent = inst;
    }

    void Transform::removeChild(const Inst& child)
    {
        auto& childTransform = child->getComponent<Transform>();
        if (childTransform.parent == inst) {
            children.erase(std::remove(children.begin(), children.end(), child), children.end());
            childTransform.parent = nullptr;
        }
    }

    glm::mat4 Transform::getWorldMatrix() const
    {
        glm::mat4 local = getModelMatrix();
        if (parent && parent->hasComponent<Transform>())
            return parent->getComponent<Transform>().getWorldMatrix() * local;
        return local;
    }

    glm::vec3 Transform::getWorldPosition() const
    {
        if (parent && parent->hasComponent<Transform>())
            return parent->getComponent<Transform>().getWorldPosition() + position;
        return position;
    }

    void Transform::deserialize(const nlohmann::json& j)
    {
        if (j.contains("screenPosition")) {
            position = glm::vec3(jsonToVec2(j, "screenPosition", glm::vec2(0.0f)), 0.0f);
            screenSpace = true;
        }

        for (const auto& f : getFields())
            deserializeField(f, *this, j);

        postDeserialize();
    }

    nlohmann::json Transform::serialize() const
    {
        nlohmann::json j;
        j["position"] = vec3ToJson(position);
        j["velocity"] = vec3ToJson(velocity);
        j["rotation"] = vec3ToJson(rotation);
        j["scale"] = vec3ToJson(scale);
        j["pivot"] = pivotToString(pivot);
        if (screenSpace)
            j["screenSpace"] = true;
        return j;
    }

    GridTransform::GridTransform(Inst inst) : Component(std::move(inst)) {}

    GridTransform::GridTransform(Inst inst, int8_t x, int8_t y)
        : Component(std::move(inst)), x(x), y(y) {}

    glm::vec2 GridTransform::getGridPosition() { return glm::vec2(x, y); }

    glm::vec2 GridTransform::getWorldPosition()
    {
        int padding = 16;
        int worldX = x * 32 + padding;
        int worldY = y * 32 + padding;
        return glm::vec2(worldX, worldY);
    }

    void GridTransform::OnAddToEntity()
    {
        if (!inst->hasComponent<Transform>())
            inst->addComponent<Transform>(glm::vec2(0,0));
    }

    BBox::BBox(Inst inst) : Component(std::move(inst)) {}

    BBox::BBox(Inst inst, const float win, const float hin)
        : Component(std::move(inst)), w(win), h(hin) {}

    void BBox::OnAddToEntity() { inst->requestAddTag("hascollider"); }
    void BBox::OnRemoveFromEntity() { inst->requestRemoveTag("hascollider"); }

    BCircle::BCircle(Inst inst) : Component(std::move(inst)) {}

    BCircle::BCircle(Inst inst, const float radiusIn)
        : Component(std::move(inst)), radius(radiusIn) {}

    void BCircle::OnAddToEntity() { inst->requestAddTag("hascollider"); }
    void BCircle::OnRemoveFromEntity() { inst->requestRemoveTag("hascollider"); }

    Sprite::Sprite(Inst inst) : Component(std::move(inst)) {}

    Sprite::Sprite(Inst inst, const std::shared_ptr<Sprout::Texture>& texin)
        : Component(std::move(inst)), texture(texin)
    {
        glm::i32 x = texin->getWidth() / numFrames;
        glm::i32 y = texin->getHeight();
        size = glm::vec2(x, y);
    }

    Sprite::Sprite(Inst inst, const std::shared_ptr<Sprout::Texture>& texin, const float animSpeed)
        : Component(std::move(inst)), texture(texin),
          type(Type::Animated), numFrames(texin->getNumFrames()),
          animationSpeed((size_t)(60.0f / animSpeed))
    {
        glm::i32 x = texin->getWidth() / numFrames;
        glm::i32 y = texin->getHeight();
        size = glm::vec2(x, y);
    }

    void Sprite::OnAddToEntity() { inst->requestAddTag("drawable"); }
    void Sprite::OnRemoveFromEntity() { inst->requestRemoveTag("drawable"); }

    void Sprite::setColorOverride(const glm::vec4& color, const float time)
    {
        color_override = color;
        colorOverrideTime = time;
    }

    void Sprite::setAnimated(const float animSpeed)
    {
        type = Type::Animated;
        numFrames = texture->getNumFrames();
        animationSpeed = (size_t)(60.0f / animSpeed);
        glm::i32 x = texture->getWidth() / numFrames;
        glm::i32 y = texture->getHeight();
        size = glm::vec2(x, y);
    }

    void Sprite::postDeserialize()
    {
        if (texture)
        {
            glm::i32 x = texture->getWidth() / texture->getNumFrames();
            glm::i32 y = texture->getHeight();
            size = glm::vec2(x, y);
            numFrames = texture->getNumFrames();
        }
    }

    void Sprite::deserialize(const nlohmann::json& j)
    {
        for (const auto& f : getFields())
            deserializeField(f, *this, j);

        if (j.contains("animated") && j["animated"].get<bool>())
        {
            float animSpeed = j.value("animationSpeed", 6.0f);
            if (texture) setAnimated(animSpeed);
        }

        postDeserialize();
    }

    nlohmann::json Sprite::serialize() const
    {
        nlohmann::json j;
        for (const auto& f : getFields())
        {
            auto val = serializeField(f, *this);
            if (!val.is_null())
                j[f.jsonKey] = std::move(val);
        }
        if (type == Type::Animated)
        {
            j["animated"] = true;
            j["animationSpeed"] = (animationSpeed > 0) ? 60.0f / static_cast<float>(animationSpeed) : 6.0f;
        }
        return j;
    }



    Text::Text(Inst inst) : Component(std::move(inst)) {}

    Text::Text(Inst inst, const std::string& text, const std::string& font, uint8_t size, glm::vec4 color, Sprout::TextJustify justify)
        : Component(std::move(inst)), text(text), font(font), size(size), color(color), justify(justify) {}

    MeshRenderer::MeshRenderer(Inst inst) : Component(std::move(inst)) {}

    MeshRenderer::MeshRenderer(Inst inst, const std::shared_ptr<Sprout::Mesh>& meshIn,
                               const std::shared_ptr<Sprout::Material>& materialIn)
        : Component(std::move(inst)), mesh(meshIn), material(materialIn) {}

    void MeshRenderer::OnAddToEntity() { inst->requestAddTag("drawable3d"); }
    void MeshRenderer::OnRemoveFromEntity() { inst->requestRemoveTag("drawable3d"); }

    Camera::Camera(Inst inst) : Component(std::move(inst)) {}

    Camera::Camera(Inst inst, float fovDegrees, float nearP, float farP)
        : Component(std::move(inst)), fov(fovDegrees), nearPlane(nearP), farPlane(farP) {}

    glm::mat4 Camera::getViewMatrix() const
    {
        if (!inst->hasComponent<Transform>()) return glm::mat4(1.0f);
        auto& t = inst->getComponent<Transform>();
        glm::vec3 target = t.position + t.forward();
        return glm::lookAt(t.position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void Camera::deserialize(const nlohmann::json& j)
    {
        for (const auto& f : getFields())
            deserializeField(f, *this, j);
        if (j.contains("projectionType"))
        {
            std::string projStr = j["projectionType"].get<std::string>();
            projectionType = (projStr == "Orthographic") ? Projection::Orthographic : Projection::Perspective;
        }
    }

    nlohmann::json Camera::serialize() const
    {
        nlohmann::json j;
        for (const auto& f : getFields())
        {
            auto val = serializeField(f, *this);
            if (!val.is_null())
                j[f.jsonKey] = std::move(val);
        }
        j["projectionType"] = (projectionType == Projection::Orthographic) ? "Orthographic" : "Perspective";
        return j;
    }

    glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const
    {
        if (projectionType == Projection::Perspective)
        {
            return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
        }
        else
        {
            float halfW = orthoSize * aspectRatio;
            float halfH = orthoSize;
            return glm::ortho(-halfW, halfW, -halfH, halfH, nearPlane, farPlane);
        }
    }


    glm::vec3 Camera::worldToScreen(const glm::vec3& worldPos, float screenW, float screenH) const
    {
        float aspect = screenW / screenH;
        glm::mat4 view = getViewMatrix();
        glm::mat4 proj = getProjectionMatrix(aspect);
        glm::vec4 clip = proj * view * glm::vec4(worldPos, 1.0f);
        if (clip.w == 0.0f) return glm::vec3(0.0f);
        glm::vec3 ndc = glm::vec3(clip) / clip.w;
        return glm::vec3(
            (ndc.x * 0.5f + 0.5f) * screenW,
            (1.0f - (ndc.y * 0.5f + 0.5f)) * screenH, // flip Y: screen Y goes down
            ndc.z * 0.5f + 0.5f
        );
    }

    glm::vec3 Camera::screenToWorld(const glm::vec2& screenPos, float depth, float screenW, float screenH) const
    {
        float aspect = screenW / screenH;
        glm::mat4 view = getViewMatrix();
        glm::mat4 proj = getProjectionMatrix(aspect);
        glm::mat4 invVP = glm::inverse(proj * view);
        // screen to NDC
        float ndcX = (screenPos.x / screenW) * 2.0f - 1.0f;
        float ndcY = 1.0f - (screenPos.y / screenH) * 2.0f; // flip Y
        float ndcZ = depth * 2.0f - 1.0f;
        glm::vec4 clip = glm::vec4(ndcX, ndcY, ndcZ, 1.0f);
        glm::vec4 world = invVP * clip;
        if (world.w == 0.0f) return glm::vec3(0.0f);
        return glm::vec3(world) / world.w;
    }

    Light::Light(Inst inst) : Component(std::move(inst)) {}

    Light::Light(Inst inst, Type typeIn, const glm::vec3& colorIn, float intensityIn)
        : Component(std::move(inst)), type(typeIn), color(colorIn), intensity(intensityIn) {}

    void Light::deserialize(const nlohmann::json& j)
    {
        for (const auto& f : getFields())
            deserializeField(f, *this, j);
        if (j.contains("type"))
        {
            std::string typeStr = j["type"].get<std::string>();
            type = (typeStr == "Point") ? Type::Point : Type::Directional;
        }
    }

    nlohmann::json Light::serialize() const
    {
        nlohmann::json j;
        for (const auto& f : getFields())
        {
            auto val = serializeField(f, *this);
            if (!val.is_null())
                j[f.jsonKey] = std::move(val);
        }
        j["type"] = (type == Type::Point) ? "Point" : "Directional";
        return j;
    }


    BSphere::BSphere(Inst inst) : Component(std::move(inst)) {}

    BSphere::BSphere(Inst inst, float radiusIn)
        : Component(std::move(inst)), radius(radiusIn) {}

    void BSphere::OnAddToEntity() { inst->requestAddTag("hascollider"); }
    void BSphere::OnRemoveFromEntity() { inst->requestRemoveTag("hascollider"); }


    BBox3D::BBox3D(Inst inst) : Component(std::move(inst)) {}

    BBox3D::BBox3D(Inst inst, const glm::vec3& halfExtentsIn)
        : Component(std::move(inst)), halfExtents(halfExtentsIn) {}

    void BBox3D::OnAddToEntity() { inst->requestAddTag("hascollider"); }
    void BBox3D::OnRemoveFromEntity() { inst->requestRemoveTag("hascollider"); }


    Animator::Animator(Inst inst) : Component(std::move(inst)) {}

    Animator::Animator(Inst inst, const std::shared_ptr<Sprout::Skeleton>& skelIn)
        : Component(std::move(inst)), skeleton(skelIn)
    {
        if (skeleton) boneMatrices.resize(skeleton->bones.size(), glm::mat4(1.0f));
    }

    void Animator::deserialize(const nlohmann::json& j)
    {
        auto skelRef = parseAssetRef(j, "skeleton");
        if (!skelRef.name.empty())
        {
            try
            {
                if (!skelRef.path.empty())
                    skeleton = AssetManager::ensureSkeleton(skelRef.name, skelRef.path);
                else
                    skeleton = AssetManager::getSkeleton(skelRef.name);
                if (skeleton)
                    boneMatrices.resize(skeleton->bones.size(), glm::mat4(1.0f));
            }
            catch (const std::exception&)
            {
                Logger::warn("Animator: skeleton not found: " + skelRef.name);
            }
        }
        playbackSpeed = j.value("playbackSpeed", playbackSpeed);
        auto clipRef = parseAssetRef(j, "clip");
        if (!clipRef.name.empty())
        {
            try
            {
                std::shared_ptr<Sprout::AnimationClip> clip;
                if (!clipRef.path.empty())
                {
                    std::string skelName = clipRef.skeleton.empty() ? skelRef.name : clipRef.skeleton;
                    clip = AssetManager::ensureAnimationClip(clipRef.name, clipRef.path, skelName, clipRef.clipName);
                }
                else
                {
                    clip = AssetManager::getAnimationClip(clipRef.name);
                }
                bool loop = j.value("looping", true);
                play(clip, loop);
            }
            catch (const std::exception&)
            {
                Logger::warn("Animator: clip not found: " + clipRef.name);
            }
        }
    }

    nlohmann::json Animator::serialize() const
    {
        nlohmann::json j;
        if (skeleton)
        {
            std::string name = AssetManager::getSkeletonName(skeleton);
            if (!name.empty())
            {
                std::string path = AssetManager::getSkeletonPath(name);
                j["skeleton"] = serializeAssetRef(name, path);
            }
        }
        if (currentClip)
        {
            std::string name = AssetManager::getAnimationClipName(currentClip);
            if (!name.empty())
            {
                std::string path = AssetManager::getAnimationClipPath(name);
                j["clip"] = serializeAssetRef(name, path);
            }
        }
        j["looping"] = looping;
        j["playbackSpeed"] = playbackSpeed;
        return j;
    }

    void Animator::play(const std::shared_ptr<Sprout::AnimationClip>& clip, bool loop, float fadeDuration)

    {
        if (fadeDuration > 0.0f && currentClip && isPlaying)

        {
            prevClip = currentClip;
            prevTime = currentTime;
            prevLooping = looping;
            blendDuration = fadeDuration;
            blendElapsed = 0.0f;
        }

        else

        {
            prevClip = nullptr;
            blendDuration = 0.0f;
            blendElapsed = 0.0f;
        }
        currentClip = clip;
        currentTime = 0.0f;
        isPlaying = true;
        looping = loop;
    }

    void Animator::stop() { isPlaying = false; }

    auto Animator::isBlending() const -> bool
    {
        return prevClip != nullptr && blendElapsed < blendDuration;
    }

    void Animator::update(float deltaTime)
    {
        if (!isPlaying || !currentClip || !skeleton) return;

        float dt = deltaTime * playbackSpeed;
        currentTime += dt;
        if (currentTime >= currentClip->duration)
        {
            if (looping)
            {
                currentTime = std::fmod(currentTime, currentClip->duration);
            }
            else
            {
                currentTime = currentClip->duration;
                isPlaying = false;
            }
        }

        if (isBlending())
        {
            prevTime += dt;
            if (prevTime >= prevClip->duration)
            {
                if (prevLooping)
                    prevTime = std::fmod(prevTime, prevClip->duration);
                else
                    prevTime = prevClip->duration;
            }
            blendElapsed += deltaTime;
            float t = std::min(blendElapsed / blendDuration, 1.0f);

            std::vector<glm::mat4> prevMatrices(skeleton->bones.size(), glm::mat4(1.0f));
            prevClip->sample(prevTime, prevMatrices, *skeleton);
            currentClip->sample(currentTime, boneMatrices, *skeleton);

            for (size_t i = 0; i < boneMatrices.size(); i++)
            {
                for (int col = 0; col < 4; col++)
                    boneMatrices[i][col] = glm::mix(prevMatrices[i][col], boneMatrices[i][col], t);
            }
            if (blendElapsed >= blendDuration) prevClip = nullptr;
        }
        else
        {
            currentClip->sample(currentTime, boneMatrices, *skeleton);
        }
    }

    void Animator::OnAddToEntity() { inst->requestAddTag("animated"); }
    void Animator::OnRemoveFromEntity() { inst->requestRemoveTag("animated"); }

} // namespace Comp
