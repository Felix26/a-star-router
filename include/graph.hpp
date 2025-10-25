#pragma once

#include <vector>
#include <unordered_map>

#include <osmnode.hpp>
#include <osmway.hpp>
#include <node.hpp>
#include <edge.hpp>

class Graph
{
    public:
        void addOsmNode(OsmNode &node);
        void addOsmWay(OsmWay &way);

        void printGraph();

    private:
        std::unordered_map<uint64_t, Node> mNodes;
        std::unordered_map<uint64_t, Edge> mEdges;
};
