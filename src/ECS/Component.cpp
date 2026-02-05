//
//  Component.cpp
//  SaplingEngine
//

#include "ECS/Component.hpp"
#include "ECS/Entity.hpp"
#include "Renderer/StandaloneTexture.hpp"

#include <utility>

namespace Comp
{
    
    Transform::Transform(Inst inst, const glm::vec2& positionin, const glm::vec2& velocityin)
        :   Component(std::move(inst)),
            position(positionin), 
            velocity(velocityin) 
        {}
    
    Transform::Transform(Inst inst, const glm::vec2& positionin)
        :   Component(std::move(inst)),
            position(positionin) 
        {}
    
    GridTransform::GridTransform(Inst inst, int8_t x, int8_t y) 
    :   Component(std::move(inst)), x(x), y(y)
    {}
    
    glm::vec2 GridTransform::getGridPosition()
    {
        return glm::vec2(x, y);
    }
    
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
        {
            inst->addComponent<Transform>(glm::vec2(0,0));
        }
    }
        
    BBox::BBox(Inst inst, const float win, const float hin) 
    :   Component(std::move(inst)),
        h(hin), 
        w(win) 
    {}
    
    void BBox::OnAddToEntity()
    {
        inst->requestAddTag("hascollider");
    }
    void BBox::OnRemoveFromEntity()
    {
        inst->requestRemoveTag("hascollider");
    }
        
    BCircle::BCircle(Inst inst, const float radiusIn) 
    :   Component(std::move(inst)),
        radius(radiusIn) 
    {}
    
    void BCircle::OnAddToEntity()
    {
        inst->requestAddTag("hascollider");
    }
    
    void BCircle::OnRemoveFromEntity()
    {
        inst->requestRemoveTag("hascollider");
    }
    
    Sprite::Sprite(Inst inst, const std::shared_ptr<Sprout::Texture>& texin)
        :   Component(std::move(inst)),
            texture(texin) 
        {
            glm::i32 x = texin->getWidth() / numFrames;
            glm::i32 y = texin->getHeight();
            size = glm::vec2(x, y);
        }
    
    Sprite::Sprite(Inst inst, const std::shared_ptr<Sprout::Texture>& texin, const float animSpeed) 
        :   Component(std::move(inst)),
            texture(texin), 
            type(Type::Animated), 
            numFrames(texin->getNumFrames()), 
            animationSpeed((size_t)(60.0f / animSpeed)) 
        {
            glm::i32 x = texin->getWidth() / numFrames;
            glm::i32 y = texin->getHeight();
            size = glm::vec2(x, y);
        }
        
    void Sprite::OnAddToEntity()
    {
        inst->requestAddTag("drawable");
    }
    
    void Sprite::OnRemoveFromEntity()
    {
        inst->requestRemoveTag("drawable");
    }
    
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
    
    Image::Image(Inst inst, const std::shared_ptr<Sprout::StandaloneTexture>& texin)
        :   Component(std::move(inst)),
            texture(texin)
        {
            glm::i32 x = texin->getWidth();
            glm::i32 y = texin->getHeight();
            size = glm::vec2(x, y);
        }

    void Image::OnAddToEntity()
    {
        inst->requestAddTag("drawable");
    }
    
    void Image::OnRemoveFromEntity()
    {
        inst->requestRemoveTag("drawable");
    }
        
    TransformHierarchy::TransformHierarchy(Inst inst)
        :   Component(std::move(inst))
        {}
        
    void TransformHierarchy::setParent(Inst newParent)
    {
        if (parent != nullptr)
        {
            auto& siblings = parent->getComponent<TransformHierarchy>().children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), inst), siblings.end());
        }
    
        parent = std::move(newParent);
    
        if (parent != nullptr)
        {
            parent->getComponent<TransformHierarchy>().children.push_back(inst);
        }    }
    
    void TransformHierarchy::removeParent()
    {
        if (parent != nullptr)
        {
            auto& siblings = parent->getComponent<TransformHierarchy>().children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), inst), siblings.end());
            parent = nullptr;
        }    }
    
    void TransformHierarchy::addChild(const Inst& child)
    {
        auto& childHierarchy = child->getComponent<TransformHierarchy>();
        if (childHierarchy.parent != nullptr)
        {
            childHierarchy.removeParent();
        }
    
        children.push_back(child);
        childHierarchy.setParent(inst);    }
    
    void TransformHierarchy::removeChild(const Inst& child)
    {
        auto& childHierarchy = child->getComponent<TransformHierarchy>();
        if (childHierarchy.parent == inst)
        {
            children.erase(std::remove(children.begin(), children.end(), child), children.end());
            childHierarchy.removeParent();
        }
    }
    
    GUITransform::GUITransform(Inst inst, const glm::vec2& screenPos, Sprout::Pivot pivotIn)
        :   Component(std::move(inst)),
            screenPosition(screenPos),
            pivot(pivotIn)
        {}
        
    Text::Text(Inst inst, const std::string& text, const std::string& font, uint8_t size, glm::vec4 color, Sprout::TextJustify justify)
            :   Component(std::move(inst)),
                text(text),
                font(font),
                size(size),
                color(color),
                justify(justify)
            {}
}