#include "parameters.hpp"

uint8_t Parameters::getHighwayTagID(const std::string &tagName)
{
    auto it = highway.find(tagName);
    if (it != highway.end())
    {
        return it->second;
    }
    
    return static_cast<uint8_t>(RoadClass::Unknown);
}
