//
//  Component.hpp
//  SaplingEngine
//

#pragma once

#include "Utility/Color.hpp"
#include "Renderer/Sprout.hpp"
#include "Renderer/Texture.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <unordered_set>
#include <vector>


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

        virtual void OnAddToEntity() {}
        virtual void OnRemoveFromEntity() {}
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

        void setParent(Inst parentIn);
        void removeParent();
        void addChild(const Inst& child);
        void removeChild(const Inst& child);

        glm::mat4 getWorldMatrix() const;
        glm::vec3 getWorldPosition() const;

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

    };

    struct BCircle final : public Component
    {
        float radius = 1.0f;

        BCircle(Inst inst);
        BCircle(Inst inst, float radiusIn);
        void OnAddToEntity() override;
        void OnRemoveFromEntity() override;

    };


    struct Sprite final : public Component
    {
        std::shared_ptr<Sprout::Texture> texture;
        glm::vec2 size;
        glm::vec2 transformOffset = glm::vec2(0.0f, 0.0f);
        glm::vec3 scaleOffset = glm::vec3(1.0f, 1.0f, 1.0f);
        float pixelsPerUnit = 0.0f;

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

        void updateSizeFromTexture();
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

    };



}
