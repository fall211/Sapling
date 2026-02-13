//
//  Component.hpp
//  SaplingEngine
//

#pragma once

#include "ECS/ComponentReflection.hpp"
#include "Utility/Color.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>

#include <nlohmann/json_fwd.hpp>


class Entity;

namespace Comp
{
    typedef std::shared_ptr<Entity> Inst;

    class Component{
    protected:
        Inst inst;

    public:
        bool has = false;
        bool enabled = true;
        Component(Inst inst) : inst(std::move(inst)) {};
        ~Component()= default;
        Inst getInst() const { return inst; }

        virtual const char* componentName() const { return "Component"; }
        virtual std::span<const FieldInfo> getFields() const { return {}; }

        virtual void OnAddToEntity() {}
        virtual void OnRemoveFromEntity() {}
        virtual void postDeserialize() {}

        virtual void deserialize(const nlohmann::json& j);
        virtual nlohmann::json serialize() const;

#ifdef SAPLING_HAS_EDITOR
        virtual bool inspect();
#endif
    };

    enum class Layer : std::uint8_t
    {
        Background,
        Midground,
        Player,
        Foreground,
        UserInterface,
        Count
    };


    struct Transform final : public Component
    {
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 velocity = glm::vec3(0.0f);
        glm::vec3 rotation = glm::vec3(0.0f);  // euler angles (radians); 2D uses .z only
        glm::vec3 scale = glm::vec3(1.0f);
        Sprout::Pivot pivot = Sprout::Pivot::TOP_LEFT;
        bool screenSpace = false;

        Inst parent = nullptr;
        std::vector<Inst> children;

        Transform(Inst inst);
        Transform(Inst inst, const glm::vec2& positionIn);
        Transform(Inst inst, const glm::vec2& positionIn, const glm::vec2& velocityIn);
        Transform(Inst inst, const glm::vec3& positionIn);
        Transform(Inst inst, const glm::vec3& positionIn, const glm::vec3& rotationIn);
        Transform(Inst inst, const glm::vec2& screenPos, Sprout::Pivot pivotIn, bool screenSpace);

        glm::mat4 getModelMatrix() const;
        glm::vec3 forward() const;
        glm::vec3 right() const;
        glm::vec3 up() const;

        void setParent(Inst parentIn);
        void removeParent();
        void addChild(const Inst& child);
        void removeChild(const Inst& child);

        glm::mat4 getWorldMatrix() const;
        glm::vec3 getWorldPosition() const;

        void deserialize(const nlohmann::json& j) override;
        nlohmann::json serialize() const override;

#ifdef SAPLING_HAS_EDITOR
        bool inspect() override;
#endif

        REGISTER_COMPONENT(Transform, "Transform")
        SAPLING_FIELDS(Transform,
            FIELD(position, "position", "Position"),
            FIELD(velocity, "velocity", "Velocity"),
            FIELD(rotation, "rotation", "Rotation"),
            FIELD(scale, "scale", "Scale"),
            FIELD(pivot, "pivot", "Pivot"),
            FIELD(screenSpace, "screenSpace", "Screen Space"),
        )
    };

    struct GridTransform final : public Component
    {
        int8_t x = 0;
        int8_t y = 0;

        GridTransform(Inst inst);
        GridTransform(Inst inst, int8_t x, int8_t y);

        glm::vec2 getGridPosition();
        glm::vec2 getWorldPosition();
        void OnAddToEntity() override;

        REGISTER_COMPONENT(GridTransform, "GridTransform")
        SAPLING_FIELDS(GridTransform,
            FIELD(x, "x", "X"),
            FIELD(y, "y", "Y"),
        )
    };

    struct BBox final : public Component
    {
        float w = 0.0f;
        float h = 0.0f;
        bool isTrigger = false;
        bool isStatic = true;
        bool interactWithTriggers = false;
        bool collisionEventsEnabled = false;
        std::unordered_set<BBox*> collidingWith = {};

        BBox(Inst inst);
        BBox(Inst inst, float win, float hin);
        void OnAddToEntity() override;
        void OnRemoveFromEntity() override;

        REGISTER_COMPONENT(BBox, "BBox")
        SAPLING_FIELDS(BBox,
            FIELD(w, "width", "Width"),
            FIELD(h, "height", "Height"),
            FIELD(isTrigger, "isTrigger", "Is Trigger"),
            FIELD(isStatic, "isStatic", "Is Static"),
            FIELD(interactWithTriggers, "interactWithTriggers", "Interact w/ Triggers"),
            FIELD(collisionEventsEnabled, "collisionEventsEnabled", "Collision Events"),
        )
    };

    struct BCircle final : public Component
    {
        float radius = 1.0f;

        BCircle(Inst inst);
        BCircle(Inst inst, float radiusIn);
        void OnAddToEntity() override;
        void OnRemoveFromEntity() override;

