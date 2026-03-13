#pragma once

#include <vector>
#include <ankerl/unordered_dense.h>
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

        const ankerl::unordered_dense::map<uint64_t, std::shared_ptr<Node>> &getNodes() const { return mNodes; }

        const ankerl::unordered_dense::map<uint64_t, std::shared_ptr<Edge>> &getEdges() const { return mEdges; }

        const Edge *getEdge(uint64_t edgeId) const { return mEdges.at(edgeId).get(); }
    private:
        ankerl::unordered_dense::map<uint64_t, std::shared_ptr<Node>> mNodes;
        ankerl::unordered_dense::map<uint64_t, std::shared_ptr<Edge>> mEdges;

        std::vector<uint64_t> mSplitItemIds;
        uint8_t mSplitItemCount = 0;
};
