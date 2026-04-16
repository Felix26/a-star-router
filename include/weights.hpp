#pragma once

#include <fstream>
#include <sstream>
#include <iostream>

#include "tags.hpp"
#include "parameters.hpp"

class HighwayWeights
{
    public:
        static double getHighwayPenalty(Tags &tags);
        static double getHighwayPenalty(size_t index);

    private:
        static const inline std::array<double, Parameters::highwayParameterCount> penalties = []()
        {
            std::array<double, Parameters::highwayParameterCount> arr;
            arr.fill(1.0);

            std::ifstream highwayProfile("highwayProfile.csv");

            if(!highwayProfile.is_open())
            {
                std::cerr << "Profil nicht geladen, nutze Fallback" << std::endl;
            }

            std::string line;
            while(std::getline(highwayProfile, line))
            {
                if(line.empty() || line[0] == '#') continue;

                std::stringstream ss(line);
                std::string tag, weightString;

                if(std::getline(ss, tag, ',') && std::getline(ss, weightString))
                {
                    arr[static_cast<size_t>(Parameters::getHighwayTagID(tag))] = std::stod(weightString);
                }
            }

            return arr;
        }();
};