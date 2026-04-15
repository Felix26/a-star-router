#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <cassert>

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
        
        Path trackPoints = HelperFunctions::getGPXTrackPoints("/home/felixm/Nextcloud/Studienarbeit/gpxdata/230305-30.gpx");
        std::vector<std::tuple<uint64_t, Coordinates>> matchedTrack = parser.mapMatching(router, trackPoints);

        uint16_t decisionNodes = 0;

        for(auto &[id, coords] : matchedTrack)
        {
            if(id && id < 0x00FFFFFFFFFFFFFF)
            {
                std::cout << *(router.getGraph().getNodes().at(id)) << std::endl;
                if(router.getGraph().getNodes().at(id)->edges.size() > 2) decisionNodes++;
            }
        }

        std::cout << "Path has " << HelperFunctions::calculatePathLength(trackPoints) << " m with " << decisionNodes << " decision nodes (" << HelperFunctions::calculatePathLength(trackPoints) / decisionNodes << " m/node).\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
}
