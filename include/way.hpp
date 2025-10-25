#pragma once

#include <vector>
#include <functional>

#include "node.hpp"

class Way
{
    public:
        Way(uint64_t id) : mId(id) {}

        void addNode(Node &node);

        double calculateWayLength() const;

    private:
        uint64_t mId;
        std::vector<std::reference_wrapper<Node>> mNodes;
};