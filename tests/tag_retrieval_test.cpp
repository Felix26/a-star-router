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

        //std::string path = std::format("points{}edges.csv", DISTANCE);
        //std::ofstream file(path);

        //file << "index,mapmatch,routing\n";

        std::vector<double> tagsMatched(27, 0);
        std::vector<double> tagsGuessed(27, 0);

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
                auto result = router.aStarEdges(ids[i], ids[std::min<uint32_t>(i + DISTANCE, ids.size() - 1)]);
                edgeSet.insert(edgeSet.end(), result.begin(), result.end());
            }

            edgeSet.emplace_back(router.getQuadtree().getClosestEdges(points[0])[0].edge);
            edgeSet.emplace_back(router.getQuadtree().getClosestEdges(points[points.size() - 1])[0].edge);
            routes.prepareEdgeSet(edgeSet);
            
            auto matchSet = routes.getEdgeSet(coordinates);

            auto tags = routes.getHighwayTags(matchSet);
            auto tags2 = routes.getHighwayTags(edgeSet);

            if(tags[0] != 0) std::cout << filename << std::endl;

            for(uint8_t i = 0; i < tags.size(); i++)
            {
                tagsMatched[i] += tags[i];
                tagsGuessed[i] += tags2[i];
                //file << std::format("{},{},{}", i, tags[i], tags2[i]);
            }

            //file.close();
        }

        for(uint8_t i = 0; i < tagsMatched.size(); i++)
        {
            std::cout << std::format("{}\t{:6.1f}\t{:6.1f}\n", i, tagsMatched[i] / 1000, tagsGuessed[i] / 1000);
            //file << std::format("{},{},{}", i, tags[i], tags2[i]);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
}
