//
//  PrefabEditor.cpp
//  SaplingEngine, Prefab Editor
//

#ifdef SAPLING_HAS_EDITOR

#include "Editor/PrefabEditor.hpp"
#include "Editor/ComponentInspector.hpp"
#include "Core/PrefabLoader.hpp"
#include "Core/AssetManager.hpp"
#include "Core/Logger.hpp"

#include <imgui.h>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

static constexpr float LEFT_PANEL_WIDTH = 320.0f;
static constexpr float FILE_PANEL_HEIGHT = 200.0f;

PrefabEditor::PrefabEditor(Engine& engine)
    : Scene(engine)
{
    init();
}

void PrefabEditor::init()
{
    m_cameraEntity = m_entityManager->addEntity({"camera"});
    auto& camT = m_cameraEntity->addComponent<Comp::Transform>(glm::vec3(0.0f, 2.0f, 5.0f));
    camT.rotation = glm::vec3(m_orbitPitch, m_orbitYaw, 0.0f);
    auto& cam = m_cameraEntity->addComponent<Comp::Camera>(60.0f, 0.1f, 1000.0f);
    cam.isActive = true;

    m_lightEntity = m_entityManager->addEntity({"light"});
    m_lightEntity->addComponent<Comp::Transform>(glm::vec3(3.0f, 5.0f, 3.0f), glm::vec3(-0.5f, -0.3f, 0.0f));
    m_lightEntity->addComponent<Comp::Light>(Comp::Light::Type::Directional, glm::vec3(1.0f), 1.0f);

    // ground grid
    {
        std::vector<Sprout::Mesh3DVertex> verts;
        std::vector<uint32_t> indices;
        constexpr float half = 5.0f;
        constexpr float lineW = 0.005f;
        constexpr int lines = 11;
        uint32_t idx = 0;

        for (int i = 0; i < lines; i++)
        {
            float x = -half + static_cast<float>(i);
            glm::vec3 n(0.0f, 1.0f, 0.0f);
            verts.push_back({{x - lineW, 0.0f, -half}, n, {0, 0}});
            verts.push_back({{x + lineW, 0.0f, -half}, n, {0, 0}});
            verts.push_back({{x + lineW, 0.0f,  half}, n, {0, 0}});
            verts.push_back({{x - lineW, 0.0f,  half}, n, {0, 0}});
            indices.push_back(idx); indices.push_back(idx+2); indices.push_back(idx+1);
            indices.push_back(idx); indices.push_back(idx+3); indices.push_back(idx+2);
            idx += 4;
        }
        for (int i = 0; i < lines; i++)
        {
            float z = -half + static_cast<float>(i);
            glm::vec3 n(0.0f, 1.0f, 0.0f);
            verts.push_back({{-half, 0.0f, z - lineW}, n, {0, 0}});
            verts.push_back({{ half, 0.0f, z - lineW}, n, {0, 0}});
            verts.push_back({{ half, 0.0f, z + lineW}, n, {0, 0}});
            verts.push_back({{-half, 0.0f, z + lineW}, n, {0, 0}});
            indices.push_back(idx); indices.push_back(idx+2); indices.push_back(idx+1);
            indices.push_back(idx); indices.push_back(idx+3); indices.push_back(idx+2);
            idx += 4;
        }

        m_gridMesh = std::make_shared<Sprout::Mesh>();
        m_gridMesh->loadFromData(verts, indices);

        m_gridMaterial = std::make_shared<Sprout::Material>();
        m_gridMaterial->create(Sprout::ShaderType::Mesh3D);
        m_gridMaterial->properties.baseColor = glm::vec4(0.85f, 0.85f, 0.85f, 0.85f);
        m_gridMaterial->properties.specularStrength = 0.0f;
    }

    createNewPrefab();
    scanPrefabFiles();
}

