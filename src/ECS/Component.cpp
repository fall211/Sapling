//
//  Component.cpp
//  SaplingEngine
//

#include "ECS/Component.hpp"
#include "ECS/Entity.hpp"
#include "Renderer/Texture.hpp"

#include <algorithm>
#include <utility>

namespace Comp
{
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
        updateSizeFromTexture();
    }

    Sprite::Sprite(Inst inst, const std::shared_ptr<Sprout::Texture>& texin, const float animSpeed)
        : Component(std::move(inst)), texture(texin),
          type(Type::Animated), numFrames(texin->getNumFrames()),
          animationSpeed((size_t)(60.0f / animSpeed))
    {
        updateSizeFromTexture();
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
        updateSizeFromTexture();
    }

    void Sprite::updateSizeFromTexture()
    {
        if (!texture) return;

        glm::i32 x = texture->getWidth() / texture->getNumFrames();
        glm::i32 y = texture->getHeight();
        size = glm::vec2(x, y);
        numFrames = texture->getNumFrames();
    }

    Text::Text(Inst inst) : Component(std::move(inst)) {}

    Text::Text(Inst inst, const std::string& text, const std::string& font, uint8_t size, glm::vec4 color, Sprout::TextJustify justify)
        : Component(std::move(inst)), text(text), font(font), size(size), color(color), justify(justify) {}

} // namespace Comp
