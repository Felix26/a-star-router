#pragma once

#include <vector>
#include <functional>

#include "osmnode.hpp"

class OsmWay
{
    public:
        OsmWay(uint64_t id) : mId(id) {}

        void addNode(OsmNode &node);

        double calculateWayLength() const;

    private:
        uint64_t mId;
        std::vector<std::reference_wrapper<OsmNode>> mNodes;
};