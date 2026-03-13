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

        uint64_t closestEdgeId = std::get<1>(router.getEdgeSplit(testCoordinates)) & 0x00FFFFFFFFFFFFFF;

        assert(closestEdgeId == assertionEdgeId);
        

        testCoordinates = {49.050133, 8.390682};
        assertionEdgeId = 132153778;

        closestEdgeId = std::get<1>(router.getEdgeSplit(testCoordinates)) & 0x00FFFFFFFFFFFFFF;

        assert(closestEdgeId == assertionEdgeId);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
}