void PrefabEditor::update()
{
    // ImGui uses different coordinates, sokol viewport uses framebuffer pixels
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    float logicalW = viewport->Size.x;
    float logicalH = viewport->Size.y;

    // menu bar
    renderMenuBar();
    renderUnsavedPopup();
    float menuBarH = ImGui::GetFrameHeight();

    // left panels
    float leftW = LEFT_PANEL_WIDTH;
    float fileH = FILE_PANEL_HEIGHT;
    float inspectorH = logicalH - menuBarH - fileH;

    renderFilePanel(leftW, menuBarH, fileH);
    renderInspector(leftW, menuBarH + fileH, inspectorH);

    float vpX = leftW;
    float vpY = menuBarH;
    float vpW = logicalW - leftW;
    float vpH = logicalH - menuBarH;
    renderViewportPanel(vpX, vpY, vpW, vpH);

    // viewport rect in framebuffer pixels for sokol
    float dpiScale = sapp_dpi_scale();
    m_viewportRect = glm::vec4(leftW * dpiScale, menuBarH * dpiScale, vpW * dpiScale, vpH * dpiScale);

    auto& window = m_engine.getWindow();

    // orbit camera
    if (m_viewportHovered)
    {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {
            auto delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
            m_orbitYaw -= delta.x * 0.005f;
            m_orbitPitch += delta.y * 0.005f;
            m_orbitPitch = glm::clamp(m_orbitPitch, -1.5f, 1.5f);
        }

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            auto delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
            if (m_cameraEntity && m_cameraEntity->hasComponent<Comp::Transform>())
            {
                glm::vec3 current_target(m_orbitXOffset, m_orbitYOffset, m_orbitZOffset);
                if (m_previewEntity && m_previewEntity->hasComponent<Comp::Transform>())
                {
                    current_target += m_previewEntity->getComponent<Comp::Transform>().position;
                }
                auto& camT = m_cameraEntity->getComponent<Comp::Transform>();
                glm::vec3 forward = glm::normalize(current_target - camT.position);
                glm::vec3 world_up(0.0f, 1.0f, 0.0f);
                glm::vec3 right = glm::normalize(glm::cross(world_up, forward));
                glm::vec3 up = glm::normalize(glm::cross(forward, right));
                float fovRad = glm::radians(m_cameraEntity->getComponent<Comp::Camera>().fov);
                float sens = 2.0f * m_orbitDistance * std::tan(fovRad * 0.5f) / vpH;
                // horizontal
                float horiz_amount = delta.x * sens;
                m_orbitXOffset += right.x * horiz_amount;
                m_orbitZOffset += right.z * horiz_amount;
                // vertical
                float vert_amount = delta.y * sens;
                m_orbitXOffset += up.x * vert_amount;
                m_orbitYOffset += up.y * vert_amount;
                m_orbitZOffset += up.z * vert_amount;
            }
        }

        float scroll = ImGui::GetIO().MouseWheel;
        if (scroll != 0.0f)
        {
            m_orbitDistance -= scroll * 0.5f;
            m_orbitDistance = glm::clamp(m_orbitDistance, 0.5f, 50.0f);
        }
    }

    if (m_cameraEntity && m_cameraEntity->hasComponent<Comp::Transform>())
    {
        glm::vec3 orbitTarget(m_orbitXOffset, m_orbitYOffset, m_orbitZOffset);
        if (m_previewEntity && m_previewEntity->hasComponent<Comp::Transform>())
        {
            orbitTarget += m_previewEntity->getComponent<Comp::Transform>().position;
        }

        auto& camT = m_cameraEntity->getComponent<Comp::Transform>();
        float cx = m_orbitDistance * std::cos(m_orbitPitch) * std::sin(m_orbitYaw);
        float cy = m_orbitDistance * std::sin(m_orbitPitch);
        float cz = m_orbitDistance * std::cos(m_orbitPitch) * std::cos(m_orbitYaw);
        camT.position = orbitTarget + glm::vec3(cx, cy, cz);
        glm::vec3 dir = glm::normalize(orbitTarget - camT.position);
        camT.rotation.x = std::asin(dir.y);
        camT.rotation.y = std::atan2(-dir.x, -dir.z);
    }

    // advance animation
    if (m_previewEntity && m_previewEntity->hasComponentEnabled<Comp::Animator>())
    {
        auto& animator = m_previewEntity->getComponent<Comp::Animator>();
        animator.update(m_engine.deltaTime());
    }

    // viewport for rendering
    window.setClearColor(0.18f, 0.18f, 0.20f);
    window.draw_frame.viewport = m_viewportRect;

    // match editor panel aspect ratio
    window.draw_frame.view_projection = glm::ortho(0.0f, vpW, vpH, 0.0f, 1.0f, -1.0f);
    window.setCameraPosition(glm::vec2(-vpW * 0.5f, -vpH * 0.5f));

    auto& pass = window.getForward3DPass();
    pass.sceneData.ambientStrength = 0.4f;

    auto& entities = m_entityManager->getEntities();
    sRender3D(entities);

    if (m_gridMesh && m_gridMaterial)
    {
        pass.submit(m_gridMesh, m_gridMaterial, glm::mat4(1.0f));
    }

    if (vpW > 0 && vpH > 0 && m_cameraEntity && m_cameraEntity->hasComponent<Comp::Camera>())
    {
        float vpAspect = vpW / vpH;
        auto& cam = m_cameraEntity->getComponent<Comp::Camera>();
        pass.sceneData.projectionMatrix = cam.getProjectionMatrix(vpAspect);
    }

    sRender(entities);
}