        REGISTER_COMPONENT(BCircle, "BCircle")
        SAPLING_FIELDS(BCircle,
            FIELD(radius, "radius", "Radius"),
        )
    };


    struct Sprite final : public Component
    {
        std::shared_ptr<Sprout::Texture> texture;
        glm::vec2 size;
        glm::vec2 transformOffset = glm::vec2(0.0f, 0.0f);
        glm::vec3 scaleOffset = glm::vec3(1.0f, 1.0f, 1.0f);

        enum class Type : std::uint8_t
        {
            Static,
            Animated
        };

        Type type = Type::Static;
        Layer layer = Layer::Midground;

        size_t numFrames = 1;
        size_t currentFrame = 0;
        size_t animationSpeed = 60;
        float animationTime = 0.0f;
        glm::vec4 color_override = Color::Transparent;
        float colorOverrideTime = 0;
        bool flip_X = false;

        explicit Sprite(Inst inst);
        explicit Sprite(Inst inst, const std::shared_ptr<Sprout::Texture>& texin);
        explicit Sprite(Inst inst, const std::shared_ptr<Sprout::Texture>& texin, float animSpeed);
        void OnAddToEntity() override;
        void OnRemoveFromEntity() override;

        void setLayer(Layer layerIn) { layer = layerIn; }
        void flipX(bool flip) { flip_X = flip; }
        void setColorOverride(const glm::vec4& color, float time);
        void setAnimated(const float animSpeed);

        void postDeserialize() override;

        void deserialize(const nlohmann::json& j) override;
        nlohmann::json serialize() const override;
#ifdef SAPLING_HAS_EDITOR
        bool inspect() override;
#endif

        REGISTER_COMPONENT(Sprite, "Sprite")
        SAPLING_FIELDS(Sprite,
            FIELD_AS(texture, "texture", "Texture", FieldType::TextureRef),
            FIELD(layer, "layer", "Layer"),
            FIELD(flip_X, "flipX", "Flip X"),
            FIELD(transformOffset, "transformOffset", "Offset"),
            FIELD(scaleOffset, "scaleOffset", "Scale Offset"),
        )
    };

    struct Image final : public Component
    {
        std::shared_ptr<Sprout::Texture> texture;
        glm::vec2 size;
        glm::vec2 transformOffset = glm::vec2(0.0f, 0.0f);
        glm::vec3 scaleOffset = glm::vec3(1.0f, 1.0f, 1.0f);
        Layer layer = Layer::Midground;

        explicit Image(Inst inst);
        explicit Image(Inst inst, const std::shared_ptr<Sprout::Texture>& texin);
        void OnAddToEntity() override;
        void OnRemoveFromEntity() override;
        void setLayer(Layer newLayer) { layer = newLayer; }

        void postDeserialize() override;
#ifdef SAPLING_HAS_EDITOR
        bool inspect() override;
#endif

        REGISTER_COMPONENT(Image, "Image")
        SAPLING_FIELDS(Image,
            FIELD_AS(texture, "texture", "Texture", FieldType::ImageTextureRef),
            FIELD(layer, "layer", "Layer"),
            FIELD(transformOffset, "transformOffset", "Offset"),
            FIELD(scaleOffset, "scaleOffset", "Scale Offset"),
        )
    };




    struct Text final : public Component
    {
        std::string text = "";
        std::string font = "";
        uint8_t size = 1;
        glm::vec4 color = Color::Black;
        glm::vec2 transformOffset = glm::vec2(0, 0);
        Sprout::TextJustify justify = Sprout::TextJustify::LEFT;
        Layer layer = Layer::UserInterface;

        Text(Inst inst);
        Text(Inst inst, const std::string& text, const std::string& font, uint8_t size, glm::vec4 color, Sprout::TextJustify justify = Sprout::TextJustify::LEFT);

#ifdef SAPLING_HAS_EDITOR
        bool inspect() override;
#endif

        REGISTER_COMPONENT(Text, "Text")
        SAPLING_FIELDS(Text,
            FIELD(text, "text", "Text"),
            FIELD(font, "font", "Font"),
            FIELD(size, "size", "Size"),
            FIELD_AS(color, "color", "Color", FieldType::Color4),
            FIELD(transformOffset, "transformOffset", "Offset"),
            FIELD(justify, "justify", "Justify"),
            FIELD(layer, "layer", "Layer"),
        )
    };



    struct MeshRenderer final : public Component
    {
        std::shared_ptr<Sprout::Mesh> mesh;
        std::shared_ptr<Sprout::Material> material;
        bool castShadow = true;

        MeshRenderer(Inst inst);
        MeshRenderer(Inst inst, const std::shared_ptr<Sprout::Mesh>& meshIn,
                     const std::shared_ptr<Sprout::Material>& materialIn);
        void OnAddToEntity() override;
        void OnRemoveFromEntity() override;
#ifdef SAPLING_HAS_EDITOR
        bool inspect() override;
#endif

