//
//  Scene.cpp
//  SaplingEngine, Canopy Scene Manager
//

#include "Core/Scene.hpp"
#include "Core/AudioEngine.hpp"
#include "ECS/Component.hpp"
#include "Renderer/Sprout.hpp"
#include "Renderer/Forward3DPass.hpp"
#include "Core/SceneMessage.hpp"
#include "Core/Logger.hpp"


Scene::Scene(Engine& engine) : m_engine(engine)
{
    m_entityManager = std::make_shared<EntityManager>();
}


void Scene::preUpdate()
{
    m_entityManager->update();
    AudioEngine::update();
    sUpdateAnimations(m_entityManager->getEntities());
}

void Scene::sUpdateAnimations(EntityList& entities)
{
    float dt = m_engine.deltaTime();

    for (const auto& e : entities)
    {
        if (e->hasComponentEnabled<Comp::Sprite>())
        {
            auto& cSprite = e->getComponent<Comp::Sprite>();

            if (cSprite.type == Comp::Sprite::Type::Animated)
            {
                cSprite.animationTime += dt;
                if (cSprite.animationTime >= (1.0f / cSprite.animationSpeed))
                {
                    cSprite.currentFrame = (cSprite.currentFrame + 1) % cSprite.numFrames;
                    cSprite.animationTime = 0.0f;
                }
            }

            if (cSprite.colorOverrideTime > 0)
            {
                cSprite.colorOverrideTime -= dt;
                if (cSprite.colorOverrideTime <= 0)
                {
                    cSprite.color_override = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
                    cSprite.colorOverrideTime = 0;
                }
            }
        }

        if (e->hasComponentEnabled<Comp::Animator>())
        {
            auto& animator = e->getComponent<Comp::Animator>();
            animator.update(dt);
        }
    }
}

void Scene::postUpdate()
{
    Input::clean();
}

void Scene::onSceneEnabled(){}
void Scene::onSceneDisabled(){}

void Scene::enable()
{
    // set the window's event callback to our input system
    m_engine.getWindow().SetEventCallback([](const sapp_event* e) { Input::update(e); });
    Logger::info("Enabling scene");
    onSceneEnabled();
}

void Scene::disable()
{
    Logger::info("Disabling scene");
    onSceneDisabled();
}

