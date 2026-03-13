#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <cassert>

#include <ankerl/unordered_dense.h>

#include "router.hpp"
#include "library.hpp"
#include "osmnode.hpp"
#include "osmway.hpp"
#include "coordinates.hpp"

int main()
{
    try
    {
        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/neureut.osm";

        Router router(osmPath);

        {
            Coordinates startCoordinates = {49.053453, 8.384183};
            Coordinates endCoordinates = {49.055255, 8.381203};

            auto [closestStartPoint, closestStartEdgeId, startSegment] = router.getEdgeSplit(startCoordinates);
            uint64_t newNodeIdStart = router.getGraph().addSplit(closestStartPoint, closestStartEdgeId, startSegment);

            auto [closestEndPoint, closestEndEdgeId, endSegment] = router.getEdgeSplit(endCoordinates);
            uint64_t newNodeIdEnd = router.getGraph().addSplit(closestEndPoint, closestEndEdgeId, endSegment);

            std::vector<std::tuple<uint64_t, Coordinates>> path = router.aStar(newNodeIdStart, newNodeIdEnd);

            router.getGraph().removeSplitItems();
            
            HelperFunctions::exportPathToGeoJSON(path, "add_split_test_path.geojson");
            assert(!path.empty());
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
}
