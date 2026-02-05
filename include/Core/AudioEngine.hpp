//
// SaplingEngine Audio Engine
// Made using FMOD Studio by Firelight Technologies Pty Ltd.
//


#pragma once

#include "fmod.hpp"
#include "fmod_studio.hpp"
#include "fmod_errors.h"

#include <unordered_map>
#include <string>

class AssetManager;

class AudioEngine
{
    private:
        static AudioEngine* Instance;
    
        FMOD::System* m_system;
        FMOD::Studio::System* m_studioSystem;
        std::unordered_map<std::string, FMOD::Channel*> m_channels;
        
        AudioEngine();
        ~AudioEngine();
        
    public:
        static AudioEngine* getInstance()
        {
            if (!Instance)
            {
                Instance = new AudioEngine();
            }
            return Instance;
        }
        static FMOD::System* getSystem()
        {
            return Instance->m_system;
        }
        
        static void initialize();
        static void update();
        static void cleanUp();
        
        static void playSound(const std::string& soundName, bool loop = false, float volume = 1.0f);
        static void stopSound(const std::string& soundName);
        static void setPitch(const std::string& soundName, float pitch);
        static void setVolume(const std::string& soundName, float volume);
        static bool isPlaying(const std::string& soundName);
};