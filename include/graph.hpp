#pragma once

#include <vector>
#include <unordered_map>

#include <osmnode.hpp>
#include <osmway.hpp>
#include <node.hpp>
#include <edge.hpp>

class Graph
{
    public:
        void addOsmNode(OsmNode &node);
        void addOsmWay(OsmWay &way);

        void printGraph();

        std::vector<std::tuple<uint64_t, Coordinates>> aStar(uint64_t startId, uint64_t goalId);

        const std::unordered_map<uint64_t, Node> &getNodes() const { return mNodes; }

    private:
        std::unordered_map<uint64_t, Node> mNodes;
        std::unordered_map<uint64_t, Edge> mEdges;

        static double heuristic(const Node &a, const Node &b);
};
