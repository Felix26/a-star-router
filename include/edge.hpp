#pragma once

#include "node.hpp"

#include <array>
#include <functional>
#include <coordinates.hpp>

class Edge
{
    public:
        Edge(uint64_t id, double waylength, Node &from, Node &to, std::vector<Coordinates> path) : mId(id), mWaylength(waylength), mNodes{std::ref(from), std::ref(to)}, mPath(std::move(path)) {}

        double calculateWayLength() const { return mWaylength; }

    private:
        const uint64_t mId;
        const double mWaylength;
        const std::array<std::reference_wrapper<Node>, 2> mNodes;

        const std::vector<Coordinates> mPath;
};