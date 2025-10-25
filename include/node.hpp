#pragma once

#include "osmnode.hpp"
#include "coordinates.hpp"

#include <vector>

class Edge;

class Node : public OsmNode
{
    public:
        Node(OsmNode &other) : OsmNode(other) {}

        std::vector<std::reference_wrapper<Edge>> edges;

    private:
};