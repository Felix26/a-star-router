#include "router.hpp"

#include <ankerl/unordered_dense.h>

#include "library.hpp"

Router::Router(std::string osmFile)
{
    ankerl::unordered_dense::map<uint64_t, std::shared_ptr<OsmNode>> nodes;
    ankerl::unordered_dense::map<uint64_t, std::unique_ptr<OsmWay>> ways;

    mGraph = std::make_unique<Graph>();

    HelperFunctions::readOSMFile(osmFile, nodes, ways);
    Box boundary = HelperFunctions::createGraph(*mGraph, nodes, ways);

    mQuadtree = std::make_unique<Quadtree>(*mGraph, boundary);
}
