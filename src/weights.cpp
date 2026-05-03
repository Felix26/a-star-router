#include "weights.hpp"

#include <fstream>
#include <sstream>

Weights::Weights(const std::string &weightsCSVFile)
{
    std::ifstream file(weightsCSVFile);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open weights CSV file: " + weightsCSVFile);
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream ss(line);
        std::string key, value;
        double weight;

        if (std::getline(ss, key, ',') && std::getline(ss, value, ',') && ss >> weight)
        {
            mWeights[key][value] = weight;
        }
    }
}

double Weights::getWeight(const Parameters &parameters)
{
    double weight = 0.0;
    constexpr double fallbackWeight = 1.0;
    size_t weightCount = 0;

    for(const auto &[key, value] : parameters.getParameters())
    {
        weightCount++;
        auto keyWeights = getKeyWeights(key);
        if(keyWeights.empty())
        {
            weight += fallbackWeight; // Default weight for unknown parameter keys
            continue;
        }

        auto it = keyWeights.find(value);
        if(it != keyWeights.end())
        {
            weight += it->second;
        }
        else
        {
            auto defaultWeight = keyWeights.find("default");
            if(defaultWeight != keyWeights.end())
            {
                weight += defaultWeight->second; // Default weight for unknown parameter values
            }
            else
            {
                weight += fallbackWeight; // Fallback default weight if no "default" value is defined
            }
        }
    }

    return weight / (weightCount > 0 ? weightCount : 1);
}

std::unordered_map<std::string, double> Weights::getKeyWeights(const std::string &key)
{
    auto it = mWeights.find(key);
    if(it != mWeights.end())
    {
        return it->second;
    }
    return {};
}

double Weights::getWeight(const std::string &key, const std::string &value)
{
    auto keyWeights = getKeyWeights(key);
    if(keyWeights.empty())
    {
        return 1.0; // Default weight for unknown parameter keys
    }

    auto it = keyWeights.find(value);
    if(it != keyWeights.end())
    {
        return it->second;
    }
    else
    {
        auto defaultWeight = keyWeights.find("default");
        if(defaultWeight != keyWeights.end())
        {
            return defaultWeight->second; // Default weight for unknown parameter values
        }
        else
        {
            return 1.0; // Fallback default weight if no "default" value is defined
        }
    }
}