#include "parameters.hpp"

Parameters::RoadClass Parameters::getHighwayTagID(const std::string &tagName)
{
    auto it = highway.find(tagName);
    if (it != highway.end())
    {
        return it->second;
    }

    return RoadClass::Unknown;
}

std::string_view Parameters::getHighwayTagName(Parameters::RoadClass tagID)
{
    size_t index = static_cast<size_t>(tagID);
    return Parameters::getHighwayTagName(index);
}

std::string_view Parameters::getHighwayTagName(uint8_t tagID)
{
    size_t index = static_cast<size_t>(tagID);
    if (index >= highwayParameterCount)
    {
        return "unknown";
    }
    return idToNameArray[index];
}
