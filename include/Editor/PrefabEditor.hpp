//
//  PrefabEditor.hpp
//  SaplingEngine, Editor
//

#pragma once

#ifdef SAPLING_HAS_EDITOR

#include "Core/Scene.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Material.hpp"
#include <nlohmann/json.hpp>
#include <functional>
#include <string>
#include <vector>

class PrefabEditor : public Scene
{
public:
    explicit PrefabEditor(Engine& engine);

    void init() override;
    void update() override;
    void onSceneEnabled() override;
    void onSceneDisabled() override;

private:
    void loadPrefab(const std::string& path);
    void savePrefab();
    void createNewPrefab();
    void scanPrefabFiles();

    void renderMenuBar();
    void renderFilePanel(float width, float yStart, float height);
    void renderInspector(float width, float yStart, float height);
    void renderViewportPanel(float xStart, float yStart, float width, float height);

    void rebuildPreviewEntity();

    nlohmann::json m_prefabJson;
    std::string m_currentFilePath;
    bool m_dirty = false;

    std::shared_ptr<Entity> m_previewEntity;
    std::shared_ptr<Entity> m_cameraEntity;
    std::shared_ptr<Entity> m_lightEntity;

    std::vector<std::string> m_prefabFiles;
    int m_selectedFileIndex = -1;

    float m_orbitYaw = 0.4f;
    float m_orbitPitch = 0.3f;
    float m_orbitDistance = 5.0f;

    float m_orbitXOffset = 0.0f;
    float m_orbitYOffset = 0.0f;
    float m_orbitZOffset = 0.0f;

    std::shared_ptr<Sprout::Mesh> m_gridMesh;
    std::shared_ptr<Sprout::Material> m_gridMaterial;

    glm::vec4 m_viewportRect = glm::vec4(0.0f);
    bool m_viewportHovered = false;

    bool m_requestSaveAs = false;

    bool m_showUnsavedPopup = false;
    std::function<void()> m_pendingAction;
    void guardDirtyAction(std::function<void()> action);
    void renderUnsavedPopup();
};

#endif // SAPLING_HAS_EDITOR
