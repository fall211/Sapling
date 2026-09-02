#pragma once

typedef int FMOD_RESULT;
typedef unsigned int FMOD_MODE;
typedef unsigned int FMOD_INITFLAGS;
typedef unsigned int FMOD_STUDIO_INITFLAGS;

enum
{
    FMOD_OK = 0,
    FMOD_ERR_INTERNAL = 1
};

enum
{
    FMOD_DEFAULT = 0x00000000,
    FMOD_LOOP_NORMAL = 0x00000002
};

enum
{
    FMOD_INIT_NORMAL = 0x00000000
};

enum
{
    FMOD_STUDIO_INIT_NORMAL = 0x00000000
};

struct FMOD_CREATESOUNDEXINFO;
