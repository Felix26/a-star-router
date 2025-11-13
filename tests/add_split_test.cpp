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

        {
            Coordinates startCoordinates = {49.053453, 8.384183};
            Coordinates endCoordinates = {49.055255, 8.381203};

            auto [closestStartPoint, closestStartEdgeId, startSegment] = graph.getEdgeSplit(startCoordinates);
            uint64_t newNodeIdStart = graph.addSplit(closestStartPoint, closestStartEdgeId, startSegment);

            auto [closestEndPoint, closestEndEdgeId, endSegment] = graph.getEdgeSplit(endCoordinates);
            uint64_t newNodeIdEnd = graph.addSplit(closestEndPoint, closestEndEdgeId, endSegment);

            std::vector<std::tuple<uint64_t, Coordinates>> path = graph.aStar(newNodeIdStart, newNodeIdEnd);

            graph.removeSplitItems();
            
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
