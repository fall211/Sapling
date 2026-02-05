//
//  Debug.hpp
//  SaplingEngine
//

#pragma once

#include "Renderer/Sprout.hpp"
#include "Core/AssetManager.hpp"

#include <iostream>

class Debug {
    public:
    
        /*
            * Logs a message to the console
            * @param message The message to print.
        */
        template<typename T>
        static void log(const T& message) 
        {
            #ifndef DEBUG
            return;
            #endif
            std::cout << message << std::endl;
        }
        static void draw_pos(glm::vec2 pos, glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f))
        {
            // draw a little dot at the position for debugging
            #ifndef DEBUG
            return;
            #endif
            Sprout::Window::getInstance()->draw_sprite(AssetManager::getTexture("debugCircle"), pos, 1.0f, 0.0f, 1, color);
        }
        
        static void draw_rectangle(float x, float y, float width, float height, glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 0.25f))
        {
            #ifndef DEBUG
            return;
            #endif
            // draw a rectangle at the position for debugging
            Sprout::Window::getInstance()->draw_rectangle(x, y, width, height, AssetManager::getTexture("debugRect"), color);
        }
};

