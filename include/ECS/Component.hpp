//
//  Component.hpp
//  SaplingEngine
//

#pragma once



#include "Renderer/StandaloneTexture.hpp"
#include "Utility/Color.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/Sprout.hpp"

#include "glm/glm.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_set>
#include <utility>
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
    
    
    /*
        * A transform component for position and velocity.
        * position (vec2): The position of the entity.
        * velocity (vec2): The velocity of the entity.
        * rotation (f32): The rotation of the entity.
    */
    struct Transform final : public Component
    {
        glm::vec2 position = glm::vec2(0, 0);
        glm::vec2 velocity = glm::vec2(0, 0);
        glm::f32 rotation = 0.0f;
        glm::vec3 scale = glm::vec3(1, 1, 1);
        Sprout::Pivot pivot = Sprout::Pivot::TOP_LEFT;
        bool reset = false;

        Transform(Inst inst, const glm::vec2& positionin, const glm::vec2& velocityin);
        Transform(Inst inst, const glm::vec2& positionin);
    };
    
    struct GridTransform final : public Component
    {
        GridTransform(Inst inst, int8_t x, int8_t y);
        
        int8_t x;
        int8_t y;
        
        glm::vec2 getGridPosition();
        glm::vec2 getWorldPosition();

        void OnAddToEntity() override;
    };
    
    /*
        * A bounding box component for collision detection.
        * w (f32): The width of the bounding box.
        * h (f32): The height of the bounding box.
    */
    struct BBox final : public Component
    {
        float h;
        float w;
        bool isTrigger = false;
        bool isStatic = true;
        bool interactWithTriggers = false;
        bool collisionEventsEnabled = false;
        std::unordered_set<BBox*> collidingWith = {};
        
        BBox(Inst inst, float win, float hin);
        void OnAddToEntity() override;
        void OnRemoveFromEntity() override;
    };
    
    /*
        * A bounding circle component for collision detection.
        * radius (f32): The radius of the bounding circle.
    */
    struct BCircle final : public Component
    {
        float radius = 1.0f;
        
        BCircle(Inst inst, float radiusIn);
        void OnAddToEntity() override;
        void OnRemoveFromEntity() override;
    };
        
    
    /*
        * A sprite component for rendering textures.
        * Supports static and animated sprites as well as layering.
        * texture (shared_ptr<Texture>): The texture to render.
        * type (Type): The type of sprite (static or animated).
        * layer (Layer): The layer to render the sprite on.
        * numFrames (size_t): The number of frames in the animation.
        * currentFrame (size_t): The current frame of the animation.
        * frameSize (size_t): The size of each frame in the animation.
        * animationSpeed (size_t): The speed of the animation.
    */
    struct Sprite final : public Component 
    {
        std::shared_ptr<Sprout::Texture> texture;
        glm::vec2 size;
        explicit Sprite(Inst inst, const std::shared_ptr<Sprout::Texture>& texin);
        explicit Sprite(Inst inst, const std::shared_ptr<Sprout::Texture>& texin, float animSpeed);
        void OnAddToEntity() override;
        void OnRemoveFromEntity() override;
        
        glm::vec2 transformOffset = glm::vec2(0.0f, 0.0f);
        glm::vec3 scaleOffset = glm::vec3(1.0f, 1.0f, 1.0f);
        
        void setLayer(Layer layerIn) { layer = layerIn; }
        void flipX(bool flip) { flip_X = flip; }
        void setColorOverride(const glm::vec4& color, float time);
        void setAnimated(const float animSpeed);
        
        enum class Type : std::uint8_t 
        {
            Static,
            Animated
        };
        
        Type type = Type::Static;
        Layer layer = Layer::Midground;
        
        // animated frame stuff
        size_t numFrames = 1;
        size_t currentFrame = 0;
        // size_t frameSize;
        size_t animationSpeed = 60;
        float animationTime = 0.0f;
        glm::vec4 color_override = Color::Transparent;
        float colorOverrideTime = 0;
        bool flip_X = false;
    };
    
    struct Image final : public Component
    {
        std::shared_ptr<Sprout::StandaloneTexture> texture;
        glm::vec2 size;
        glm::vec2 transformOffset = glm::vec2(0.0f, 0.0f);
        glm::vec3 scaleOffset = glm::vec3(1.0f, 1.0f, 1.0f);
        
        
        explicit Image(Inst inst, const std::shared_ptr<Sprout::StandaloneTexture>& texin);
        
        void OnAddToEntity() override;
        void OnRemoveFromEntity() override;
        void setLayer(Layer newLayer) { layer = newLayer; }
        
        Layer layer = Layer::Midground;
    };

    
    struct TransformHierarchy final : public Component
    {
        Inst parent = nullptr;
        std::vector<Inst> children = {};
        
        TransformHierarchy(Inst inst);
        
        void setParent(Inst parentIn);
        void removeParent();
        
        void addChild(const Inst& child);
        void removeChild(const Inst& child);
    };
    
    struct GUITransform final : public Component
    {
        GUITransform(Inst inst, const glm::vec2& screenPos, Sprout::Pivot pivotIn);
        // void OnAddToEntity() override;
        // void OnRemoveFromEntity() override;
        
        glm::vec2 screenPosition = glm::vec2(0, 0);
        glm::vec3 scale = glm::vec3(1, 1, 1);
        Sprout::Pivot pivot = Sprout::Pivot::CENTER;
    };
    
    struct Text final : public Component
    {
        Text(Inst inst, const std::string& text, const std::string& font, uint8_t size, glm::vec4 color, Sprout::TextJustify justify = Sprout::TextJustify::LEFT);
        
        std::string text = "";
        std::string font = "";
        uint8_t size = 1;
        glm::vec4 color = Color::Black;
        glm::vec2 transformOffset = glm::vec2(0, 0);
        Sprout::TextJustify justify = Sprout::TextJustify::LEFT;
        Layer layer = Layer::UserInterface;
    };
}

