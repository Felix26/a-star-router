#include "weights.hpp"

#include <fstream>
#include <sstream>
#include <format>

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

double Weights::getWeight(const Parameters &parameters) const
{
    if(parameters.getParameters().empty())
    {
        return fallbackWeight;
    }

    double weight = 0.0;
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
            if(key.find("cross:") != std::string::npos)
            {
                weight += 0;
            }
            else
            {
                weight += fallbackWeight; // Fallback default weight if no "default" value is defined
            }
        }
    }

    return weight;
}

void Weights::saveWeights(const std::string &filename)
{
    std::ofstream file(filename);

    for(const auto &[key, keymap] : mWeights)
    {
        for(const auto &[value, weight] : keymap)
        {
            file << std::format("{},{},{}\n", key, value, weight);
        }
    }

    file.close();
}

ankerl::unordered_dense::map<std::string, double> Weights::getKeyWeights(const std::string &key) const
{
    auto it = mWeights.find(key);
    if(it != mWeights.end())
    {
        return it->second;
    }
    return {};
}

double Weights::getWeight(const std::string &key, const std::string &value) const
{
    auto keyWeights = getKeyWeights(key);
    if(keyWeights.empty())
    {
        return fallbackWeight; // Default weight for unknown parameter keys
    }

    auto it = keyWeights.find(value);
    if(it != keyWeights.end())
    {
        return it->second;
    }
    else
    {
        if(key.find("cross:") != std::string::npos)
        {
            return 0;
        }
        else
        {
            return fallbackWeight; // Fallback default weight if no "default" value is defined
        }
    }
}