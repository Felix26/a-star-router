#pragma once

#include "node.hpp"

#include <array>
#include <functional>
#include <coordinates.hpp>
#include <memory>

class Edge
{
    public:
        Edge(uint64_t id, double waylength, std::shared_ptr<Node> from, std::shared_ptr<Node> to, std::vector<Coordinates> path);

        double calculateWayLength() const { return mWaylength; }

        const uint64_t getId() const { return mId; }

        std::shared_ptr<Node> from() const { return mNodes[0].lock(); }
        std::shared_ptr<Node> to() const { return mNodes[1].lock(); }

        const std::vector<Coordinates> &getPath() const { return mPath; }

    private:
        const uint64_t mId;
        const double mWaylength;
        const std::array<std::weak_ptr<Node>, 2> mNodes;

        const std::vector<Coordinates> mPath;
};
