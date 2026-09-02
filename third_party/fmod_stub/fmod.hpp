#pragma once
#include "fmod_common.h"

namespace FMOD
{
    class ChannelGroup;
    class Channel;
    class Sound;
    class System;

    class Sound
    {
    public:
        FMOD_RESULT setMode(FMOD_MODE) { return FMOD_OK; }
        FMOD_RESULT setLoopCount(int) { return FMOD_OK; }
        FMOD_RESULT release()
        {
            delete this;
            return FMOD_OK;
        }
    };

    class Channel
    {
    public:
        FMOD_RESULT setVolume(float) { return FMOD_OK; }
        FMOD_RESULT stop() { return FMOD_OK; }
        FMOD_RESULT setPitch(float) { return FMOD_OK; }
        FMOD_RESULT isPlaying(bool* playing)
        {
            if (playing)
            {
                *playing = false;
            }
            return FMOD_OK;
        }
    };

    class System
    {
    public:
        FMOD_RESULT createSound(const char*, FMOD_MODE, FMOD_CREATESOUNDEXINFO*, Sound** sound)
        {
            if (sound)
            {
                *sound = new Sound();
            }
            return FMOD_OK;
        }

        FMOD_RESULT playSound(Sound*, ChannelGroup*, bool, Channel** channel)
        {
            if (channel)
            {
                *channel = &m_channel;
            }
            return FMOD_OK;
        }

        FMOD_RESULT release() { return FMOD_OK; }

    private:
        Channel m_channel;
    };
}
