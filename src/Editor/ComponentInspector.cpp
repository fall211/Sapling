//
//  ComponentInspector.cpp
//  SaplingEngine, Editor
//

#ifdef SAPLING_HAS_EDITOR

#include "Editor/ComponentInspector.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Component.hpp"
#include "Core/AssetManager.hpp"
#include "Core/Logger.hpp"

#include <imgui.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <regex>
#include <string>
#include <vector>

struct AssetPickerResult
{
    bool changed = false;
    std::string path;
    std::string assetName;
};

static AssetPickerResult assetFilePicker(const char* label, const std::string& currentPath,
                                         const std::vector<std::string>& extensions)
{
    AssetPickerResult result;

    std::string displayName = "(none)";
    if (!currentPath.empty())
        displayName = std::filesystem::path(currentPath).filename().string();

    static std::unordered_map<std::string, std::string> s_filters;
    std::string& filter = s_filters[label];

    float labelWidth = ImGui::CalcTextSize(label, nullptr, true).x;
    float innerSpacing = ImGui::GetStyle().ItemInnerSpacing.x;
    float clearButtonSpace = currentPath.empty() ? 0.0f : (ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.x);
    float availableWidth = ImGui::GetContentRegionAvail().x;
    float comboWidth = availableWidth - labelWidth - innerSpacing - clearButtonSpace;
    if (comboWidth < 60.0f) comboWidth = 60.0f;

    ImGui::SetNextItemWidth(comboWidth);

    if (ImGui::BeginCombo(label, displayName.c_str()))
    {
        char filterBuf[256] = {0};
        strncpy(filterBuf, filter.c_str(), sizeof(filterBuf) - 1);
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##filter", filterBuf, sizeof(filterBuf)))
            filter = filterBuf;
        if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere(-1);

        ImGui::Separator();

        auto files = AssetManager::scanAssetFiles(extensions);

        std::regex filterRegex;
        bool useRegex = false;
        if (!filter.empty())
        {
            try
            {
                filterRegex = std::regex(filter, std::regex_constants::icase);
                useRegex = true;
            }
            catch (...) {}
        }

        for (const auto& relPath : files)
        {
            std::string fileName = std::filesystem::path(relPath).filename().string();

            if (!filter.empty())
            {
                if (useRegex)
                {
                    if (!std::regex_search(fileName, filterRegex)) continue;
                }
                else
                {
                    std::string fileLower = fileName, filterLower = filter;
                    for (auto& c : fileLower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                    for (auto& c : filterLower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                    if (fileLower.find(filterLower) == std::string::npos) continue;
                }
            }

            bool selected = (relPath == currentPath);
            if (ImGui::Selectable(fileName.c_str(), selected))
            {
                result.changed = true;
                result.path = relPath;
                result.assetName = std::filesystem::path(relPath).stem().string();
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", relPath.c_str());
            if (selected) ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    if (!currentPath.empty())
    {
        ImGui::SameLine();
        ImGui::PushID((std::string(label) + "_clear").c_str());
        if (ImGui::SmallButton("X"))
        {
            result.changed = true;
            result.path = "";
            result.assetName = "";
        }
        ImGui::PopID();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Clear asset");
    }

    return result;
}

static bool editVec2(const char* label, glm::vec2& v, float speed = 0.1f)
{
    float arr[2] = {v.x, v.y};
    if (ImGui::DragFloat2(label, arr, speed)) { v.x = arr[0]; v.y = arr[1]; return true; }
    return false;
}

static bool editVec3(const char* label, glm::vec3& v, float speed = 0.1f)
{
    float arr[3] = {v.x, v.y, v.z};
    if (ImGui::DragFloat3(label, arr, speed)) { v.x = arr[0]; v.y = arr[1]; v.z = arr[2]; return true; }
    return false;
}

static bool editVec4(const char* label, glm::vec4& v, float speed = 0.1f)
{
    float arr[4] = {v.x, v.y, v.z, v.w};
    if (ImGui::DragFloat4(label, arr, speed)) { v.x = arr[0]; v.y = arr[1]; v.z = arr[2]; v.w = arr[3]; return true; }
    return false;
}

static bool editColor3(const char* label, glm::vec3& c)
{
    float arr[3] = {c.x, c.y, c.z};
    if (ImGui::ColorEdit3(label, arr)) { c.x = arr[0]; c.y = arr[1]; c.z = arr[2]; return true; }
    return false;
}

static bool editColor4(const char* label, glm::vec4& c)
{
    float arr[4] = {c.x, c.y, c.z, c.w};
    if (ImGui::ColorEdit4(label, arr)) { c.x = arr[0]; c.y = arr[1]; c.z = arr[2]; c.w = arr[3]; return true; }
    return false;
}

static bool editPivot(const char* label, Sprout::Pivot& pivot)
{
    const char* names[] = {
        "TOP_LEFT", "TOP_CENTER", "TOP_RIGHT",
        "CENTER_LEFT", "CENTER", "CENTER_RIGHT",
        "BOTTOM_LEFT", "BOTTOM_CENTER", "BOTTOM_RIGHT"
    };
    int current = static_cast<int>(pivot);
    if (ImGui::Combo(label, &current, names, 9)) { pivot = static_cast<Sprout::Pivot>(current); return true; }
    return false;
}

static bool editLayer(const char* label, Comp::Layer& layer)
{
    const char* names[] = {"Background", "Midground", "Player", "Foreground", "UserInterface"};
    int current = static_cast<int>(layer);
    if (ImGui::Combo(label, &current, names, 5)) { layer = static_cast<Comp::Layer>(current); return true; }
    return false;
}

static bool editTextJustify(const char* label, Sprout::TextJustify& justify)
{
    const char* names[] = {"LEFT", "CENTER", "RIGHT"};
    int current = static_cast<int>(justify);
    if (ImGui::Combo(label, &current, names, 3)) { justify = static_cast<Sprout::TextJustify>(current); return true; }
    return false;
}

static bool inspectField(const Comp::FieldInfo& f, Comp::Component& comp)
{
    using Comp::FieldType;
    bool modified = false;
    const std::string id = std::string("##") + comp.componentName();

    switch (f.type)
    {
        case FieldType::Float:
            modified |= ImGui::DragFloat((std::string(f.displayName) + id).c_str(),
                                          &f.get<float>(comp), 0.1f);
            break;
        case FieldType::Int:
            modified |= ImGui::DragInt((std::string(f.displayName) + id).c_str(),
                                        &f.get<int>(comp));
            break;
        case FieldType::Int8:
        {
            int val = f.get<int8_t>(comp);
            if (ImGui::DragInt((std::string(f.displayName) + id).c_str(), &val, 1, -128, 127))
            {
                f.get<int8_t>(comp) = static_cast<int8_t>(val);
                modified = true;
            }
            break;
        }
        case FieldType::UInt8:
        {
            int val = f.get<uint8_t>(comp);
            if (ImGui::DragInt((std::string(f.displayName) + id).c_str(), &val, 1, 0, 255))
            {
                f.get<uint8_t>(comp) = static_cast<uint8_t>(val);
                modified = true;
            }
            break;
        }
        case FieldType::SizeT:
        {
            int val = static_cast<int>(f.get<size_t>(comp));
            if (ImGui::DragInt((std::string(f.displayName) + id).c_str(), &val, 1, 0, 100000))
            {
                f.get<size_t>(comp) = static_cast<size_t>(val);
                modified = true;
            }
            break;
        }
        case FieldType::Bool:
            modified |= ImGui::Checkbox((std::string(f.displayName) + id).c_str(),
                                          &f.get<bool>(comp));
            break;
        case FieldType::String:
        {
            auto& str = f.get<std::string>(comp);
            char buf[512];
            strncpy(buf, str.c_str(), sizeof(buf) - 1);
            buf[sizeof(buf) - 1] = '\0';
            if (ImGui::InputText((std::string(f.displayName) + id).c_str(), buf, sizeof(buf)))
            {
                str = buf;
                modified = true;
            }
            break;
        }
        case FieldType::Vec2:
            modified |= editVec2((std::string(f.displayName) + id).c_str(),
                                   f.get<glm::vec2>(comp));
            break;
        case FieldType::Vec3:
            modified |= editVec3((std::string(f.displayName) + id).c_str(),
                                   f.get<glm::vec3>(comp));
            break;
        case FieldType::Vec4:
            modified |= editVec4((std::string(f.displayName) + id).c_str(),
                                   f.get<glm::vec4>(comp));
            break;
        case FieldType::Color3:
            modified |= editColor3((std::string(f.displayName) + id).c_str(),
                                     f.get<glm::vec3>(comp));
            break;
        case FieldType::Color4:
            modified |= editColor4((std::string(f.displayName) + id).c_str(),
                                     f.get<glm::vec4>(comp));
            break;
        case FieldType::Layer:
            modified |= editLayer((std::string(f.displayName) + id).c_str(),
                                    f.get<Comp::Layer>(comp));
            break;
        case FieldType::Pivot:
            modified |= editPivot((std::string(f.displayName) + id).c_str(),
                                    f.get<Sprout::Pivot>(comp));
            break;
        case FieldType::TextJustify:
            modified |= editTextJustify((std::string(f.displayName) + id).c_str(),
                                          f.get<Sprout::TextJustify>(comp));
            break;
        case FieldType::ShaderType:
            // lazy to add this
            break;
        case FieldType::TextureRef:
        {
            auto& tex = f.get<std::shared_ptr<Sprout::Texture>>(comp);
            std::string currentName;
            if (tex)
            {
                currentName = AssetManager::getTextureName(tex);
                if (currentName.empty()) currentName = AssetManager::getImageTextureName(tex);
            }
            std::string currentPath;
            if (!currentName.empty())
            {
                currentPath = AssetManager::getTexturePath(currentName);
                if (currentPath.empty()) currentPath = AssetManager::getImageTexturePath(currentName);
            }
            auto pick = assetFilePicker((std::string(f.displayName) + id).c_str(), currentPath,
                                         {".png", ".jpg", ".jpeg", ".bmp", ".tga"});
            if (pick.changed)
            {
                if (pick.path.empty())
                {
                    tex = nullptr;
                    modified = true;
                }
                else
                {
                    try
                    {
                        tex = AssetManager::ensureImageTexture(pick.assetName, pick.path);
                        modified = true;
                    }
                    catch (...) {}
                }
            }
            break;
        }
        case FieldType::ImageTextureRef:
        {
            auto& tex = f.get<std::shared_ptr<Sprout::Texture>>(comp);
            std::string currentName = tex ? AssetManager::getImageTextureName(tex) : "";
            std::string currentPath = currentName.empty() ? "" : AssetManager::getImageTexturePath(currentName);
            auto pick = assetFilePicker((std::string(f.displayName) + id).c_str(), currentPath,
                                         {".png", ".jpg", ".jpeg", ".bmp", ".tga"});
            if (pick.changed)
            {
                if (pick.path.empty())
                {
                    tex = nullptr;
                    modified = true;
                }
                else
                {
                    try
                    {
                        tex = AssetManager::ensureImageTexture(pick.assetName, pick.path);
                        modified = true;
                    }
                    catch (...) {}
                }
            }
            break;
        }
        case FieldType::MeshRef:
        {
            auto& mesh = f.get<std::shared_ptr<Sprout::Mesh>>(comp);
            std::string currentName = mesh ? AssetManager::getMeshName(mesh) : "";
            std::string currentPath = currentName.empty() ? "" : AssetManager::getMeshPath(currentName);
            auto pick = assetFilePicker((std::string(f.displayName) + id).c_str(), currentPath,
                                         {".obj", ".fbx"});
            if (pick.changed)
            {
                if (pick.path.empty())
                {
                    mesh = nullptr;
                    modified = true;
                }
                else
                {
                    try
                    {
                        mesh = AssetManager::ensureMesh(pick.assetName, pick.path);
                        modified = true;
                    }
                    catch (...) {}
                }
            }
            break;
        }
        case FieldType::SkeletonRef:
        {
            auto& skel = f.get<std::shared_ptr<Sprout::Skeleton>>(comp);
            std::string currentName = skel ? AssetManager::getSkeletonName(skel) : "";
            std::string currentPath = currentName.empty() ? "" : AssetManager::getSkeletonPath(currentName);
            auto pick = assetFilePicker((std::string(f.displayName) + id).c_str(), currentPath, {".fbx"});
            if (pick.changed)
            {
                if (pick.path.empty())
                {
                    skel = nullptr;
                    modified = true;
                }
                else
                {
                    try
                    {
                        skel = AssetManager::ensureSkeleton(pick.assetName, pick.path);
                        modified = true;
                    }
                    catch (...) {}
                }
            }
            break;
        }
        case FieldType::AnimClipRef:
        {
            auto& clip = f.get<std::shared_ptr<Sprout::AnimationClip>>(comp);
            std::string currentName = clip ? AssetManager::getAnimationClipName(clip) : "";
            std::string currentPath = currentName.empty() ? "" : AssetManager::getAnimationClipPath(currentName);
            auto pick = assetFilePicker((std::string(f.displayName) + id).c_str(), currentPath, {".fbx"});
            if (pick.changed)
            {
                if (pick.path.empty())
                {
                    clip = nullptr;
                    modified = true;
                }
                else
                {
                    try
                    {
                        clip = AssetManager::ensureAnimationClip(pick.assetName, pick.path, "");
                        modified = true;
                    }
                    catch (...) {}
                }
            }
            break;
        }
        case FieldType::MaterialRef:
        {
            auto& mat = f.get<std::shared_ptr<Sprout::Material>>(comp);
            if (!mat)
            {
                mat = std::make_shared<Sprout::Material>();
                mat->create(Sprout::ShaderType::Mesh3D);
                modified = true;
            }
            ImGui::Text("Material:");
            ImGui::Indent();
            modified |= editColor4(("Base Color" + id + "_mat").c_str(), mat->properties.baseColor);
            modified |= ImGui::DragFloat(("Specular" + id + "_mat").c_str(),
                                          &mat->properties.specularStrength, 0.01f, 0.0f, 1.0f);

            std::string diffuseName;
            if (mat->diffuseTexture) diffuseName = AssetManager::getImageTextureName(mat->diffuseTexture);
            std::string diffusePath = diffuseName.empty() ? "" : AssetManager::getImageTexturePath(diffuseName);
            auto diffusePick = assetFilePicker(("Diffuse" + id + "_mat_diff").c_str(), diffusePath,
                                                {".png", ".jpg", ".jpeg", ".bmp", ".tga"});
            if (diffusePick.changed)
            {
                if (diffusePick.path.empty())
                {
                    mat->diffuseTexture = nullptr;
                    mat->create(mat->getShaderType());
                    modified = true;
                }
                else
                {
                    try
                    {
                        auto tex = AssetManager::ensureImageTexture(diffusePick.assetName, diffusePick.path);
                        mat->create(mat->getShaderType());
                        mat->diffuseTexture = tex;
                        modified = true;
                    }
                    catch (...) {}
                }
            }
            ImGui::Unindent();
            break;
        }
    }
    return modified;
}

bool Comp::Component::inspect()
{
    bool modified = false;
    auto fields = getFields();
    if (fields.empty()) return false;

    if (ImGui::CollapsingHeader(componentName(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (const auto& f : fields)
            modified |= inspectField(f, *this);
    }
    return modified;
}

bool Comp::Sprite::inspect()
{
    bool modified = false;
    if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen))
    {
        std::string currentName;
        if (texture)
        {
            currentName = AssetManager::getTextureName(texture);
            if (currentName.empty()) currentName = AssetManager::getImageTextureName(texture);
        }
        std::string currentPath;
        if (!currentName.empty())
        {
            currentPath = AssetManager::getTexturePath(currentName);
            if (currentPath.empty()) currentPath = AssetManager::getImageTexturePath(currentName);
        }

        auto pick = assetFilePicker("Texture##spr", currentPath, {".png", ".jpg", ".jpeg", ".bmp", ".tga"});
        if (pick.changed)
        {
            if (pick.path.empty())
            {
                texture = nullptr;
                modified = true;
            }
            else
            {
                try
                {
                    texture = AssetManager::ensureImageTexture(pick.assetName, pick.path);
                    if (texture)
                    {
                        glm::i32 x = texture->getWidth() / texture->getNumFrames();
                        glm::i32 y = texture->getHeight();
                        size = glm::vec2(x, y);
                        numFrames = texture->getNumFrames();
                    }
                    modified = true;
                }
                catch (...) {}
            }
        }
        if (!currentName.empty())
            ImGui::LabelText("Asset Name##spr", "%s", currentName.c_str());

        modified |= editLayer("Layer##spr", layer);
        bool animated = (type == Comp::Sprite::Type::Animated);
        if (ImGui::Checkbox("Animated##spr", &animated))
        {
            type = animated ? Comp::Sprite::Type::Animated : Comp::Sprite::Type::Static;
            modified = true;
        }
        if (animated)
        {
            int speed = static_cast<int>(animationSpeed);
            if (ImGui::DragInt("Anim Speed##spr", &speed, 1, 1, 120))
            {
                animationSpeed = static_cast<size_t>(speed);
                modified = true;
            }
        }
        modified |= ImGui::Checkbox("Flip X##spr", &flip_X);
        modified |= editVec2("Offset##spr", transformOffset);
        modified |= editVec3("Scale Offset##spr", scaleOffset, 0.01f);
        modified |= ImGui::DragFloat("Pixels Per Unit##spr", &pixelsPerUnit, 0.1f, 0.1f, 1000.0f);
    }
    return modified;
}



bool Comp::Text::inspect()
{
    bool modified = false;
    if (ImGui::CollapsingHeader("Text", ImGuiTreeNodeFlags_DefaultOpen))
    {
        char textBuf[512];
        strncpy(textBuf, text.c_str(), sizeof(textBuf) - 1);
        textBuf[sizeof(textBuf) - 1] = '\0';
        if (ImGui::InputText("Text##txt", textBuf, sizeof(textBuf)))
        {
            text = textBuf;
            modified = true;
        }

        std::string fontDisplay = font.empty() ? "(none)" : font;
        if (ImGui::BeginCombo("Font##txt", fontDisplay.c_str()))
        {
            auto fontNames = AssetManager::getFontNames();
            for (const auto& fname : fontNames)
            {
                bool selected = (fname == font);
                if (ImGui::Selectable(fname.c_str(), selected))
                {
                    font = fname;
                    modified = true;
                }
                if (selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        int sz = this->size;
        if (ImGui::DragInt("Size##txt", &sz, 1, 1, 255))
        {
            this->size = static_cast<uint8_t>(sz);
            modified = true;
        }

        modified |= editColor4("Color##txt", color);
        modified |= editTextJustify("Justify##txt", justify);
        modified |= editLayer("Layer##txt", layer);
        modified |= editVec2("Offset##txt", transformOffset);
    }
    return modified;
}

bool Comp::Transform::inspect()
{
    bool modified = false;
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        modified |= editVec3("Position##xform", position);
        modified |= editVec3("Velocity##xform", velocity);
        modified |= editVec3("Rotation##xform", rotation, 0.01f);
        modified |= editVec3("Scale##xform", scale, 0.01f);
        modified |= editPivot("Pivot##xform", pivot);
        modified |= ImGui::Checkbox("Screen Space##xform", &screenSpace);

        // Hierarchy
        if (parent || !children.empty())
        {
            ImGui::Separator();
            ImGui::Text("Hierarchy");
            std::string parentName = parent ? parent->getName() : "(none)";
            ImGui::LabelText("Parent", "%s", parentName.c_str());
            ImGui::LabelText("Children", "%zu", children.size());
        }
    }
    return modified;
}

bool Comp::MeshRenderer::inspect()
{
    bool modified = false;
    if (ImGui::CollapsingHeader("MeshRenderer", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (!material)
        {
            material = std::make_shared<Sprout::Material>();
            material->create(Sprout::ShaderType::Mesh3D);
            modified = true;
        }

        std::string currentMeshName = mesh ? AssetManager::getMeshName(mesh) : "";
        std::string currentMeshPath = currentMeshName.empty() ? "" : AssetManager::getMeshPath(currentMeshName);

        auto meshPick = assetFilePicker("Mesh##mr", currentMeshPath, {".obj", ".fbx"});
        if (meshPick.changed)
        {
            if (meshPick.path.empty())
            {
                mesh = nullptr;
                modified = true;
            }
            else
            {
                try
                {
                    mesh = AssetManager::ensureMesh(meshPick.assetName, meshPick.path);
                    modified = true;
                }
                catch (...) {}
            }
        }
        if (!currentMeshName.empty())
            ImGui::LabelText("Asset Name##mr_mesh", "%s", currentMeshName.c_str());

        ImGui::Text("Material:");
        ImGui::Indent();
        modified |= editColor4("Base Color##mat", material->properties.baseColor);
        modified |= ImGui::DragFloat("Specular##mat", &material->properties.specularStrength, 0.01f, 0.0f, 1.0f);

        std::string diffuseName;
        if (material->diffuseTexture) diffuseName = AssetManager::getImageTextureName(material->diffuseTexture);
        std::string diffusePath = diffuseName.empty() ? "" : AssetManager::getImageTexturePath(diffuseName);

        auto diffusePick = assetFilePicker("Diffuse##mat_diff", diffusePath, {".png", ".jpg", ".jpeg", ".bmp", ".tga"});
        if (diffusePick.changed)
        {
            if (diffusePick.path.empty())
            {
                material->diffuseTexture = nullptr;
                material->create(material->getShaderType());
                modified = true;
            }
            else
            {
                try
                {
                    auto tex = AssetManager::ensureImageTexture(diffusePick.assetName, diffusePick.path);
                    material->create(material->getShaderType());
                    material->diffuseTexture = tex;
                    modified = true;
                }
                catch (...) {}
            }
        }
        if (!diffuseName.empty())
            ImGui::LabelText("Asset Name##mat_diffname", "%s", diffuseName.c_str());
        ImGui::Unindent();

        const char* shaderItems[] = {"Auto", "Mesh3D"};
        int shaderIdx = 0;
        if (shaderOverride == Sprout::ShaderType::Mesh3D) shaderIdx = 1;
        if (ImGui::Combo("Shader Override##mr", &shaderIdx, shaderItems, 2))
        {
            if (shaderIdx == 1) shaderOverride = Sprout::ShaderType::Mesh3D;
            else shaderOverride = Sprout::ShaderType::Custom;
            modified = true;
        }

        modified |= ImGui::Checkbox("Cast Shadow##mr", &castShadow);
        modified |= ImGui::Checkbox("Receive Shadow##mr", &receiveShadow);
        modified |= ImGui::Checkbox("Depth Test##mr", &depthTest);
        modified |= ImGui::Checkbox("Depth Write##mr", &depthWrite);
        modified |= ImGui::Checkbox("Double Sided##mr", &doubleSided);

        int blendIdx = static_cast<int>(blendMode);
        const char* blendItems[] = {"Alpha", "Opaque"};
        if (ImGui::Combo("Blend Mode##mr", &blendIdx, blendItems, 2))
        {
            blendMode = static_cast<uint8_t>(blendIdx);
            modified = true;
        }

        int renderLayerValue = static_cast<int>(renderLayer);
        if (ImGui::InputInt("Render Layer##mr", &renderLayerValue))
        {
            if (renderLayerValue < 0) renderLayerValue = 0;
            renderLayer = static_cast<size_t>(renderLayerValue);
            modified = true;
        }
    }
    return modified;
}

bool Comp::Camera::inspect()
{
    bool modified = false;
    if (ImGui::CollapsingHeader("Camera"))
    {
        const char* projNames[] = {"Perspective", "Orthographic"};
        int projIdx = (projectionType == Comp::Camera::Projection::Orthographic) ? 1 : 0;
        if (ImGui::Combo("Projection##cam", &projIdx, projNames, 2))
        {
            projectionType = (projIdx == 1) ? Comp::Camera::Projection::Orthographic : Comp::Camera::Projection::Perspective;
            modified = true;
        }
        modified |= ImGui::DragFloat("FOV##cam", &fov, 1.0f, 1.0f, 179.0f);
        modified |= ImGui::DragFloat("Near##cam", &nearPlane, 0.01f, 0.001f, 100.0f);
        modified |= ImGui::DragFloat("Far##cam", &farPlane, 1.0f, 1.0f, 10000.0f);
        modified |= ImGui::DragFloat("Ortho Size##cam", &orthoSize, 0.1f, 0.1f, 1000.0f);
        modified |= ImGui::Checkbox("Active##cam", &isActive);
    }
    return modified;
}

bool Comp::Light::inspect()
{
    bool modified = false;
    if (ImGui::CollapsingHeader("Light"))
    {
        const char* typeNames[] = {"Directional", "Point"};
        int typeIdx = (type == Comp::Light::Type::Point) ? 1 : 0;
        if (ImGui::Combo("Type##light", &typeIdx, typeNames, 2))
        {
            type = (typeIdx == 1) ? Comp::Light::Type::Point : Comp::Light::Type::Directional;
            modified = true;
        }
        modified |= editColor3("Color##light", color);
        modified |= ImGui::DragFloat("Intensity##light", &intensity, 0.01f, 0.0f, 100.0f);
        if (type == Comp::Light::Type::Point)
            modified |= ImGui::DragFloat("Range##light", &range, 0.1f, 0.0f, 1000.0f);
    }
    return modified;
}

bool Comp::Animator::inspect()
{
    bool modified = false;
    if (ImGui::CollapsingHeader("Animator", ImGuiTreeNodeFlags_DefaultOpen))
    {
        std::string currentSkelName = skeleton ? AssetManager::getSkeletonName(skeleton) : "";
        std::string currentSkelPath = currentSkelName.empty() ? "" : AssetManager::getSkeletonPath(currentSkelName);

        auto skelPick = assetFilePicker("Skeleton##anim", currentSkelPath, {".fbx"});
        if (skelPick.changed)
        {
            if (skelPick.path.empty())
            {
                skeleton = nullptr;
                boneMatrices.clear();
                currentSkelName = "";
                modified = true;
            }
            else
            {
                try
                {
                    skeleton = AssetManager::ensureSkeleton(skelPick.assetName, skelPick.path);
                    if (skeleton) boneMatrices.resize(skeleton->bones.size(), glm::mat4(1.0f));
                    currentSkelName = skelPick.assetName;
                    modified = true;
                }
                catch (const std::exception& e)
                {
                    Logger::error("Failed to load skeleton: " + std::string(e.what()));
                }
            }
        }
        if (!currentSkelName.empty())
            ImGui::LabelText("Asset Name##anim_skel", "%s", currentSkelName.c_str());

        std::string currentClipName = currentClip ? AssetManager::getAnimationClipName(currentClip) : "";
        std::string currentClipPath = currentClipName.empty() ? "" : AssetManager::getAnimationClipPath(currentClipName);

        auto clipPick = assetFilePicker("Clip##anim", currentClipPath, {".fbx"});
        if (clipPick.changed)
        {
            if (clipPick.path.empty())
            {
                stop();
                currentClip = nullptr;
                currentTime = 0.0f;
                modified = true;
            }
            else if (!currentSkelName.empty())
            {
                try
                {
                    auto clip = AssetManager::ensureAnimationClip(clipPick.assetName, clipPick.path, currentSkelName);
                    play(clip, looping);
                    modified = true;
                }
                catch (const std::exception& e)
                {
                    Logger::error("Failed to load animation clip: " + std::string(e.what()));
                }
            }
        }
        if (!currentClipName.empty())
            ImGui::LabelText("Asset Name##anim_clip", "%s", currentClipName.c_str());

        if (isPlaying)
        {
            if (ImGui::Button("Pause##anim")) { isPlaying = false; }
        }
        else
        {
            if (ImGui::Button("Play##anim")) { isPlaying = true; }
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop##anim"))
        {
            stop();
            currentTime = 0.0f;
        }

        if (currentClip)
        {
            float duration = currentClip->duration;
            if (duration > 0.0f)
            {
                if (ImGui::SliderFloat("Time##anim", &currentTime, 0.0f, duration, "%.2f s"))
                {
                    if (skeleton)
                        currentClip->sample(currentTime, boneMatrices, *skeleton);
                }
            }
        }

        modified |= ImGui::Checkbox("Looping##anim", &looping);
        modified |= ImGui::DragFloat("Speed##anim", &playbackSpeed, 0.01f, 0.0f, 10.0f);
    }
    return modified;
}

static bool renderRemoveButton(const char* componentName)
{
    ImGui::SameLine(ImGui::GetContentRegionAvail().x + ImGui::GetCursorPosX() - 20.0f);
    ImGui::PushID(componentName);
    bool remove = ImGui::SmallButton("X");
    ImGui::PopID();
    return remove;
}

InspectorResult ComponentInspector::renderAllComponents(const std::shared_ptr<Entity>& entity)
{
    InspectorResult result;

    for (const auto& [typeIdx, comp] : entity->getComponents())
    {
        result.modified |= comp->inspect();
        if (renderRemoveButton(comp->componentName()))
            result.removeComponent = comp->componentName();
    }

    return result;
}

void ComponentInspector::serializeAllComponents(const std::shared_ptr<Entity>& entity, nlohmann::json& out)
{
    for (const auto& [typeIdx, comp] : entity->getComponents())
        out[comp->componentName()] = comp->serialize();
}

#endif // SAPLING_HAS_EDITOR
