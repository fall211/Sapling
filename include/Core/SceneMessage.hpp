//
//  SceneMessage.hpp
//  SaplingEngine
//

#pragma once

#include "Utility/Debug.hpp"
#include <any>
#include <string>

class SceneMessage {
private:
    std::string m_type;
    std::any m_data;
    
public:
    template<typename T>
    SceneMessage(const std::string& type, const T& data) 
        : m_type(type), m_data(data) {}
    
    const std::string& getType() const { return m_type; }
    
    template<typename T>
    T getData() const 
    { 
        try
        {
            return std::any_cast<T>(m_data); 
        } 
        catch (const std::bad_any_cast& e)
        {
            Debug::log("ERROR: bad cast in SceneMessage. requested type doesnt match stored type");
            return T();
        }
    }
    
    bool hasType(const std::string& type) const { return m_type == type; }
};