void Scene::sRender(EntityList& entities)
{
    glm::f32 numEntities = entities.size();
    glm::f32 currentEnt = 0;

    for (const auto& e : entities)
    {
        if (e->hasComponent<Comp::GridTransform>() && e->hasComponent<Comp::Transform>())
        {
            auto& gridTransform = e->getComponent<Comp::GridTransform>();
            auto& transform = e->getComponent<Comp::Transform>();
            auto worldPos = gridTransform.getWorldPosition();
            transform.position = glm::vec3(worldPos, transform.position.z);
        }

        if (e->hasComponentEnabled<Comp::Sprite>() && e->getComponent<Comp::Sprite>().texture)
        {
            auto& cSprite = e->getComponent<Comp::Sprite>();

            glm::f32 depth = 1 - (static_cast<glm::f32>(cSprite.layer) + currentEnt / numEntities) / static_cast<glm::f32>(Comp::Layer::Count);
            glm::vec2 pos = glm::vec2(0.0f);
            glm::vec3 scale = glm::vec3(1.0f);
            glm::f32 rotation = 0.0f;
            Sprout::Pivot pivot = Sprout::Pivot::CENTER;
            bool worldSpace = true;

            if (e->hasComponent<Comp::Transform>())
            {
                auto& cTransform = e->getComponent<Comp::Transform>();
                pos = glm::vec2(cTransform.position) + cSprite.transformOffset;
                scale = cTransform.scale * cSprite.scaleOffset;
                rotation = cTransform.rotation.z;
                pivot = cTransform.pivot;
                worldSpace = !cTransform.screenSpace;
            }

            // apply pixels-per-unit scaling when rendering in world space
            if (worldSpace && cSprite.pixelsPerUnit > 0.0f)
            {
                float worldScaleFactor = 1.0f / cSprite.pixelsPerUnit;
                scale *= worldScaleFactor;
            }

            if (cSprite.flip_X)
            {
                scale.x *= -1;
            }

            // independent mode textures (not atlas-packed) must use draw_image
            if (cSprite.texture->getMode() == Sprout::TextureMode::Independent) {
                m_engine.getWindow().draw_image(cSprite.texture, pos, depth, rotation, scale, pivot, cSprite.color_override, worldSpace);
            } else {
                m_engine.getWindow().draw_sprite(cSprite.texture, pos, depth, rotation, (int)cSprite.currentFrame, cSprite.color_override, scale, pivot, worldSpace);
            }
        }
        if (e->hasComponentEnabled<Comp::Text>())
        {
            auto& cText = e->getComponent<Comp::Text>();

            if (!cText.font.empty() && AssetManager::hasFont(cText.font))
            {
                glm::vec2 pos = glm::vec2(0.0f);
                float scale = 0.025f * cText.size;
                Sprout::Pivot pivot = Sprout::Pivot::TOP_LEFT;
                bool worldSpace = true;
                float depth = 1 - (static_cast<glm::f32>(cText.layer) + currentEnt / numEntities) / static_cast<glm::f32>(Comp::Layer::Count);

                if (e->hasComponent<Comp::Transform>())
                {
                    auto& cTransform = e->getComponent<Comp::Transform>();
                    pos = glm::vec2(cTransform.position) + cText.transformOffset;
                    pivot = cTransform.pivot;
                    worldSpace = !cTransform.screenSpace;
                }

                m_engine.getWindow().draw_text(cText.text, AssetManager::getFont(cText.font), pos, depth, cText.color, scale, pivot, worldSpace, cText.justify);
            }
        }

        currentEnt++;
    }
}

void Scene::setupRenderState(EntityList& entities)
{
    auto& pass = m_engine.getWindow().getForward3DPass();

    // find active camera
    Comp::Camera* activeCamera = nullptr;
    Comp::Transform* cameraTransform = nullptr;
    for (const auto& e : entities)
    {
        if (e->hasComponentEnabled<Comp::Camera>())
        {
            auto& cam = e->getComponent<Comp::Camera>();
            if (cam.isActive)
            {
                activeCamera = &cam;
                if (e->hasComponent<Comp::Transform>())
                {
                    cameraTransform = &e->getComponent<Comp::Transform>();
                }
                break;
            }
        }
    }

    if (!activeCamera || !cameraTransform)
    {
        return; // no active camera, skip 3D setup
    }

    // set up scene data
    float aspectRatio = static_cast<float>(m_engine.getWindow().getWidth()) /
                        static_cast<float>(m_engine.getWindow().getHeight());
    pass.sceneData.viewMatrix = activeCamera->getViewMatrix();
    pass.sceneData.projectionMatrix = activeCamera->getProjectionMatrix(aspectRatio);
    pass.sceneData.cameraPosition = cameraTransform->position;

    // extract frustum planes for culling
    pass.frustum.extract(pass.sceneData.projectionMatrix * pass.sceneData.viewMatrix);

    // collect up to MAX_LIGHTS lights (directional + point)
    pass.sceneData.numLights = 0;
    for (const auto& e : entities)
    {
        if (pass.sceneData.numLights >= Sprout::MAX_LIGHTS) break;

        if (e->hasComponentEnabled<Comp::Light>())
        {
            auto& light = e->getComponent<Comp::Light>();
            auto& ld = pass.sceneData.lights[pass.sceneData.numLights];

            if (light.type == Comp::Light::Type::Directional)
            {
                ld.type = 0.0f;
                if (e->hasComponent<Comp::Transform>())
                {
                    auto& lt = e->getComponent<Comp::Transform>();
                    ld.positionOrDirection = glm::normalize(lt.forward());
                }
                ld.color = light.color * light.intensity;
                ld.range = 0.0f;
                pass.sceneData.numLights++;
            }
            else if (light.type == Comp::Light::Type::Point)
            {
                ld.type = 1.0f;
                if (e->hasComponent<Comp::Transform>())
                {
                    ld.positionOrDirection = e->getComponent<Comp::Transform>().position;
                }
                ld.color = light.color * light.intensity;
                ld.range = light.range;
                pass.sceneData.numLights++;
            }
        }
    }
}

