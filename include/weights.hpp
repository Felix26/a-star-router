#pragma once

#include <unordered_map>
#include <string>

#include "parameters.hpp"

class Weights
{
    public:
        Weights(const std::string &weightsCSVFile);

        double getWeight(const Parameters &parameters);

    private:
        // a map of parameter types, e.g. "highway", to a map of parameter values, e.g. "primary", to their corresponding weights
        std::unordered_map<std::string, std::unordered_map<std::string, double>> mWeights;

        std::unordered_map<std::string, double> getKeyWeights(const std::string &key);
};