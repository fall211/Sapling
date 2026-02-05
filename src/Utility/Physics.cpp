//
//  Physics.cpp
//  SaplingEngine, Twig Physics
//

#include "Utility/Physics.hpp"


auto Physics2D::bBoxCollision(const std::shared_ptr<Entity>& e0, const std::shared_ptr<Entity>& e1) -> glm::vec2
{
    if (e0->getId() == e1->getId()) return {0, 0};

    const auto& t0 = e0->getComponent<Comp::Transform>();
    const auto& t1 = e1->getComponent<Comp::Transform>();
    const auto& b0 = e0->getComponent<Comp::BBox>();
    const auto& b1 = e1->getComponent<Comp::BBox>();

    const glm::vec2 scaledSize0 = glm::vec2(b0.w, b0.h) * glm::abs(glm::vec2(t0.scale.x, t0.scale.y));
    const glm::vec2 scaledSize1 = glm::vec2(b1.w, b1.h) * glm::abs(glm::vec2(t1.scale.x, t1.scale.y));

    const glm::vec2 pivotOffset0 = scaledSize0 * (Sprout::getPivotOffset(t0.pivot) - glm::vec2(0.5f));
    const glm::vec2 pivotOffset1 = scaledSize1 * (Sprout::getPivotOffset(t1.pivot) - glm::vec2(0.5f));

    const glm::vec2 pos0 = t0.position + pivotOffset0;
    const glm::vec2 pos1 = t1.position + pivotOffset1;

    const glm::vec2 dr = pos0 - pos1;
    const float dx = fabs(dr.x);
    const float dy = fabs(dr.y);

    const float xOverlap = scaledSize0.x / 2.0f + scaledSize1.x / 2.0f - dx;
    const float yOverlap = scaledSize0.y / 2.0f + scaledSize1.y / 2.0f - dy;

    if (xOverlap > 0 && yOverlap > 0) 
    {
        return {xOverlap, yOverlap};
    }
    return {0, 0};
}

auto bCircleCollision(const std::shared_ptr<Entity>& e0, const std::shared_ptr<Entity>& e1) -> glm::vec2
{
    /**
    Detects the overlap of the bounding circles of the two entities e0 and e1.
    Return: The overlap vector.
    **/
    
    if (e0->getId() == e1->getId()) return {0, 0}; // ignore self collision
    
    const glm::vec2 pos0 = e0->getComponent<Comp::Transform>().position;
    const glm::vec2 pos1 = e1->getComponent<Comp::Transform>().position;

    const float rsum = e0->getComponent<Comp::BCircle>().radius + e1->getComponent<Comp::BCircle>().radius;
    const float dist2 = glm::distance(pos0, pos1);
    
    if (dist2 <= rsum * rsum) 
    {
        const float dist = std::sqrt(dist2);
        glm::vec2 overlap = (pos0 - pos1) * ((rsum - dist) / dist);
        return overlap;
    }
    return {0, 0};
}

auto bBoxCircleCollision(const std::shared_ptr<Entity>& eBox, const std::shared_ptr<Entity>& eCircle) -> glm::vec2
{
    
    const glm::vec2 circlePos = eCircle->getComponent<Comp::Transform>().position;
    const float circleR = eCircle->getComponent<Comp::BCircle>().radius;

    const glm::vec2 boxPos = eBox->getComponent<Comp::Transform>().position;
    const float boxW = eBox->getComponent<Comp::BBox>().w;
    const float boxH = eBox->getComponent<Comp::BBox>().h;
    float boxLeft = boxPos.x - boxW / 2.0f;
    float boxRight = boxPos.x + boxW / 2.0f;
    float boxTop = boxPos.y - boxH / 2.0f;
    float boxBottom = boxPos.y + boxH / 2.0f;
    
    float testX = circlePos.x;
    float testY = circlePos.y;
    
    if (circlePos.x < boxLeft) 
        testX = boxLeft;
        
    else if (circlePos.x > boxRight) 
        testX = boxRight;
        
    if (circlePos.y < boxTop) 
        testY = boxTop;
        
    else if (circlePos.y > boxBottom) 
        testY = boxBottom;
    
    float distX = circlePos.x - testX;
    float distY = circlePos.y - testY;
    float distance = sqrt((distX*distX) + (distY*distY));
    
    if (distance <= circleR)
    {
        float overlap = circleR - distance;
        glm::vec2 overlapVector(distX, distY);
        if (distance != 0) 
        {
            overlapVector = glm::normalize(overlapVector);
            overlapVector *= overlap;
        } 
        else 
        {
            overlapVector = glm::vec2(circleR, 0);
        }
        return overlapVector;
    }
    
    return {0, 0};
}

auto Physics2D::collisionData(const std::shared_ptr<Entity>& e0, const std::shared_ptr<Entity>& e1) -> CollisionData
{
    if (e0->getId() == e1->getId()) return CollisionData();

    const auto& t0 = e0->getComponent<Comp::Transform>();
    const auto& t1 = e1->getComponent<Comp::Transform>();
    const auto& b0 = e0->getComponent<Comp::BBox>();
    const auto& b1 = e1->getComponent<Comp::BBox>();

    const glm::vec2 scaledSize0 = glm::vec2(b0.w, b0.h) * glm::abs(glm::vec2(t0.scale.x, t0.scale.y));
    const glm::vec2 scaledSize1 = glm::vec2(b1.w, b1.h) * glm::abs(glm::vec2(t1.scale.x, t1.scale.y));

    const glm::vec2 pivotOffset0 = scaledSize0 * (Sprout::getPivotOffset(t0.pivot) - glm::vec2(0.5f));
    const glm::vec2 pivotOffset1 = scaledSize1 * (Sprout::getPivotOffset(t1.pivot) - glm::vec2(0.5f));

    const glm::vec2 pos0 = t0.position + pivotOffset0;
    const glm::vec2 pos1 = t1.position + pivotOffset1;

    const glm::vec2 dr = pos0 - pos1;
    const float dx = fabs(dr.x);
    const float dy = fabs(dr.y);

    const float xOverlap = scaledSize0.x / 2.0f + scaledSize1.x / 2.0f - dx;
    const float yOverlap = scaledSize0.y / 2.0f + scaledSize1.y / 2.0f - dy;

    if (xOverlap > 0 && yOverlap > 0) 
    {
        CollisionData data;
        data.overlap = glm::vec2(xOverlap, yOverlap);
        data.normal = glm::normalize(dr);
        data.trigger = b0.isTrigger || b1.isTrigger;
        
        if (b0.isStatic)
        {
            if (b1.isStatic)
            {
               data.type = CollisionType::STATIC_STATIC;
            }
            else
            {
               data.type = CollisionType::STATIC_DYNAMIC;
            }
        } 
        else 
        {
            if (b1.isStatic)
            {
                data.type = CollisionType::DYNAMIC_STATIC;
            }
            else
            {
                data.type = CollisionType::DYNAMIC_DYNAMIC;
            }
        }
        
        if (b0.interactWithTriggers && b1.isTrigger)
        {
            data.triggerEvent = true;
        }
        
        return data;
    }
    return CollisionData();
}