#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <cassert>

#include <ankerl/unordered_dense.h>

#include "graph.hpp"
#include "library.hpp"
#include "osmnode.hpp"
#include "osmway.hpp"
#include "coordinates.hpp"

int main()
{
    try
    {
        Graph graph;
        ankerl::unordered_dense::map<uint64_t, std::shared_ptr<OsmNode>> nodes;
        ankerl::unordered_dense::map<uint64_t, std::unique_ptr<OsmWay>> ways;

        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/neureut.osm";
        HelperFunctions::readOSMFile(osmPath, nodes, ways);
        HelperFunctions::createGraph(graph, nodes, ways);

        {
            Coordinates testCoordinates = {49.053453, 8.384183};
            uint64_t assertionEdgeId = 779542839;
            Coordinates assertionCoordinates = {49.053404, 8.384092};

            auto [closestPoint, closestEdgeId, _] = graph.getEdgeSplit(testCoordinates);
            closestEdgeId &= 0x00FFFFFFFFFFFFFF;

            assert(closestEdgeId == assertionEdgeId);
            assert(HelperFunctions::haversine(closestPoint, assertionCoordinates) < 5);
        }

        {
            Coordinates testCoordinates = {49.055255, 8.381203};
            uint64_t assertionEdgeId = 132046539;
            Coordinates assertionCoordinates = {49.055195, 8.381031};

            auto [closestPoint, closestEdgeId, _] = graph.getEdgeSplit(testCoordinates);
            closestEdgeId &= 0x00FFFFFFFFFFFFFF;

            assert(closestEdgeId == assertionEdgeId);
            assert(HelperFunctions::haversine(closestPoint, assertionCoordinates) < 5);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
}
