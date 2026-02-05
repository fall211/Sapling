//
//  Engine.hpp
//  SaplingEngine
//

#pragma once

#include "Renderer/Sprout.hpp"
#include "Core/Scene.hpp"
#include "Core/AssetManager.hpp"
#include "Core/SceneMessage.hpp"
#include "Utility/Debug.hpp"

#include <memory>
#include <string>


class Scene;
class AssetManager;

using namespace std::chrono;

using sceneMap = std::unordered_map<std::string, std::shared_ptr<Scene>>;

class Engine {
    Sprout::Window m_window;// = Sprout::Window(640, 360, "Sapling Engine");
    float m_deltaTime = 0.0f;

    sceneMap m_scenes;
    std::shared_ptr<Scene> m_currentScene = nullptr;
    

    size_t m_currentFrame = 0;

public:

    Engine(size_t viewportWidth, size_t viewportHeight, const char* title);
    ~Engine() = default;
    
    /**
     * The main loop of the game. Nothing after this function gets called.
     */
    void run();
    
    /**
     * The main update function of the game, called each frame. Must be set as callback to the window.
    */
    void update(double dt);
    
    /**
     * Gets the current frame of the simulation.
     *
     * @return The current frame.
     */
    auto getCurrentFrame() const -> size_t { return m_currentFrame; }

    /**
     * Gets the delta time of the simaltion.
     * @return delta time
     */
    auto deltaTime() const -> float { return m_deltaTime; }

    /**
     * Gets the Sprout window.
     *
     * @return The Sorout window.
     */
    auto getWindow() -> Sprout::Window& { return m_window; }


    /**
     * Makes a scene and adds it to the engine simulation.
     *
     * @param name  The name of the new scene
     * @param ptr  Pointer to the scene
     */
    void makeScene(const std::string& name, std::shared_ptr<Scene> ptr);
    
    /**
     * Adds a scene to the engine simulation.
     *
     * @tparam SceneType  The type of the scene
     * @param name  The name of the scene
     */
    template <typename SceneType>
    void newScene(const std::string& name)
    {
        makeScene(name, std::make_shared<SceneType>(*this));
    }
        
    void changeScene(const std::string& name);
    auto getScene(const std::string& name) -> std::shared_ptr<Scene>;
    auto getCurrentScene() -> std::shared_ptr<Scene>&;

    /**
     * Sends a message to the current scene
     *
     * @tparam T The type of data to send
     * @param type The type of the message
     * @param data The data to send
     * @return True if the message was handled by the scene, false otherwise
     */
    template <typename T>
    bool sendToCurrentScene(const std::string& type, const T& data);
    
    /**
     * Sends a message to a specific scene by name
     *
     * @tparam T The type of data to send
     * @param sceneName The name of the scene to send the message to
     * @param type The type of the message
     * @param data The data to send
     * @return True if the message was handled by the scene, false otherwise
     */
    template <typename T>
    bool sendToScene(const std::string& sceneName, const std::string& type, const T& data);
    
};



