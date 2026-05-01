#include "parameters.hpp"

void Parameters::setParameter(const std::string &key, const std::string &value)
{
    parameters[key] = value;
}

std::string Parameters::getParameter(const std::string &key) const
{
    auto it = parameters.find(key);
    if(it != parameters.end())
    {
        return it->second;
    }
    return "";
}