void PrefabEditor::onSceneEnabled()
{
    m_engine.getWindow().setImGuiEnabled(true);
    scanPrefabFiles();
}

void PrefabEditor::onSceneDisabled()
{
    m_engine.getWindow().setImGuiEnabled(false);
    m_engine.getWindow().setClearColor(0.0f, 0.0f, 0.0f);
}

void PrefabEditor::loadPrefab(const std::string& path)
{
    m_prefabJson = PrefabLoader::getInstance().loadFile(path);
    m_currentFilePath = path;
    m_dirty = false;

    if (!m_prefabJson.contains("name") || !m_prefabJson["name"].is_string() ||
        m_prefabJson["name"].get<std::string>().empty())
    {
        std::string stem = std::filesystem::path(path).stem().string();
        m_prefabJson["name"] = stem;
    }

    rebuildPreviewEntity();
}

void PrefabEditor::savePrefab()
{
    if (m_currentFilePath.empty())
    {
        m_requestSaveAs = true;
        return;
    }

    if (m_previewEntity)
    {
        nlohmann::json components = nlohmann::json::object();
        ComponentInspector::serializeAllComponents(m_previewEntity, components);
        m_prefabJson["components"] = components;
    }

    std::string fullPath = AssetManager::getAssetsPath() + "/" + m_currentFilePath;
    std::ofstream file(fullPath);
    if (file.is_open())
    {
        file << m_prefabJson.dump(4);
        m_dirty = false;
        Logger::info("PrefabEditor: Saved " + m_currentFilePath);
    } else
    {
        Logger::error("PrefabEditor: Failed to save " + fullPath);
    }
}

void PrefabEditor::createNewPrefab()
{
    m_prefabJson = {
        {"name", ""},
        {"tags", nlohmann::json::array()},
        {"components", nlohmann::json::object()}
    };
    m_currentFilePath = "";
    m_dirty = false;
    rebuildPreviewEntity();
}

void PrefabEditor::scanPrefabFiles()
{
    m_prefabFiles.clear();
    std::string prefabDir = AssetManager::getAssetsPath() + "/Prefabs";
    if (std::filesystem::exists(prefabDir))
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(prefabDir))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".json")
            {
                auto relPath = std::filesystem::relative(entry.path(), AssetManager::getAssetsPath());
                m_prefabFiles.push_back(relPath.string());
            }
        }
        std::sort(m_prefabFiles.begin(), m_prefabFiles.end());
    }
}

