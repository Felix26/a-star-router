#pragma once

#include "osmnode.hpp"
#include "coordinates.hpp"

#include <vector>
#include <limits>
#include <memory>

class Edge;

class Node : public OsmNode
{
    public:
        Node(const OsmNode &other) : OsmNode(other) {}

        std::vector<std::shared_ptr<Edge>> edges;

        double g = std::numeric_limits<double>::infinity();  // known cost from start
        double f = std::numeric_limits<double>::infinity();  // estimated total cost
        uint64_t parent = 0;                                 // predecessor node id
        bool visited = false;
        const Edge *parentEdge = nullptr;                    // edge used to reach this node
        bool parentEdgeReversed = false;                     // traversal direction flag

    private:
};
