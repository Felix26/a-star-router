#include "weights.hpp"

#include <fstream>
#include <sstream>

Weights::Weights(std::string &weightsCSVFile)
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
    double weight = 1.0;
    constexpr double defaultPenalty = 2.0;

    for(const auto &[key, value] : parameters.getParameters())
    {
        auto keyWeights = getKeyWeights(key);
        if(keyWeights.empty())
        {
            weight *= defaultPenalty; // Default penalty for unknown parameter keys
            continue;
        }

        auto it = keyWeights.find(value);
        if(it != keyWeights.end())
        {
            weight *= it->second;
        }
        else
        {
            auto defaultWeight = keyWeights.find("default");
            if(defaultWeight != keyWeights.end())
            {
                weight *= defaultWeight->second; // Default penalty for unknown parameter values
            }
            else
            {
                weight *= defaultPenalty; // Fallback default penalty if no "default" value is defined
            }
        }
    }

    return weight;
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