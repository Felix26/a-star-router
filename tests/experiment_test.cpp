#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <cassert>

#include <unordered_dense.h>

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
        ankerl::unordered_dense::map<u_int64_t, std::shared_ptr<OsmNode>> nodes;
        ankerl::unordered_dense::map<uint64_t, std::unique_ptr<OsmWay>> ways;

        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/neureut.osm";
        HelperFunctions::readOSMFile(osmPath, nodes, ways);
        HelperFunctions::createGraph(graph, nodes, ways);

        Coordinates testCoordinates = {49.055824, 8.385704};

        std::cout << (graph.getClosestEdgeId(testCoordinates) & 0x00FFFFFFFFFFFFFF);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
}
