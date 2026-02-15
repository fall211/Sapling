//
//  Scene.hpp
//  SaplingEngine, Canopy Scene Manager
//

#pragma once

#include "Core/Engine.hpp"
#include "Core/Input.hpp"
#include "Core/AssetManager.hpp"
#include "Core/AudioEngine.hpp"
#include "Utility/Debug.hpp"
#include "Utility/Physics.hpp"
#include "ECS/EntityManager.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Component.hpp"

#include <cstdio>
#include <memory>
#include <random>
#include <cstddef>
#include <string>
#include <functional>
#include "Core/SceneMessage.hpp"

class Entity;
class Input;
class EntityManager;
class Engine;



typedef std::vector<std::shared_ptr<Entity>> EntityList;


class Scene
{
    protected:
        std::shared_ptr<EntityManager> m_entityManager;
        Engine& m_engine;


    public:
        explicit Scene(Engine& engine);
        virtual ~Scene()= default;

        /*
            * Called when the scene is first created.
            * Override this function to initialize the scene
        */
        virtual void init() = 0;

        /*
            * Called every frame.
            * Override this function to update the scene
        */
        virtual void update() = 0;

        /*
            * Called every frame to render entities
            * Override this function to implement custom rendering, by default it renders all entities with a sprite component (static and animated)
            * @param entities The list of entities to render
        */
        virtual void sRender(EntityList& entities);

        /*
            * Called every frame to render 3D entities
            * Override this function to implement custom 3D rendering
            * By default it finds the active camera, collects lights, and submits mesh draw requests
            * @param entities The list of entities to render
        */
        virtual void sRender3D(EntityList& entities);

        /*
            * Sets up 3D render state: finds active camera, computes projection/view matrices,
            * extracts frustum, and collects lights. Override to customize camera/projection setup
            * (e.g. editor viewport with different aspect ratio).
            * @param entities The list of entities to search for camera and lights
        */
        virtual void setupRenderState(EntityList& entities);

        /*
            * Unified render: sets up render state, then iterates all entities once
            * to submit both 3D meshes and 2D sprites/text/images.
            * Override for custom render ordering.
        */
        virtual void render();

        virtual void onSceneEnabled();
        virtual void onSceneDisabled();

        /*
            * Processes a message sent from the Engine
            * Override this function to handle custom messages
            * @param message The message to process
            * @return True if the message was handled, false otherwise
        */
        virtual bool onMessage(const SceneMessage& /*message*/) { return false; }

        /*
            * Called when the scene is switched to
        */
        void enable();

        /*
            * Called when the scene is switched from
        */
        void disable();

        /*
            * Updates all animation state (2D sprite frames, color overrides, 3D skeletal animation).
            * Called automatically from preUpdate(). Can also be called manually.
        */
        void sUpdateAnimations(EntityList& entities);

        /*
            * Called before each update loop, reserved for updates that are necessary for all scenes
        */
        void preUpdate();

        /*
            * Called after each update loop, reserved for updates that are necessary for all scenes
        */
        void postUpdate();


};
