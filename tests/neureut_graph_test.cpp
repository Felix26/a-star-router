#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <string>

#include <ankerl/unordered_dense.h>

#include "graph.hpp"
#include "library.hpp"
#include "osmnode.hpp"
#include "osmway.hpp"

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

        return graph.getNodes().empty() ? 1 : 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
}
