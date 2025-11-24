#pragma once

#include <vector>
#include <unordered_dense.h>
#include <memory>
#include <tuple>

#include <osmnode.hpp>
#include <osmway.hpp>
#include <node.hpp>
#include <edge.hpp>

class Graph
{
    public:
        void addOsmNode(std::shared_ptr<OsmNode> node);
        void addOsmWay(const OsmWay *way);

        uint64_t addSplit(Coordinates closestCoords, uint64_t edgeId, uint8_t segmentIndex);

        void removeSplitItems();

        void printGraph();

        std::vector<std::tuple<uint64_t, Coordinates>> aStar(uint64_t startId, uint64_t goalId);

        std::vector<std::tuple<uint64_t, Coordinates>> aStar(Coordinates startCoords, Coordinates goalCoords);

        const ankerl::unordered_dense::map<uint64_t, std::shared_ptr<Node>> &getNodes() const { return mNodes; }
        std::tuple<Coordinates, uint64_t, uint8_t> getEdgeSplit(Coordinates coords) const;

    private:
        ankerl::unordered_dense::map<uint64_t, std::shared_ptr<Node>> mNodes;
        ankerl::unordered_dense::map<uint64_t, std::shared_ptr<Edge>> mEdges;

        static double heuristic(const Node &a, const Node &b);

        std::tuple<uint64_t, uint8_t> getClosestSegment(Coordinates coords) const;

        // Returns the closest point on the segment to the given edge and the index of the segment
        Coordinates getClosestPointOnEdge(Coordinates coords, uint64_t edgeId, uint8_t segmentIndex) const;

        std::vector<uint64_t> mSplitItemIds;
        uint8_t mSplitItemCount = 0;
};
