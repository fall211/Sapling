//
//  Input.cpp
//  SaplingEngine, sokol Input Wrapper
//

#include "Core/Input.hpp"
#include "Renderer/Sprout.hpp"

#include <stdexcept>

Input* Input::Instance = nullptr;

InputAxis::InputAxis(std::string  name, const int pKey, const int nKey) 
    :   name(std::move(name)), 
        postiveKey(pKey), 
        negativeKey(nKey)
    {}



Input::Input()
{
    m_actionsMap = std::map<std::string, std::vector<int>>();
    m_axisMap = std::map<std::string, std::shared_ptr<InputAxis>>();
    m_keyMap = std::map<int, std::shared_ptr<Key>>();
    
    m_mousePosition = glm::vec2(0, 0);
    
    m_mouseKeys = std::array<Key, static_cast<size_t>(MouseButton::COUNT)>();
}

Input::~Input()
{
    cleanUp();
}

void Input::initialize()
{
    if (!Instance) Instance = new Input();
}

void Input::cleanUp()
{
    if (Instance)
    {
        delete Instance;
    }
}

void Input::clean()
{
    for (const auto& pair : Instance->m_keyMap)
    {
        const std::shared_ptr<Key> key = pair.second;
        key->justPressed = false;
        key->justReleased = false;
    }
    
    for (auto& key : Instance->m_mouseKeys)
    {
        key.justPressed = false;
        key.justReleased = false;
    }
}

void Input::update(const sapp_event * event)
{
    
    Instance->m_mousePosition = glm::vec2(event->mouse_x, event->mouse_y);    
    
    if ((event->type == SAPP_EVENTTYPE_KEY_DOWN || event->type == SAPP_EVENTTYPE_KEY_UP) && !event->key_repeat) 
    {
        // check if there is a key for this code in m_keyMap
        if (Instance->m_keyMap.count(event->key_code) == 0) return;

        const std::shared_ptr<Key> key = Instance->m_keyMap[event->key_code];
        
        if (event->type == SAPP_EVENTTYPE_KEY_DOWN)
        {
            key->justPressed = true;
            key->pressed = true;

        } 
        else 
        {
            key->justReleased = true;
            key->pressed = false;
            
        }

    }
    else if (event->type == SAPP_EVENTTYPE_MOUSE_DOWN) 
    {
        if (event->mouse_button >= Instance->m_mouseKeys.size()) return;
        Instance->m_mouseKeys[event->mouse_button].justPressed = true;
        Instance->m_mouseKeys[event->mouse_button].pressed = true;
    }
    else if (event->type == SAPP_EVENTTYPE_MOUSE_UP) 
    {
        if (event->mouse_button >= Instance->m_mouseKeys.size()) return;
        Instance->m_mouseKeys[event->mouse_button].justReleased = true;
        Instance->m_mouseKeys[event->mouse_button].pressed = false;
    }
}

auto Input::getKey(const int key) -> bool
{
    if (Instance->m_keyMap.count(key) == 0)
    {
        throw std::out_of_range("tried to get invalid key: " + std::to_string(key));
    }
    return Instance->m_keyMap[key]->pressed;
}
auto Input::getKeyDown(const int key) -> bool
{
    if (Instance->m_keyMap.count(key) == 0)
    {
        throw std::out_of_range("tried to get invalid key: " + std::to_string(key));
    }
    return Instance->m_keyMap[key]->justPressed;
}
auto Input::getKeyUp(const int key) -> bool
{
    if (Instance->m_keyMap.count(key) == 0)
    {
        throw std::out_of_range("tried to get invalid key: " + std::to_string(key));
    }
    return Instance->m_keyMap[key]->justReleased;
}

void Input::makeAction(const std::string& name, const std::vector<int>& keycodes)
{
    /*
     TODO: return if an action with the given name already exists
     */
    
    Instance->m_actionsMap.insert({name, keycodes});
    for (const auto& k : keycodes)
    {
        if (Instance->m_keyMap.count(k) == 0)
        {
            auto key = std::make_shared<Key>();
            Instance->m_keyMap.insert({k, key});
        }
    }
}

auto Input::isAction(const std::string& name) -> bool
{
    for (const auto& key : Instance->m_actionsMap[name]){
        if (getKey(key)) return true;
    }
    return false;
}


auto Input::isActionDown(const std::string& name) -> bool
{
    for (const auto& key : Instance->m_actionsMap[name]){
        if (getKeyDown(key)) {
            return true;
        }
    }
    return false;
}

auto Input::isActionUp(const std::string& name) -> bool
{
    for (const auto& key : Instance->m_actionsMap[name]){
        if (getKeyUp(key)) {
            return true;
        }
    }
    return false;
}

void Input::makeAxis(const std::string& name, const int positiveKey, const int negativeKey)
{
    /*
     TODO: allow multiple keys to provide positive/negative key input
     TODO: return if an axis with the given name already exists
     */
    
    Instance->m_axisMap.insert({name, std::make_shared<InputAxis>(name, positiveKey, negativeKey)});
    
    // check if the keys are registered already, if not, add them to m_keyMap
    if (Instance->m_keyMap.count(positiveKey) == 0)
    {
        auto key = std::make_shared<Key>();
        Instance->m_keyMap.insert({positiveKey, key});
    }
    if (Instance->m_keyMap.count(negativeKey) == 0)
    {
        auto key = std::make_shared<Key>();
        Instance->m_keyMap.insert({negativeKey, key});
    }

}

auto Input::getAxis(const std::string& name) -> float
{
    // check if axis exists
    if (Instance->m_axisMap.count(name) == 0)
    {
        throw std::out_of_range("tried to get invalid axis: " + name);
    }
    const std::shared_ptr<InputAxis> axis = Instance->m_axisMap[name];
    const int pos = getKey(axis->postiveKey) ? 1 : 0;
    const int neg = getKey(axis->negativeKey) ? 1 : 0;
    return pos - neg;
}

auto Input::getMouseKey(MouseButton button) -> Key&
{
    if (button >= Instance->m_mouseKeys.size())
    {
        throw std::out_of_range("tried to get invalid button: " + std::to_string(static_cast<int>(button)));
    }
    return Instance->m_mouseKeys[button];
}

auto Input::getMouseDown(MouseButton button) -> bool
{
    return getMouseKey(button).justPressed;
}

auto Input::getMouseUp(MouseButton button) -> bool
{
    return getMouseKey(button).justReleased;
}

auto Input::getMouse(MouseButton button) -> bool
{
    return getMouseKey(button).pressed;
}

auto Input::getMousePosition() -> glm::vec2
{
    return Instance->m_mousePosition;
}

auto Input::getMouseWorldPosition() -> glm::vec2
{
    return Sprout::Window::screenToWorld(Instance->m_mousePosition);
}

