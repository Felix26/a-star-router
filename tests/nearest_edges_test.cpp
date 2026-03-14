#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <cassert>

#include "router.hpp"
#include "library.hpp"
#include "coordinates.hpp"

int main()
{
    try
    {
        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/neureut.osm";

        Router router(osmPath);

        Coordinates testCoordinates = {49.053453, 8.384183};
        uint64_t assertionEdgeId = 779542839;

        auto closestEdges = router.getQuadtree().getClosestEdges(testCoordinates, 10);

        for(const auto &edge : closestEdges)
        {
            std::cout << "Edge ID: " << (edge.edge->getId() % 0x00FFFFFFFFFFFFFF) << ", Subway ID: " << static_cast<int>(edge.subwayId) << ", Distance: " << edge.distance << " m\n";
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
}
