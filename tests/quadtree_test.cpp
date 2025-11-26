#include "gpxparser.hpp"

#include <iostream>
#include <unordered_dense.h>
#include "osmway.hpp"
#include "osmnode.hpp"
#include "graph.hpp"
#include "library.hpp"

#include "quadtree.hpp"

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

        Box boundary(Coordinates(49.7913749328, 7.5113934084), Coordinates(47.5338000528, 10.4918239143));
        Quadtree quadtree(graph, boundary);
        
        for(const auto &[edgeId, edge] : graph.getEdges())
        {
            const auto &path = edge->getPath();
            for(size_t subWayId = 0; subWayId < path.size() - 1; subWayId++)
            {
                quadtree.insert(edgeId, subWayId);
            }
        }
        std::cout << quadtree;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
    return 0;
}