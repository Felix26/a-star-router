#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <cassert>
#include <fstream>
#include <format>

#include "router.hpp"
#include "gpxparser.hpp"
#include "routes.hpp"
#include "library.hpp"
#include "weights.hpp"
#include "parameters.hpp"

#define DISTANCE 5

int main()
{
    try
    {
        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/Karlsruhe_Region.osm";
        Router router(osmPath, "weights.csv");
        GPXParser parser;
        Routes routes(router);

        parser.loadGPXFiles(std::string(PROJECT_SOURCE_DIR) + "/testdata/gpxdata");

        std::string path = std::format("weights.csv");
        std::ofstream file(path);

        //file << "#parameter,weight\n";

        double jaccardSum = 0;
        uint8_t jaccardCount = 0;

        std::unordered_map<std::string, std::unordered_map<std::string, double>> tagsMatched;
        std::unordered_map<std::string, std::unordered_map<std::string, double>> tagsGuessed;

        for(auto &[filename, points] : parser.getTracks())
        {
            uint16_t decisionNodes = 0;
            std::vector<Edge *> edgeSet;
            Path coordinates;
            std::vector<uint64_t> ids;

            if(!router.getQuadtree().getBoundary().contains(points[0])) continue;
            std::vector<std::tuple<uint64_t, Coordinates>> matchedTrack = parser.mapMatching(router, points);

            for(uint32_t i = 0; i < matchedTrack.size(); i++)
            {
                auto &[id, coords] = matchedTrack[i];
                coordinates.emplace_back(coords);
                if(id && id < 0x00FFFFFFFFFFFFFF)
                {
                    if(router.getGraph().getNodes().at(id)->edges.size() > 2) decisionNodes++;
                    ids.emplace_back(id);
                }
            }

            for(uint32_t i = 0; i < ids.size(); i += DISTANCE)
            {
                auto result = router.aStarEdges(ids[i], ids[std::min<uint32_t>(i + DISTANCE, ids.size() - 1)], true);
                edgeSet.insert(edgeSet.end(), result.begin(), result.end());
            }

            edgeSet.emplace_back(router.getQuadtree().getClosestEdges(points[0])[0].edge);
            edgeSet.emplace_back(router.getQuadtree().getClosestEdges(points[points.size() - 1])[0].edge);
            routes.prepareEdgeSet(edgeSet);
            
            auto matchSet = routes.getEdgeSet(coordinates);

            double jaccard = routes.getJaccardCoefficient(edgeSet, matchSet);
            jaccardSum += jaccard;
            jaccardCount++;

            //std::cout << filename << ": " << jaccard << std::endl;

            auto tags = routes.getParameterLengthMap(matchSet);
            auto tags2 = routes.getParameterLengthMap(edgeSet);

            for(const auto &[key, valueMap] : tags)
            {
                for(const auto &[value, length] : valueMap)
                {
                    tagsMatched[key][value] += length;
                }
            }
            for(const auto &[key, valueMap] : tags2)
            {
                for(const auto &[value, length] : valueMap)
                {
                    tagsGuessed[key][value] += length;
                }
            }

        }

        std::vector<std::tuple<std::string, std::string, double>> tagsDifferenceVector;
        double lengthMatched = 0;
        double lengthGuessed = 0;
        double costMatched = 0;
        double costGuessed = 0;

        for(const auto &[key, valueMap] : tagsMatched)
        {
            for(const auto &[value, length] : valueMap)
            {
                double tagsGuessedLength = 0;
                double tagCost = router.getWeights().getWeight(key, value);
                if(tagsGuessed[key].find(value) != tagsGuessed[key].end())
                {
                    tagsGuessedLength = tagsGuessed[key][value];
                    tagsGuessed[key].erase(value);
                }
                std::cout << std::format("{}: {:20} {:6.1f} {:6.1f}\n", key, value, length / 1000, tagsGuessedLength / 1000);
                tagsDifferenceVector.emplace_back(key, value, length - tagsGuessedLength);

                lengthMatched += length;
                lengthGuessed += tagsGuessedLength;
                costMatched += length + length * tagCost;
                costGuessed += tagsGuessedLength + tagsGuessedLength * tagCost;

            }
            for(const auto &[value, length] : tagsGuessed[key])
            {
                std::cout << std::format("{}: {:20} {:6.1f} {:6.1f}\n", key, value, 0.0, length / 1000);
                tagsDifferenceVector.emplace_back(key, value, -length);
                lengthGuessed += length;
                costGuessed += length + length * router.getWeights().getWeight(key, value);
            }
        }

        std::cout << std::format("mean jaccard: {:4}\n", jaccardSum / jaccardCount);

        double lengthDifference = lengthMatched - lengthGuessed;
        double costDifference = costMatched - costGuessed;
        std::cout << std::format("length matched: {:6.1f} km, cost: {:6.1f} \n", lengthMatched / 1000, costMatched / 1000);
        std::cout << std::format("length guessed: {:6.1f} km, cost: {:6.1f} \n", lengthGuessed / 1000, costGuessed / 1000);
        std::cout << std::format("length difference: {:6.1f} km\n", lengthDifference / 1000);
        std::cout << std::format("cost difference: {:6.1f} \n", costDifference / 1000);

        double norm = 0;
        for(auto &[key, value, difference] : tagsDifferenceVector)
        {
            norm += difference * difference;
        }
        for(auto &[key, value, difference] : tagsDifferenceVector)
        {
            double normedgradient = difference / norm * costDifference;
            std::cout << std::format("{},{},{}\n", key, value, normedgradient);

            double currentWeight = router.getWeights().getWeight(key, value);
            double newWeight = std::max(0.0, currentWeight - normedgradient);
            file << std::format("{},{},{}\n", key, value, newWeight);
        }

        
        file.close();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
}