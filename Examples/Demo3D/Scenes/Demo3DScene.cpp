//
//  Demo3DScene.cpp
//  Sapling Demo3D
//
//  Tests all new unified 2D/3D rendering pipeline features:
//  - Phase 1: Unified render() instead of manual sRender3D+sRender
//  - Phase 2: 32-bit indices (implicit), binary search keyframes (implicit)
//  - Phase 3A: Multi-light (directional + orbiting point light)
//  - Phase 3B: Frustum culling via BSphere components
//  - Phase 3C: worldToScreen for HUD label above 3D object
//  - Phase 4A: Normal mapping + roughness maps on ant model
//  - Phase 4B: Material loaded from .mat JSON file
//  - Phase 4C: Unified texture registration via registerTexture/findTexture
//

#include "Demo3DScene.hpp"
#include "Core/AssetManager.hpp"
#include "Core/Input.hpp"
#include "ECS/Component.hpp"
#include "Renderer/Sprout.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Material.hpp"

#include "Utility/Color.hpp"

#include <cmath>
#include <sstream>
#include <iomanip>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Demo3DScene::Demo3DScene(Engine& engine) : Scene(engine)
{
    init();
}

void Demo3DScene::init()
{
    // ── Load 3D Assets ──────────────────────────────────

    // Load ant mesh
    auto antMesh = std::make_shared<Sprout::Mesh>();
    if (!antMesh->loadOBJ("Models/ant/Ant.obj")) {
        std::cerr << "Failed to load ant model" << '\n';
    }

    // TEST 4B: Load ant material from .mat JSON file
    auto antMat = std::make_shared<Sprout::Material>();
    if (!antMat->loadFromFile("Materials/ant.mat")) {
        std::cerr << "Failed to load ant.mat — falling back to defaults" << '\n';
        antMat->create(Sprout::ShaderType::Mesh3D, "Models/ant/Texture_Ant_bcolor.png");
    }

    // ── Camera ──────────────────────────────────────────
    auto cameraEntity = m_entityManager->addEntity({"camera"});
    cameraEntity->addComponent<Comp::Transform>(glm::vec3(0.0f, m_orbitHeight, m_orbitRadius));
    auto& cam = cameraEntity->addComponent<Comp::Camera>(60.0f, 0.1f, 500.0f);
    cam.isActive = true;

    // ── Directional Light ───────────────────────────────
    auto lightEntity = m_entityManager->addEntity({"light"});
    lightEntity->addComponent<Comp::Transform>(
        glm::vec3(0.0f),
        glm::vec3(glm::radians(-45.0f), glm::radians(-30.0f), 0.0f)
    );
    lightEntity->addComponent<Comp::Light>(
        Comp::Light::Type::Directional,
        glm::vec3(1.0f, 0.95f, 0.9f),
        0.8f
    );

    // TEST 3A: Point light orbiting the scene
    m_pointLightEntity = m_entityManager->addEntity({"pointlight", "light"});
    m_pointLightEntity->addComponent<Comp::Transform>(glm::vec3(3.0f, 1.5f, 0.0f));
    auto& pointLight = m_pointLightEntity->addComponent<Comp::Light>();
    pointLight.type = Comp::Light::Type::Point;
    pointLight.color = glm::vec3(0.3f, 0.5f, 1.0f); // blue-ish
    pointLight.intensity = 2.0f;
    pointLight.range = 8.0f;

    // ── Ground Plane ────────────────────────────────────
    auto ground = m_entityManager->addEntity({"ground", "drawable3d"});
    auto& groundTransform = ground->addComponent<Comp::Transform>(glm::vec3(0.0f, 0.0f, 0.0f));
    groundTransform.scale = glm::vec3(10.0f, 1.0f, 10.0f);
    auto groundMat = std::make_shared<Sprout::Material>();
    groundMat->create(Sprout::ShaderType::Mesh3D);
    groundMat->properties.baseColor = glm::vec4(0.3f, 0.6f, 0.3f, 1.0f);
    groundMat->properties.specularStrength = 0.1f;
    ground->addComponent<Comp::MeshRenderer>(
        Sprout::Mesh::createPlane(),
        groundMat
    );

    // ── Center Cube ─────────────────────────────────────
    auto cube = m_entityManager->addEntity({"cube", "drawable3d"});
    cube->addComponent<Comp::Transform>(glm::vec3(0.0f, 0.75f, 0.0f));
    auto cubeMat = std::make_shared<Sprout::Material>();
    cubeMat->create(Sprout::ShaderType::Mesh3D);
    cubeMat->properties.baseColor = glm::vec4(0.9f, 0.2f, 0.2f, 1.0f);
    cubeMat->properties.specularStrength = 0.8f;
    cubeMat->properties.shininess = 64.0f;
    cube->addComponent<Comp::MeshRenderer>(
        Sprout::Mesh::createCube(),
        cubeMat
    );
    // TEST 3B: Add bounding sphere for frustum culling
    cube->addComponent<Comp::BSphere>(0.87f); // cube diagonal / 2

    // ── Ant Model (with normal mapping from .mat file) ──
    auto ant = m_entityManager->addEntity({"ant", "drawable3d"});
    auto& antTransform = ant->addComponent<Comp::Transform>(glm::vec3(2.5f, 0.0f, 0.0f));
    antTransform.scale = glm::vec3(200.0f);
    ant->addComponent<Comp::MeshRenderer>(antMesh, antMat);
    // TEST 3B: Bounding sphere for ant
    ant->addComponent<Comp::BSphere>(1.5f);

    // ── Animated Character (Walking Male) ───────────────
    m_characterEntity = m_entityManager->loadPrefab("Prefabs/male_character.json");

    // ── Second Cube (smaller, offset) ───────────────────
    auto cube2 = m_entityManager->addEntity({"cube2", "drawable3d"});
    auto& cube2Transform = cube2->addComponent<Comp::Transform>(glm::vec3(-2.0f, 0.4f, 1.5f));
    cube2Transform.scale = glm::vec3(0.8f);
    auto cube2Mat = std::make_shared<Sprout::Material>();
    cube2Mat->create(Sprout::ShaderType::Mesh3D);
    cube2Mat->properties.baseColor = glm::vec4(0.9f, 0.8f, 0.2f, 1.0f);
    cube2Mat->properties.specularStrength = 0.5f;
    cube2->addComponent<Comp::MeshRenderer>(
        Sprout::Mesh::createCube(),
        cube2Mat
    );
    // TEST 3B: Bounding sphere
    cube2->addComponent<Comp::BSphere>(0.7f);

    // ── Sphere to show normal mapping effect ────────────
    auto sphere = m_entityManager->addEntity({"sphere", "drawable3d"});
    auto& sphereTransform = sphere->addComponent<Comp::Transform>(glm::vec3(-2.5f, 0.6f, -1.5f));
    sphereTransform.scale = glm::vec3(1.0f);
    auto sphereMat = std::make_shared<Sprout::Material>();
    sphereMat->create(Sprout::ShaderType::Mesh3D);
    sphereMat->properties.baseColor = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
    sphereMat->properties.specularStrength = 0.9f;
    sphereMat->properties.shininess = 128.0f;
    sphere->addComponent<Comp::MeshRenderer>(
        Sprout::Mesh::createSphere(32, 32),
        sphereMat
    );
    sphere->addComponent<Comp::BSphere>(0.5f);

    // ── 2D HUD Text ─────────────────────────────────────
    auto title = m_entityManager->addEntity({"ui"});
    title->addComponent<Comp::Transform>(glm::vec2(10, 10), Sprout::Pivot::TOP_LEFT, true);
    title->addComponent<Comp::Text>(
        "Sapling 3D Demo - Unified Pipeline Test",
        "ui_font", 16, Color::White,
        Sprout::TextJustify::LEFT
    );

    auto controls = m_entityManager->addEntity({"ui"});
    controls->addComponent<Comp::Transform>(glm::vec2(10, 40), Sprout::Pivot::TOP_LEFT, true);
    controls->addComponent<Comp::Text>(
        "A/D: Orbit  W/S: Zoom  1: Walk  2: Kneel  ESC: Quit",
        "ui_font", 10, Color::LightGray,
        Sprout::TextJustify::LEFT
    );

    // TEST 3C: Label entity that tracks a 3D object's screen position
    m_antLabelEntity = m_entityManager->addEntity({"ui"});
    m_antLabelEntity->addComponent<Comp::Transform>(glm::vec2(0, 0), Sprout::Pivot::CENTER, true);
    m_antLabelEntity->addComponent<Comp::Text>(
        "Ant (Normal Mapped)",
        "ui_font", 10, Color::Yellow,
        Sprout::TextJustify::CENTER
    );
}