        REGISTER_COMPONENT(MeshRenderer, "MeshRenderer")
        SAPLING_FIELDS(MeshRenderer,
            FIELD(mesh, "mesh", "Mesh"),
            FIELD(material, "material", "Material"),
            FIELD(castShadow, "castShadow", "Cast Shadow"),
        )
    };

    struct Camera final : public Component
    {
        enum class Projection { Perspective, Orthographic };
        Projection projectionType = Projection::Perspective;
        float fov = 60.0f;
        float nearPlane = 0.1f;
        float farPlane = 1000.0f;
        float orthoSize = 10.0f;
        bool isActive = false;

        Camera(Inst inst);
        Camera(Inst inst, float fovDegrees, float nearP, float farP);

        glm::mat4 getViewMatrix() const;
        glm::mat4 getProjectionMatrix(float aspectRatio) const;

        glm::vec3 worldToScreen(const glm::vec3& worldPos, float screenW, float screenH) const;
        glm::vec3 screenToWorld(const glm::vec2& screenPos, float depth, float screenW, float screenH) const;

        void deserialize(const nlohmann::json& j) override;
        nlohmann::json serialize() const override;
#ifdef SAPLING_HAS_EDITOR
        bool inspect() override;
#endif

        REGISTER_COMPONENT(Camera, "Camera")
        SAPLING_FIELDS(Camera,
            FIELD(fov, "fov", "FOV"),
            FIELD(nearPlane, "nearPlane", "Near"),
            FIELD(farPlane, "farPlane", "Far"),
            FIELD(orthoSize, "orthoSize", "Ortho Size"),
            FIELD(isActive, "isActive", "Active"),
        )
    };

    struct Light final : public Component
    {
        enum class Type { Directional, Point };
        Type type = Type::Directional;
        glm::vec3 color = glm::vec3(1.0f);
        float intensity = 1.0f;
        float range = 10.0f;

        Light(Inst inst);
        Light(Inst inst, Type typeIn, const glm::vec3& colorIn, float intensityIn);

        void deserialize(const nlohmann::json& j) override;
        nlohmann::json serialize() const override;
#ifdef SAPLING_HAS_EDITOR
        bool inspect() override;
#endif

        REGISTER_COMPONENT(Light, "Light")
        SAPLING_FIELDS(Light,
            FIELD_AS(color, "color", "Color", FieldType::Color3),
            FIELD(intensity, "intensity", "Intensity"),
            FIELD(range, "range", "Range"),
        )
    };

    struct BSphere final : public Component
    {
        float radius = 1.0f;

        BSphere(Inst inst);
        BSphere(Inst inst, float radiusIn);
        void OnAddToEntity() override;
        void OnRemoveFromEntity() override;

        REGISTER_COMPONENT(BSphere, "BSphere")
        SAPLING_FIELDS(BSphere,
            FIELD(radius, "radius", "Radius"),
        )
    };

    struct BBox3D final : public Component
    {
        glm::vec3 halfExtents = glm::vec3(0.5f);

        BBox3D(Inst inst);
        BBox3D(Inst inst, const glm::vec3& halfExtentsIn);
        void OnAddToEntity() override;
        void OnRemoveFromEntity() override;

        REGISTER_COMPONENT(BBox3D, "BBox3D")
        SAPLING_FIELDS(BBox3D,
            FIELD(halfExtents, "halfExtents", "Half Extents"),
        )
    };

    struct Animator final : public Component
    {
        std::shared_ptr<Sprout::Skeleton> skeleton;
        std::shared_ptr<Sprout::AnimationClip> currentClip;
        float currentTime = 0.0f;
        bool isPlaying = true;
        bool looping = true;
        float playbackSpeed = 1.0f;

        std::shared_ptr<Sprout::AnimationClip> prevClip;
        float prevTime = 0.0f;
        bool prevLooping = true;
        float blendDuration = 0.0f;
        float blendElapsed = 0.0f;

        std::vector<glm::mat4> boneMatrices;

        Animator(Inst inst);
        Animator(Inst inst, const std::shared_ptr<Sprout::Skeleton>& skelIn);

        void play(const std::shared_ptr<Sprout::AnimationClip>& clip, bool loop = true, float fadeDuration = 0.0f);
        void stop();
        void update(float deltaTime);
        auto isBlending() const -> bool;

        void OnAddToEntity() override;
        void OnRemoveFromEntity() override;

        void deserialize(const nlohmann::json& j) override;
        nlohmann::json serialize() const override;
#ifdef SAPLING_HAS_EDITOR
        bool inspect() override;
#endif

        REGISTER_COMPONENT(Animator, "Animator")
        SAPLING_FIELDS(Animator,
            FIELD(skeleton, "skeleton", "Skeleton"),
            FIELD(playbackSpeed, "playbackSpeed", "Speed"),
            FIELD(looping, "looping", "Looping"),
        )
    };
}