void PrefabEditor::rebuildPreviewEntity()
{
    // destroy old
    if (m_previewEntity)
    {
        m_previewEntity->destroy();
        m_previewEntity = nullptr;
    }

    // make sure all entities are destroyed
    m_entityManager->update();

    // create new
    if (m_prefabJson.contains("components") && !m_prefabJson["components"].empty())
    {
        try
        {
            m_previewEntity = PrefabLoader::getInstance().createEntityFromJson(m_prefabJson, *m_entityManager);
            if (m_previewEntity && m_previewEntity->hasComponent<Comp::Camera>())
            {
                m_previewEntity->getComponent<Comp::Camera>().isActive = false;
            }
            m_orbitXOffset = 0.0f;
            m_orbitYOffset = 0.0f;
            m_orbitZOffset = 0.0f;
        }
        catch (const std::exception& e)
        {
            Logger::error("PrefabEditor: Failed to build preview: " + std::string(e.what()));
            m_previewEntity = nullptr;
        }
    }
}

void PrefabEditor::guardDirtyAction(std::function<void()> action)
{
    if (m_dirty)
    {
        m_pendingAction = std::move(action);
        m_showUnsavedPopup = true;
    }
    else
    {
        action();
    }
}

void PrefabEditor::renderUnsavedPopup()
{
    if (m_showUnsavedPopup)
    {
        ImGui::OpenPopup("Unsaved Changes");
        m_showUnsavedPopup = false;
    }
    if (ImGui::BeginPopupModal("Unsaved Changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("You have unsaved changes. Discard them?");
        if (ImGui::Button("Discard"))
        {
            if (m_pendingAction)
            {
                m_pendingAction();
                m_pendingAction = nullptr;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            m_pendingAction = nullptr;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}


void PrefabEditor::renderMenuBar()
{
    // crtl + s to save
    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S))
    {
        savePrefab();
    }

    bool openSaveAs = m_requestSaveAs;
    m_requestSaveAs = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Prefab"))
            {
                guardDirtyAction([this]() { createNewPrefab(); });
            }
            if (ImGui::MenuItem("Save", "Ctrl+S"))
            {
                savePrefab();
            }
            if (ImGui::MenuItem("Save As..."))
            {
                openSaveAs = true;
            }
            ImGui::EndMenu();
        }

        std::string title = "Prefab Editor";
        if (!m_currentFilePath.empty())
        {
            title += " - " + m_currentFilePath;
        }
        if (m_dirty) title += " *";
        float titleWidth = ImGui::CalcTextSize(title.c_str()).x;
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - titleWidth - 10.0f);
        ImGui::Text("%s", title.c_str());

        ImGui::EndMainMenuBar();
    }

    static char pathBuf[256];
    if (openSaveAs)
    {
        std::string defaultPath;
        if (!m_currentFilePath.empty())
        {
            defaultPath = m_currentFilePath;
        }
        else
        {
            std::string prefabName;
            if (m_prefabJson.contains("name") && m_prefabJson["name"].is_string())
                prefabName = m_prefabJson["name"].get<std::string>();
            if (prefabName.empty()) prefabName = "new_prefab";
            defaultPath = "Prefabs/" + prefabName + ".json";
        }
        strncpy(pathBuf, defaultPath.c_str(), sizeof(pathBuf) - 1);
        pathBuf[sizeof(pathBuf) - 1] = '\0';
        ImGui::OpenPopup("SaveAsPopup");
    }
    if (ImGui::BeginPopupModal("SaveAsPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("Path", pathBuf, sizeof(pathBuf));
        if (ImGui::Button("Save"))
        {
            m_currentFilePath = pathBuf;
            if (m_currentFilePath.size() < 5 ||
                m_currentFilePath.substr(m_currentFilePath.size() - 5) != ".json")
            {
                m_currentFilePath += ".json";
            }
            savePrefab();
            scanPrefabFiles();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void PrefabEditor::renderFilePanel(float width, float yStart, float height)
{
    ImGui::SetNextWindowPos(ImVec2(0, yStart), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
    if (ImGui::Begin("Files", nullptr, flags))
    {
        if (ImGui::Button("New")) guardDirtyAction([this]() { createNewPrefab(); });
        ImGui::SameLine();
        if (ImGui::Button("Refresh")) scanPrefabFiles();

        ImGui::Separator();

        for (int i = 0; i < static_cast<int>(m_prefabFiles.size()); i++)
        {
            bool selected = (i == m_selectedFileIndex);
            std::string displayName = std::filesystem::path(m_prefabFiles[i]).filename().string();
            if (ImGui::Selectable(displayName.c_str(), selected))
            {
                int idx = i;
                guardDirtyAction([this, idx]() {
                    m_selectedFileIndex = idx;
                    loadPrefab(m_prefabFiles[idx]);
                });
            }
        }
    }
    ImGui::End();
}

void PrefabEditor::renderInspector(float width, float yStart, float height)
{
    ImGui::SetNextWindowPos(ImVec2(0, yStart), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    if (ImGui::Begin("Inspector", nullptr, flags))
    {
        ImGui::BeginChild("InspectorScroll", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_AlwaysVerticalScrollbar);

        if (m_prefabJson.contains("name") && m_prefabJson["name"].is_string())
        {
            std::string name = m_prefabJson["name"].get<std::string>();
            char nameBuf[128];
            strncpy(nameBuf, name.c_str(), sizeof(nameBuf) - 1);
            nameBuf[sizeof(nameBuf) - 1] = '\0';
            if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf)))
            {
                m_prefabJson["name"] = std::string(nameBuf);
                m_dirty = true;
            }
        }

        // tags
        if (ImGui::TreeNode("Tags"))
        {
            if (m_prefabJson.contains("tags") && m_prefabJson["tags"].is_array())
            {
                int removeIdx = -1;
                for (int i = 0; i < static_cast<int>(m_prefabJson["tags"].size()); i++)
                {
                    std::string tag = m_prefabJson["tags"][i].get<std::string>();
                    char tagBuf[64];
                    strncpy(tagBuf, tag.c_str(), sizeof(tagBuf) - 1);
                    tagBuf[sizeof(tagBuf) - 1] = '\0';
                    ImGui::PushID(i);
                    if (ImGui::InputText("##tag", tagBuf, sizeof(tagBuf)))
                    {
                        m_prefabJson["tags"][i] = std::string(tagBuf);
                        m_dirty = true;
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("X")) removeIdx = i;
                    ImGui::PopID();
                }
                if (removeIdx >= 0)
                {
                    m_prefabJson["tags"].erase(removeIdx);
                    m_dirty = true;
                }
            }
            if (ImGui::SmallButton("+ Tag"))
            {
                m_prefabJson["tags"].push_back("");
                m_dirty = true;
            }
            ImGui::TreePop();
        }

        ImGui::Separator();

        // inspector
        if (m_previewEntity)
        {
            auto inspResult = ComponentInspector::renderAllComponents(m_previewEntity);
            if (inspResult.modified) {
                m_dirty = true;
            }
            if (!inspResult.removeComponent.empty())
            {
                m_prefabJson["components"].erase(inspResult.removeComponent);
                m_dirty = true;
                rebuildPreviewEntity();
            }
        }

        ImGui::Separator();

        if (ImGui::Button("+ Add Component"))
        {
            ImGui::OpenPopup("AddComponentPopup");
        }
        if (ImGui::BeginPopup("AddComponentPopup"))
        {
            auto names = PrefabLoader::getInstance().getRegisteredNames();
            for (const auto& name : names)
        {
                if (m_prefabJson.contains("components") && m_prefabJson["components"].contains(name))
                    continue;
                if (ImGui::MenuItem(name.c_str()))
                {
                    m_prefabJson["components"][name] = nlohmann::json::object();
                    m_dirty = true;
                    rebuildPreviewEntity();
                }
            }
            ImGui::EndPopup();
        }

        ImGui::Spacing();

        if (ImGui::Button("Save"))
        {
            savePrefab();
        }
        ImGui::SameLine();
        if (ImGui::Button("Revert") && !m_currentFilePath.empty())
        {
            loadPrefab(m_currentFilePath);
        }

        ImGui::EndChild();
    }
    ImGui::End();
}

void PrefabEditor::renderViewportPanel(float xStart, float yStart, float width, float height)
{
    ImGui::SetNextWindowPos(ImVec2(xStart, yStart), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (ImGui::Begin("##Viewport", nullptr, flags))
    {
        m_viewportHovered = ImGui::IsWindowHovered();
    }
    ImGui::End();
    ImGui::PopStyleColor();
}

#endif // SAPLING_HAS_EDITOR
