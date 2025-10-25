#pragma once

#include "osmnode.hpp"
#include "coordinates.hpp"

class Node : public OsmNode
{
    public:
        Node(OsmNode &other) : OsmNode(other) {}

    private:
};