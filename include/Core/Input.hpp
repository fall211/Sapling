//
//  Input.hpp
//  SaplingEngine, sokol Input Wrapper
//

#pragma once

#include "glm/glm.hpp"
#include "sokol/sokol_app.h"

#include <cstdint>
#include <string>
#include <map>
#include <memory>
#include <array>

struct Key
{
    bool justPressed = false;
    bool justReleased = false;
    bool pressed = false;
    
    Key() = default;
};

struct InputAxis
{
    std::string name;
    int postiveKey;
    int negativeKey;
    
    InputAxis(std::string  name, int pKey, int nKey);
};


using ActionsMap = std::map<std::string, std::vector<int>>;
using AxisMap = std::map<std::string, std::shared_ptr<InputAxis>>;
using KeyMap = std::map<int, std::shared_ptr<Key>>;

/*
 TODO: support mouse inputs
 TODO: support controller inputs
 */
class Input 
{
    static Input* Instance;
    
    private:
        ActionsMap m_actionsMap;
        AxisMap m_axisMap;
        KeyMap m_keyMap;
        
        glm::vec2 m_mousePosition;
        
        static auto getKey(int key) -> bool;
        static auto getKeyDown(int key) -> bool;
        static auto getKeyUp(int key) -> bool;
    
        Input();
        ~Input();
    public:
        static Input* getInstance()
        {
            if (Instance == nullptr) {
                Instance = new Input();
            }
            return Instance;
        }
        static void initialize();
        static void cleanUp();
        
        /*
            * Updates the input state. This is set as the event callback for the window by the owning scene.
        */
        static void update(const sapp_event * event);
        
        /*
            * Resets the justPressed and justReleased flags for all keys and mouse buttons. This prevents the flags from carrying over to the next frame.
        */
        static void clean();
        
        /*
            * Creates a new action with the given name and keycodes. 
            * @param name The name of the action
            * @param keycodes A vector of keycodes that will trigger the action
        */
        static void makeAction(const std::string& name, const std::vector<int>& keycodes);
        
        /*
            * Checks if the action with the given name is currently active
            * @param name The name of the action
            * @return True if the action is active
        */
        static auto isAction(const std::string& name) -> bool;
        
        /*
            * Checks if the action with the given name was just activated
            * @param name The name of the action
            * @return True if the action was just activated
        */
        static auto isActionDown(const std::string& name) -> bool;
        
        /*
            * Checks if the action with the given name was just deactivated
            * @param name The name of the action
            * @return True if the action was just deactivated
        */
        static auto isActionUp(const std::string& name) -> bool;
    
        /*
            * Creates a new axis with the given name and keycodes. 
            * @param name The name of the axis
            * @param positiveKey The keycode that will provide positive input
            * @param negativeKey The keycode that will provide negative input
        */
        static void makeAxis(const std::string& name, int positiveKey, int negativeKey);
        
        /*
            * Gets the value of the axis with the given name
            * @param name The name of the axis
            * @return The value of the axis (-1, 0, 1)
        */
        static auto getAxis(const std::string& name) -> float;
        
        enum MouseButton : std::uint8_t
        {
            LEFT = 0,
            RIGHT = 1,
            MIDDLE = 2,
            COUNT
        };
        
        /*
            * Checks if the given mouse button was just pressed
            * @param button The mouse button to check
            * @return True if the button was just pressed
        */
        static auto getMouseDown(Input::MouseButton button) -> bool;
        
        /*
            * Checks if the given mouse button was just released
            * @param button The mouse button to check
            * @return True if the button was just released
        */
        static auto getMouseUp(Input::MouseButton button) -> bool;
        
        /*
            * Checks if the given mouse button is currently pressed
            * @param button The mouse button to check
            * @return True if the button is currently pressed
        */
        static auto getMouse(Input::MouseButton button) -> bool;
        
        /*
            * Gets the current mouse position
            * @return The current mouse position relative to the window
        */
        static auto getMousePosition() -> glm::vec2;
        
        /*
            * Gets the current mouse position in world coordinates
            * @return The current mouse position in world coordinates
        */
        static auto getMouseWorldPosition() -> glm::vec2;
        

    private:
        std::array<Key, static_cast<size_t>(Input::MouseButton::COUNT)> m_mouseKeys;
        
        static auto getMouseKey(Input::MouseButton button) -> Key&;

};



