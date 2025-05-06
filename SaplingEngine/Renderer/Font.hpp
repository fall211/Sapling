//
//
//
//

#pragma once


#include <stb/stb_truetype.h>

#include <cstdint>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

namespace Sprout
{

    class Font
    {

        public:
            unsigned char* data;
            int charCount = 96;
            stbtt_bakedchar* bakedChars = (stbtt_bakedchar*)malloc(sizeof(stbtt_bakedchar) * charCount);
            uint8_t fontId;
            float size;
            
            Font() = default;
            ~Font();
            
            bool loadFromFile(const std::string& fontPath, float size);
            void registerFont();
            void release();
        };
}
