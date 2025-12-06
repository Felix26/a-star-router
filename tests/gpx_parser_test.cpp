#include "gpxparser.hpp"

#include <iostream>
#include <ankerl/unordered_dense.h>
#include "osmway.hpp"
#include "osmnode.hpp"
#include "graph.hpp"
#include "library.hpp"

int main()
{
    try
    {
        Graph graph;
        ankerl::unordered_dense::map<uint64_t, std::shared_ptr<OsmNode>> nodes;
        ankerl::unordered_dense::map<uint64_t, std::unique_ptr<OsmWay>> ways;

        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/karlsruhe_roads_min.osm";
        HelperFunctions::readOSMFile(osmPath, nodes, ways);
        HelperFunctions::createGraph(graph, nodes, ways);

        Box boundary(Coordinates(49.73600, 7.949946), Coordinates(48.31047, 9.605534));
        Quadtree quadtree(graph, boundary);
        quadtree.initQuadTree();
        
        GPXParser parser;

        parser.loadGPXFiles("/home/felixm/Desktop/Studienarbeit/Router/testdata/gpxdata");

        parser.fillEdgeIDs(quadtree);

        std::cout << "Number of found edges: " << parser.getEdgeIDs().size() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
    return 0;
}