void Scene::sRender3D(EntityList& entities)
{
    setupRenderState(entities);

    auto& pass = m_engine.getWindow().getForward3DPass();

    // submit mesh draw requests (with frustum culling)
    for (const auto& e : entities)
    {
        if (e->hasComponentEnabled<Comp::MeshRenderer>() && e->hasComponent<Comp::Transform>())
        {
            auto& transform = e->getComponent<Comp::Transform>();

            // frustum culling — entities without bounding volumes always pass (conservative)
            if (e->hasComponent<Comp::BSphere>())
            {
                auto& sphere = e->getComponent<Comp::BSphere>();
                float maxScale = glm::max(glm::max(std::abs(transform.scale.x), std::abs(transform.scale.y)), std::abs(transform.scale.z));
                if (!pass.frustum.testSphere(transform.position, sphere.radius * maxScale))
                    continue;
            }
            else if (e->hasComponent<Comp::BBox3D>())
            {
                auto& box = e->getComponent<Comp::BBox3D>();
                glm::vec3 scaledExtents = box.halfExtents * glm::abs(transform.scale);
                if (!pass.frustum.testAABB(transform.position, scaledExtents))
                    continue;
            }

            auto& meshRenderer = e->getComponent<Comp::MeshRenderer>();
            if (!meshRenderer.mesh) continue;

            bool isSkinned = false;
            const std::vector<glm::mat4>* boneMatrices = nullptr;
            if (e->hasComponentEnabled<Comp::Animator>())
            {
                auto& animator = e->getComponent<Comp::Animator>();
                if (!animator.boneMatrices.empty())
                {
                    isSkinned = true;
                    boneMatrices = &animator.boneMatrices;
                }
            }

            Sprout::ShaderType shaderType = meshRenderer.shaderOverride;
            if (shaderType == Sprout::ShaderType::Custom)
            {
                shaderType = Sprout::ShaderType::Mesh3D;
            }

            if (!meshRenderer.material)
            {
                const std::string defaultMatName =
                    "__default_mesh3d";

                if (AssetManager::hasMaterial(defaultMatName))
                {
                    meshRenderer.material = AssetManager::getMaterial(defaultMatName);
                }
                else
                {
                    auto defaultMaterial = std::make_shared<Sprout::Material>();
                    defaultMaterial->create(shaderType);
                    AssetManager::addMaterial(defaultMatName, defaultMaterial);
                    meshRenderer.material = defaultMaterial;
                }
            }

            Sprout::PipelineState3D pipelineState;
            pipelineState.depthTest = meshRenderer.depthTest;
            pipelineState.depthWrite = meshRenderer.depthWrite;
            pipelineState.doubleSided = meshRenderer.doubleSided;
            pipelineState.blendMode = (meshRenderer.blendMode == 1)
                ? Sprout::BlendMode3D::Opaque
                : Sprout::BlendMode3D::Alpha;

            pass.submit(meshRenderer.mesh, meshRenderer.material, transform.getModelMatrix(),
                        pipelineState, isSkinned, boneMatrices);
        }
    }
}

