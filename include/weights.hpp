#pragma once

#include <ankerl/unordered_dense.h>
#include <string>

#include "parameters.hpp"

class Weights
{
    public:
        Weights(const std::string &weightsCSVFile);

        double getWeight(const Parameters &parameters) const;
        double getWeight(const std::string &key, const std::string &value) const;

        void setWeight(const std::string &key, const std::string &value, double weight)
        {
            mWeights[key][value] = weight;
        }

        void saveWeights(const std::string &filename);

    private:
        // a map of parameter types, e.g. "highway", to a map of parameter values, e.g. "primary", to their corresponding weights
        ankerl::unordered_dense::map<std::string, ankerl::unordered_dense::map<std::string, double>> mWeights;

        ankerl::unordered_dense::map<std::string, double> getKeyWeights(const std::string &key) const;

        
        static constexpr double fallbackWeight = 0.0;
};