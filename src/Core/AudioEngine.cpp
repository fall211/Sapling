//
//
//
//

#include "Core/AudioEngine.hpp"
#include "Core/AssetManager.hpp"


AudioEngine* AudioEngine::Instance = nullptr;


AudioEngine::AudioEngine() : m_system(nullptr), m_studioSystem(nullptr) {}

AudioEngine::~AudioEngine() {
    cleanUp();
}

void AudioEngine::initialize() {
    if (!Instance) Instance = new AudioEngine();

    // Initialize FMOD Studio System
    FMOD::Studio::System::create(&Instance->m_studioSystem);
    Instance->m_studioSystem->initialize(32, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr);
    
    // Get core system
    Instance->m_studioSystem->getCoreSystem(&Instance->m_system);
}

void AudioEngine::update() {
    if (Instance->m_studioSystem) {
        Instance->m_studioSystem->update();
    }
}

void AudioEngine::cleanUp() {
    // Stop all channels
    for (auto& channel : Instance->m_channels) {
        if (channel.second) {
            channel.second->stop();
        }
    }
    Instance->m_channels.clear();
    
    // Release FMOD systems
    if (Instance->m_studioSystem) {
        Instance->m_studioSystem->release();
        Instance->m_studioSystem = nullptr;
    }
    if (Instance->m_system) {
        Instance->m_system->release();
        Instance->m_system = nullptr;
    }
    
    if (Instance)
    {
        delete Instance;
        Instance = nullptr;
    }
}

void AudioEngine::playSound(const std::string& name, bool loop, float volume) {
    FMOD::Sound* sound = AssetManager::getSound(name);
    if (sound) {
        if (!loop) sound->setLoopCount(0);
        else sound->setLoopCount(-1);
        FMOD::Channel* channel = nullptr;
        Instance->m_system->playSound(sound, nullptr, false, &channel);
        if (channel) {
            channel->setVolume(volume);
            Instance->m_channels[name] = channel;
        }
    }
}

void AudioEngine::stopSound(const std::string& name) {
    auto it = Instance->m_channels.find(name);
    if (it != Instance->m_channels.end() && it->second) {
        it->second->stop();
    }
}

void AudioEngine::setPitch(const std::string& name, float pitch) {
    auto it = Instance->m_channels.find(name);
    if (it != Instance->m_channels.end() && it->second) {
        it->second->setPitch(pitch);
    }
}

void AudioEngine::setVolume(const std::string& name, float volume) {
    auto it = Instance->m_channels.find(name);
    if (it != Instance->m_channels.end() && it->second) {
        it->second->setVolume(volume);
    }
}

bool AudioEngine::isPlaying(const std::string& name) {
    auto it = Instance->m_channels.find(name);
    if (it != Instance->m_channels.end() && it->second) {
        bool playing = false;
        it->second->isPlaying(&playing);
        return playing;
    }
    return false;
}