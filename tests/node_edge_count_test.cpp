#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <cassert>

#include "router.hpp"
#include "library.hpp"

int main()
{
    try
    {

        Graph graph;
        ankerl::unordered_dense::map<uint64_t, std::shared_ptr<OsmNode>> nodes;
        ankerl::unordered_dense::map<uint64_t, std::unique_ptr<OsmWay>> ways;

        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/bw_min.osm";
        HelperFunctions::readOSMFile(osmPath, nodes, ways);
        HelperFunctions::createGraph(graph, nodes, ways);

        std::cout << "Node count after erasing empty nodes: " << nodes.size() << std::endl;
        std::cout << "Edge count before converting: " << ways.size() << std::endl;

        uint64_t potentialEdgeCount = 0;
        for(auto &way : ways)
        {
            potentialEdgeCount+= way.second->getNodes().size() - 1;
        }

        std::cout << "Potential Edge Count if doing naive transformation to graph: " << potentialEdgeCount << std::endl;

        std::cout << "Graph has " << graph.getNodes().size() << " nodes and "
                  << graph.getEdges().size() << " edges.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
}
