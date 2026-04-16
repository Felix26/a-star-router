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
