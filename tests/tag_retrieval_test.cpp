#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <cassert>
#include <fstream>

#include "router.hpp"
#include "gpxparser.hpp"
#include "routes.hpp"
#include "library.hpp"
#include "weights.hpp"

#define DISTANCE 20

int main()
{
    try
    {
        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/Karlsruhe_Region.osm";
        Router router(osmPath);
        GPXParser parser;
        Routes routes(router);

        parser.loadGPXFiles(std::string(PROJECT_SOURCE_DIR) + "/testdata/gpxdata");

        //std::string path = std::format("highwayProfile.csv");
        //std::ofstream file(path);

        //file << "#parameter,weight\n";

        std::vector<double> tagsMatched(27, 0);
        std::vector<double> tagsGuessed(27, 0);

        double jaccardSum = 0;
        uint8_t jaccardCount = 0;

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

            auto tags = routes.getHighwayTags(matchSet);
            auto tags2 = routes.getHighwayTags(edgeSet);

            for(uint8_t i = 0; i < tags.size(); i++)
            {
                tagsMatched[i] += tags[i];
                tagsGuessed[i] += tags2[i];
                //file << std::format("{},{},{}", i, tags[i], tags2[i]);
            }

        }

        std::array<double, 27> weightRatios;
        double minRatio = std::numeric_limits<double>::max();

        for(uint8_t i = 0; i < tagsMatched.size(); i++)
        {
            weightRatios[i] = std::max<double>(0.25, HighwayWeights::getHighwayPenalty(i) * (0.9 + 0.1 * (tagsMatched[i] > 1000 ? tagsGuessed[i] / tagsMatched[i] : 2)));
            std::cout << std::format("{:15} ({})\t{:6.1f}\t{:6.1f}\t{:6.2f}\n", Parameters::getHighwayTagName(i), i, tagsMatched[i] / 1000, tagsGuessed[i] / 1000, weightRatios[i]);

            if(weightRatios[i] < minRatio) minRatio = weightRatios[i];
        }

        for(uint8_t i = 0; i < weightRatios.size(); i++)
        {
            weightRatios[i] *= 1 / minRatio;
            //file << std::format("{},{}\n", Parameters::getHighwayTagName(i), weightRatios[i]);
        }

        std::cout << std::format("mean jaccard: {:4}\n", jaccardSum / jaccardCount);

        
        //file.close();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
}