void Scene::render()
{
    auto& entities = m_entityManager->getEntities();

    // set up 3D render state (camera, projection, frustum, lights)
    setupRenderState(entities);

    auto& window = m_engine.getWindow();
    auto& pass = window.getForward3DPass();

    // save the original 2D projection for screen-space elements
    glm::mat4 orthoViewProj = window.draw_frame.view_projection;
    glm::mat4 orthoCameraXform = window.draw_frame.camera_xform;

    // for world-space 2D elements, use the 3D camera's view-projection
    // so sprites appear at their 3D world position as billboards
    glm::mat4 viewProj3D = pass.sceneData.projectionMatrix * pass.sceneData.viewMatrix;

    glm::f32 numEntities = entities.size();
    glm::f32 currentEnt = 0;

    for (const auto& e : entities)
    {
        // GridTransform → Transform sync
        if (e->hasComponent<Comp::GridTransform>() && e->hasComponent<Comp::Transform>())
        {
            auto& gridTransform = e->getComponent<Comp::GridTransform>();
            auto& transform = e->getComponent<Comp::Transform>();
            auto worldPos = gridTransform.getWorldPosition();
            transform.position = glm::vec3(worldPos, transform.position.z);
        }

        // --- 3D: MeshRenderer submission ---
        if (e->hasComponentEnabled<Comp::MeshRenderer>() && e->hasComponent<Comp::Transform>())
        {
            auto& transform = e->getComponent<Comp::Transform>();

            bool culled = false;
            if (e->hasComponent<Comp::BSphere>())
            {
                auto& sphere = e->getComponent<Comp::BSphere>();
                float maxScale = glm::max(glm::max(std::abs(transform.scale.x), std::abs(transform.scale.y)), std::abs(transform.scale.z));
                if (!pass.frustum.testSphere(transform.position, sphere.radius * maxScale))
                    culled = true;
            }
            else if (e->hasComponent<Comp::BBox3D>())
            {
                auto& box = e->getComponent<Comp::BBox3D>();
                glm::vec3 scaledExtents = box.halfExtents * glm::abs(transform.scale);
                if (!pass.frustum.testAABB(transform.position, scaledExtents))
                    culled = true;
            }

            if (!culled)
            {
                auto& meshRenderer = e->getComponent<Comp::MeshRenderer>();
                if (meshRenderer.mesh)
                {
                    bool isSkinned = false;
                    const std::vector<glm::mat4>* boneMatrices = nullptr;
                    if (e->hasComponentEnabled<Comp::Animator>())
                    {
                        auto& animator = e->getComponent<Comp::Animator>();
                        if (!animator.boneMatrices.empty())
                        {
                            isSkinned = true;
                            boneMatrices = &animator.boneMatrices;
                        }
                    }

                    Sprout::ShaderType shaderType = meshRenderer.shaderOverride;
                    if (shaderType == Sprout::ShaderType::Custom)
                    {
                        shaderType = Sprout::ShaderType::Mesh3D;
                    }

                    if (!meshRenderer.material)
                    {
                        const std::string defaultMatName = "__default_mesh3d";
                        if (AssetManager::hasMaterial(defaultMatName))
                        {
                            meshRenderer.material = AssetManager::getMaterial(defaultMatName);
                        }
                        else
                        {
                            auto defaultMaterial = std::make_shared<Sprout::Material>();
                            defaultMaterial->create(shaderType);
                            AssetManager::addMaterial(defaultMatName, defaultMaterial);
                            meshRenderer.material = defaultMaterial;
                        }
                    }

                    Sprout::PipelineState3D pipelineState;
                    pipelineState.depthTest = meshRenderer.depthTest;
                    pipelineState.depthWrite = meshRenderer.depthWrite;
                    pipelineState.doubleSided = meshRenderer.doubleSided;
                    pipelineState.blendMode = (meshRenderer.blendMode == 1)
                        ? Sprout::BlendMode3D::Opaque
                        : Sprout::BlendMode3D::Alpha;

                    pass.submit(meshRenderer.mesh, meshRenderer.material, transform.getModelMatrix(),
                                pipelineState, isSkinned, boneMatrices);
                }
            }
        }

        // --- 2D: Sprite ---
        if (e->hasComponentEnabled<Comp::Sprite>() && e->getComponent<Comp::Sprite>().texture)
        {
            auto& cSprite = e->getComponent<Comp::Sprite>();

            glm::f32 depth = 1 - (static_cast<glm::f32>(cSprite.layer) + currentEnt / numEntities) / static_cast<glm::f32>(Comp::Layer::Count);
            glm::vec2 pos = glm::vec2(0.0f);
            glm::vec3 scale = glm::vec3(1.0f);
            glm::f32 rotation = 0.0f;
            Sprout::Pivot pivot = Sprout::Pivot::CENTER;
            bool worldSpace = true;

            if (e->hasComponent<Comp::Transform>())
            {
                auto& cTransform = e->getComponent<Comp::Transform>();
                pos = glm::vec2(cTransform.position) + cSprite.transformOffset;
                scale = cTransform.scale * cSprite.scaleOffset;
                rotation = cTransform.rotation.z;
                pivot = cTransform.pivot;
                worldSpace = !cTransform.screenSpace;
            }

            if (worldSpace)
            {
                // use 3D camera so sprite appears at its world position
                window.draw_frame.view_projection = viewProj3D;
                window.draw_frame.camera_xform = glm::mat4(1.0f);

                // apply pixels-per-unit scaling when rendering in world space
                if (cSprite.pixelsPerUnit > 0.0f)
                {
                    float worldScaleFactor = 1.0f / cSprite.pixelsPerUnit;
                    scale *= worldScaleFactor;
                }
            }
            else
            {
                // screen-space: use 2D ortho projection
                window.draw_frame.view_projection = orthoViewProj;
                window.draw_frame.camera_xform = orthoCameraXform;
            }

            if (cSprite.flip_X)
            {
                scale.x *= -1;
            }

            if (cSprite.texture->getMode() == Sprout::TextureMode::Independent) {
                m_engine.getWindow().draw_image(cSprite.texture, pos, depth, rotation, scale, pivot, cSprite.color_override, worldSpace);
            } else {
                m_engine.getWindow().draw_sprite(cSprite.texture, pos, depth, rotation, (int)cSprite.currentFrame, cSprite.color_override, scale, pivot, worldSpace);
            }
        }

        // --- 2D: Text ---
        if (e->hasComponentEnabled<Comp::Text>())
        {
            auto& cText = e->getComponent<Comp::Text>();

            if (!cText.font.empty() && AssetManager::hasFont(cText.font))
            {
                glm::vec2 pos = glm::vec2(0.0f);
                float scale = 0.025f * cText.size;
                Sprout::Pivot pivot = Sprout::Pivot::TOP_LEFT;
                bool worldSpace = true;
                float depth = 1 - (static_cast<glm::f32>(cText.layer) + currentEnt / numEntities) / static_cast<glm::f32>(Comp::Layer::Count);

                if (e->hasComponent<Comp::Transform>())
                {
                    auto& cTransform = e->getComponent<Comp::Transform>();
                    pos = glm::vec2(cTransform.position) + cText.transformOffset;
                    pivot = cTransform.pivot;
                    worldSpace = !cTransform.screenSpace;
                }

                if (worldSpace)
                {
                    window.draw_frame.view_projection = viewProj3D;
                    window.draw_frame.camera_xform = glm::mat4(1.0f);
                }
                else
                {
                    window.draw_frame.view_projection = orthoViewProj;
                    window.draw_frame.camera_xform = orthoCameraXform;
                }

                window.draw_text(cText.text, AssetManager::getFont(cText.font), pos, depth, cText.color, scale, pivot, worldSpace, cText.justify);
            }
        }

        currentEnt++;
    }

    // restore 2D projection state
    window.draw_frame.view_projection = orthoViewProj;
    window.draw_frame.camera_xform = orthoCameraXform;
}
