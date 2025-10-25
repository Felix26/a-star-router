#pragma once

#include "node.hpp"

#include <array>
#include <functional>

class Edge
{
    public:
        Edge(uint64_t id, double waylength, Node &from, Node &to) : mId(id), mWaylength(waylength), mNodes{std::ref(from), std::ref(to)} {};

        double calculateWayLength() const;

    private:
        const uint64_t mId;
        const double mWaylength;
        const std::array<std::reference_wrapper<Node>, 2> mNodes;
};