void Demo3DScene::update()
{
    float dt = m_engine.deltaTime();

    // ── Camera orbit controls ───────────────────────────
    if (Input::isAction("moveLeft"))
        m_orbitAngle += m_orbitSpeed * dt;
    if (Input::isAction("moveRight"))
        m_orbitAngle -= m_orbitSpeed * dt;
    if (Input::isAction("moveUp"))
        m_orbitRadius = std::max(2.0f, m_orbitRadius - 3.0f * dt);
    if (Input::isAction("moveDown"))
        m_orbitRadius = std::min(15.0f, m_orbitRadius + 3.0f * dt);

    if (Input::isActionUp("quit"))
        sapp_request_quit();

    // Animation switching with crossfade
    if (m_characterEntity) {
        auto& animator = m_characterEntity->getComponent<Comp::Animator>();
        if (Input::isActionDown("anim1"))
            animator.play(AssetManager::getAnimationClip("walk"), true, 0.3f);
        if (Input::isActionDown("anim2"))
            animator.play(AssetManager::getAnimationClip("kneel"), true, 0.3f);
    }

    // Update camera position (orbit around origin)
    auto& cameras = m_entityManager->getEntities("camera");
    if (!cameras.empty())
    {
        auto& camTransform = cameras[0]->getComponent<Comp::Transform>();
        camTransform.position.x = std::sin(m_orbitAngle) * m_orbitRadius;
        camTransform.position.z = std::cos(m_orbitAngle) * m_orbitRadius;
        camTransform.position.y = m_orbitHeight;

        glm::vec3 toTarget = glm::normalize(-camTransform.position);
        camTransform.rotation.x = std::asin(toTarget.y);
        camTransform.rotation.y = std::atan2(-toTarget.x, -toTarget.z);
    }

    // ── Rotate the center cube ──────────────────────────
    m_cubeRotation += dt * 0.8f;
    auto& cubes = m_entityManager->getEntities("cube");
    if (!cubes.empty())
    {
        auto& cubeTransform = cubes[0]->getComponent<Comp::Transform>();
        cubeTransform.rotation.y = m_cubeRotation;
        cubeTransform.position.y = 0.75f + 0.15f * std::sin(m_cubeRotation * 2.0f);
    }

    // ── Rotate the second cube on a different axis ──────
    auto& cubes2 = m_entityManager->getEntities("cube2");
    if (!cubes2.empty())
    {
        auto& c2t = cubes2[0]->getComponent<Comp::Transform>();
        c2t.rotation.x = m_cubeRotation * 0.7f;
        c2t.rotation.z = m_cubeRotation * 0.4f;
    }

    // TEST 3A: Orbit the point light around the scene
    m_pointLightAngle += dt * 1.2f;
    if (m_pointLightEntity)
    {
        auto& plt = m_pointLightEntity->getComponent<Comp::Transform>();
        plt.position.x = std::cos(m_pointLightAngle) * 3.5f;
        plt.position.z = std::sin(m_pointLightAngle) * 3.5f;
        plt.position.y = 1.5f + 0.5f * std::sin(m_pointLightAngle * 2.0f);
    }

    // TEST 3C: worldToScreen — project ant position to screen for HUD label
    if (m_antLabelEntity && !cameras.empty())
    {
        auto& cam = cameras[0]->getComponent<Comp::Camera>();
        float screenW = static_cast<float>(m_engine.getWindow().getWidth());
        float screenH = static_cast<float>(m_engine.getWindow().getHeight());

        // Ant is at (2.5, 0, 0) with scale 200 — label above it
        glm::vec3 antWorldPos(2.5f, 1.5f, 0.0f);
        glm::vec3 screenPos = cam.worldToScreen(antWorldPos, screenW, screenH);

        // Only show if in front of camera (z > 0 and z < 1)
        auto& labelText = m_antLabelEntity->getComponent<Comp::Text>();
        if (screenPos.z > 0.0f && screenPos.z < 1.0f)
        {
            auto& labelTransform = m_antLabelEntity->getComponent<Comp::Transform>();
            labelTransform.position = glm::vec3(screenPos.x, screenPos.y, 0.0f);
            labelText.enabled = true;
        }
        else
        {
            labelText.enabled = false;
        }
    }

    // TEST 1D: Use unified render() instead of manual sRender3D + sRender
    render();
}

void Demo3DScene::onSceneEnabled()
{
}
