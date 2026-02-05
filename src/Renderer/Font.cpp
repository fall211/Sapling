//
//
//
//

#include "Renderer/Font.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

#include "Renderer/Sprout.hpp"

#include <fstream>
#include <iostream>
#include <vector>

namespace Sprout
{

    Font::~Font() 
    {
        free(bakedChars);
        free(data);
    }
    
    bool Font::loadFromFile(const std::string& fontPath, float size)
        {
            // read the font .ttf file into memory and assing data to this.data
            std::ifstream file(fontPath, std::ios::binary | std::ios::ate);
            if (!file)
            {
                return false;
            }

            std::streamsize filesize = file.tellg();
            file.seekg(0, std::ios::beg);

            std::vector<unsigned char> buffer(filesize);
            if (!file.read(reinterpret_cast<char*>(buffer.data()), filesize)) {
                return false;
            }

            // Allocate memory and copy data from vector to raw pointer
            this->data = static_cast<unsigned char*>(malloc(filesize));
            if (!this->data) {
                return false;
            }
            memcpy(this->data, buffer.data(), filesize);
            this->size = size;
            registerFont();
            return true;
        }
    
    void Font::registerFont()
    {
        Window::getInstance()->addFont(std::shared_ptr<Font>(this));
    }
    
    void Font::release()
    {
        free(bakedChars);
        free(data);
    }

}
