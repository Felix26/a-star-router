#pragma once

#include <vector>
#include <unordered_dense.h>
#include <memory>

#include <osmnode.hpp>
#include <osmway.hpp>
#include <node.hpp>
#include <edge.hpp>

class Graph
{
    public:
        void addOsmNode(std::shared_ptr<OsmNode> node);
        void addOsmWay(const OsmWay *way);

        void printGraph();

        std::vector<std::tuple<uint64_t, Coordinates>> aStar(uint64_t startId, uint64_t goalId);

        const ankerl::unordered_dense::map<uint64_t, std::shared_ptr<Node>> &getNodes() const { return mNodes; }

    private:
        ankerl::unordered_dense::map<uint64_t, std::shared_ptr<Node>> mNodes;
        ankerl::unordered_dense::map<uint64_t, std::shared_ptr<Edge>> mEdges;

        static double heuristic(const Node &a, const Node &b);
};
