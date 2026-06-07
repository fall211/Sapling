//
//  Input.cpp
//  SaplingEngine, sokol Input Wrapper
//

#include "Core/Input.hpp"
#include "Core/Logger.hpp"
#include "Core/ManifestLoader.hpp"
#include "Renderer/Sprout.hpp"

#include <nlohmann/json.hpp>

#include <stdexcept>
#include <unordered_map>

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

    Logger::info("Input init completed");
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

static auto keyCodeFromManifestName(const std::string& keyName, int& keyCode) -> bool
{
    static const std::unordered_map<std::string, int> keyCodes =
    {
        {"LEFT", SAPP_KEYCODE_LEFT},
        {"RIGHT", SAPP_KEYCODE_RIGHT},
        {"UP", SAPP_KEYCODE_UP},
        {"DOWN", SAPP_KEYCODE_DOWN},
        {"A", SAPP_KEYCODE_A},
        {"D", SAPP_KEYCODE_D},
        {"W", SAPP_KEYCODE_W},
        {"S", SAPP_KEYCODE_S},
        {"SPACE", SAPP_KEYCODE_SPACE},
        {"ENTER", SAPP_KEYCODE_ENTER},
        {"ESCAPE", SAPP_KEYCODE_ESCAPE},
    };

    const auto keyIt = keyCodes.find(keyName);
    if (keyIt == keyCodes.end())
    {
        return false;
    }

    keyCode = keyIt->second;
    return true;
}

void Input::loadManifest(const std::string& manifestPath)
{
    const auto manifestJson = ManifestLoader::loadJson(manifestPath);
    loadManifest(manifestJson, manifestPath);
}

void Input::loadManifest(const nlohmann::json& manifestJson, const std::string& sourceName)
{
    getInstance();

    if (!manifestJson.is_object() || !manifestJson.contains("input"))
    {
        return;
    }

    const auto& input = manifestJson["input"];
    if (!input.is_object())
    {
        Logger::error("Input: " + sourceName + " field 'input' must be an object");
        return;
    }

    if (input.contains("actions"))
    {
        if (!input["actions"].is_array())
        {
            Logger::error("Input: " + sourceName + " field 'input.actions' must be an array");
        }
        else
        {
            for (const auto& action : input["actions"])
            {
                if (!action.is_object())
                {
                    Logger::error("Input: " + sourceName + " has an action entry that is not an object");
                    continue;
                }

                if (!action.contains("name") || !action["name"].is_string() || action["name"].get<std::string>().empty())
                {
                    Logger::error("Input: " + sourceName + " has action entry without required string field 'name'");
                    continue;
                }

                const std::string actionName = action["name"].get<std::string>();
                if (!action.contains("keys") || !action["keys"].is_array())
                {
                    Logger::error("Input: " + sourceName + " action '" + actionName + "' requires array field 'keys'");
                    continue;
                }

                std::vector<int> keyCodes;
                for (const auto& keyNameJson : action["keys"])
                {
                    if (!keyNameJson.is_string())
                    {
                        Logger::error("Input: " + sourceName + " action '" + actionName + "' has a non-string key entry");
                        continue;
                    }

                    int keyCode = 0;
                    const std::string keyName = keyNameJson.get<std::string>();
                    if (!keyCodeFromManifestName(keyName, keyCode))
                    {
                        Logger::error("Input: " + sourceName + " action '" + actionName + "' has unknown key '" + keyName + "'");
                        continue;
                    }

                    keyCodes.push_back(keyCode);
                }

                if (keyCodes.empty())
                {
                    Logger::error("Input: " + sourceName + " action '" + actionName + "' has no valid keys");
                    continue;
                }

                makeAction(actionName, keyCodes);
            }
        }
    }

    if (input.contains("axes"))
    {
        if (!input["axes"].is_array())
        {
            Logger::error("Input: " + sourceName + " field 'input.axes' must be an array");
        }
        else
        {
            for (const auto& axis : input["axes"])
            {
                if (!axis.is_object())
                {
                    Logger::error("Input: " + sourceName + " has an axis entry that is not an object");
                    continue;
                }

                if (!axis.contains("name") || !axis["name"].is_string() || axis["name"].get<std::string>().empty())
                {
                    Logger::error("Input: " + sourceName + " has axis entry without required string field 'name'");
                    continue;
                }

                const std::string axisName = axis["name"].get<std::string>();
                if (!axis.contains("positive") || !axis["positive"].is_string() ||
                    !axis.contains("negative") || !axis["negative"].is_string())
                {
                    Logger::error("Input: " + sourceName + " axis '" + axisName + "' requires string fields 'positive' and 'negative'");
                    continue;
                }

                int positiveKey = 0;
                int negativeKey = 0;
                const std::string positiveName = axis["positive"].get<std::string>();
                const std::string negativeName = axis["negative"].get<std::string>();

                if (!keyCodeFromManifestName(positiveName, positiveKey))
                {
                    Logger::error("Input: " + sourceName + " axis '" + axisName + "' has unknown positive key '" + positiveName + "'");
                    continue;
                }

                if (!keyCodeFromManifestName(negativeName, negativeKey))
                {
                    Logger::error("Input: " + sourceName + " axis '" + axisName + "' has unknown negative key '" + negativeName + "'");
                    continue;
                }

                makeAxis(axisName, positiveKey, negativeKey);
            }
        }
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
