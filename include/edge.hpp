#pragma once

#include "node.hpp"
#include "box.hpp"

#include <array>
#include <functional>
#include <coordinates.hpp>
#include <memory>

#include "parameters.hpp"
#include "tags.hpp"

class Edge
{
    public:
        Edge(uint64_t id, double waylength, std::shared_ptr<Node> from, std::shared_ptr<Node> to, std::vector<Coordinates> path, Tags tags);

        double getWeight() const { return mWaylength; }
        void setWeight(double waylength) { mWaylength = waylength; }

        double calculateWayLength() const;

        const uint64_t getId() const { return mId; }

        std::shared_ptr<Node> from() const { return mNodes[0].lock(); }
        std::shared_ptr<Node> to() const { return mNodes[1].lock(); }

        const std::vector<Coordinates> &getPath() const { return mPath; }

        const Box getBoundingBox(uint8_t subWayId) const { return Box(mPath[subWayId], mPath[subWayId + 1]); }

        uint16_t snapPointCounter = 0;
        uint16_t bestSnapPointCounter = 0;

        Tags tags;

        bool operator<(const Edge& other) const { return mId < other.mId; }
        bool operator==(const Edge& other) const { return mId == other.mId; }

    private:
        uint64_t mId;
        double mWaylength;
        std::array<std::weak_ptr<Node>, 2> mNodes;

        std::vector<Coordinates> mPath;
};
