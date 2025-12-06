#include "gpxparser.hpp"

#include <iostream>
#include <ankerl/unordered_dense.h>
#include <cassert>

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
        ankerl::unordered_dense::map<uint64_t, std::shared_ptr<OsmNode>> nodes;
        ankerl::unordered_dense::map<uint64_t, std::unique_ptr<OsmWay>> ways;

        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/neureut.osm";
        HelperFunctions::readOSMFile(osmPath, nodes, ways);
        HelperFunctions::createGraph(graph, nodes, ways);

        Box boundary(Coordinates(49.7913749328, 7.5113934084), Coordinates(47.5338000528, 10.4918239143));
        Quadtree quadtree(graph, boundary);

        std::cout << quadtree;

        // Check every edge bounding box
        for(Quadtree *subtree : quadtree.getAllSubtrees())
        {
            for(const auto &[edge, subwayId] : subtree->getEdgeSubwayIDs())
            {
                if(edge == nullptr) continue;

                const Box &box = edge->getBoundingBox(subwayId);
                assert(subtree->getBoundary().contains(box));
            }
        }

        Coordinates testPoint(49.053357, 8.384253);

        auto closestEdges = quadtree.getClosestEdges(testPoint, 10);
        std::cout << "Closest edges to point " << testPoint << ":\n";
        for(const auto &ce : closestEdges)
        {
            std::cout << "Edge ID: " << (ce.edge->getId() % 0x00FFFFFFFFFFFFFF) << ", Subway ID: " << static_cast<int>(ce.subwayId) << ", Distance: " << ce.distance << " m\n";
        }

        assert(closestEdges.size() == 10);
        assert(closestEdges[0].distance < closestEdges[1].distance);
        assert(closestEdges[0].distance < 10);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
    return 0;
}