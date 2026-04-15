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

int main()
{
    try
    {
        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/Karlsruhe_Region.osm";
        Router router(osmPath);
        GPXParser parser;
        Routes routes(router);

        parser.loadGPXFiles("/home/felixm/Nextcloud/Studienarbeit/gpxdata");
        
        //Path trackPoints = HelperFunctions::getGPXTrackPoints("/home/felixm/Nextcloud/Studienarbeit/gpxdata/230305-30.gpx");
        //std::vector<std::tuple<uint64_t, Coordinates>> matchedTrack = parser.mapMatching(router, trackPoints);

        std::ofstream file("points.csv");

        file << "name,length,decisionNodes\n";


        for(auto &[filename, points] : parser.getTracks())
        {
            uint16_t decisionNodes = 0;
            if(!router.getQuadtree().getBoundary().contains(points[0])) continue;
            std::vector<std::tuple<uint64_t, Coordinates>> matchedTrack = parser.mapMatching(router, points);

            for(auto &[id, coords] : matchedTrack)
            {
                if(id && id < 0x00FFFFFFFFFFFFFF)
                {
                    //std::cout << *(router.getGraph().getNodes().at(id)) << std::endl;
                    if(router.getGraph().getNodes().at(id)->edges.size() > 2) decisionNodes++;
                }
            }
            //std::cout << "Path " << filename << " has " << HelperFunctions::calculatePathLength(points) << " m with " << decisionNodes << " decision nodes (" << HelperFunctions::calculatePathLength(points) / decisionNodes << " m/node).\n";
            file << filename << "," << std::format("{:2}", HelperFunctions::calculatePathLength(points)) << "," << decisionNodes << std::endl;
        }

        file.close();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
}
