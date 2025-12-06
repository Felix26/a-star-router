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

        Coordinates testCoordinates = {49.053453, 8.384183};
        uint64_t assertionEdgeId = 779542839;

        uint64_t closestEdgeId = std::get<1>(graph.getEdgeSplit(testCoordinates)) & 0x00FFFFFFFFFFFFFF;

        assert(closestEdgeId == assertionEdgeId);
        

        testCoordinates = {49.050133, 8.390682};
        assertionEdgeId = 132153778;

        closestEdgeId = std::get<1>(graph.getEdgeSplit(testCoordinates)) & 0x00FFFFFFFFFFFFFF;

        assert(closestEdgeId == assertionEdgeId);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
}
