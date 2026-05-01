#pragma once

#include <vector>
#include <functional>
#include <memory>

#include "osmnode.hpp"

class OsmWay
{
    public:
        OsmWay(uint64_t id) : mId(id) {}

        void addNode(std::shared_ptr<OsmNode> node);

        const std::vector<std::shared_ptr<OsmNode>> &getNodes() const { return mNodes; }

        const uint64_t getId() const {return mId;}

    private:
        uint64_t mId;
        std::vector<std::shared_ptr<OsmNode>> mNodes;
};