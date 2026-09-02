#pragma once
#include "fmod.hpp"

namespace FMOD
{
    namespace Studio
    {
        class System
        {
        public:
            static FMOD_RESULT create(System** system, unsigned int = 0)
            {
                static System instance;
                if (system)
                {
                    *system = &instance;
                }
                return FMOD_OK;
            }

            FMOD_RESULT initialize(int, FMOD_STUDIO_INITFLAGS, FMOD_INITFLAGS, void*)
            {
                return FMOD_OK;
            }

            FMOD_RESULT getCoreSystem(FMOD::System** system)
            {
                if (system)
                {
                    *system = &m_core;
                }
                return FMOD_OK;
            }

            FMOD_RESULT update() { return FMOD_OK; }

            FMOD_RESULT release() { return FMOD_OK; }

        private:
            FMOD::System m_core;
        };
    }
}
