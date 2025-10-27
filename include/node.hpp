#pragma once

#include "osmnode.hpp"
#include "coordinates.hpp"

#include <vector>
#include <limits>

class Edge;

class Node : public OsmNode
{
    public:
        Node(OsmNode &other) : OsmNode(other) {}

        std::vector<std::reference_wrapper<Edge>> edges;

        double g = std::numeric_limits<double>::infinity();  // known cost from start
        double f = std::numeric_limits<double>::infinity();  // estimated total cost
        uint64_t parent = 0;                                 // predecessor node id
        bool visited = false;

    private:
};