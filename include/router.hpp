#pragma once

#include <string>
#include <memory>

#include "graph.hpp"
#include "quadtree.hpp"

#define NO_EDGE_SNAP_PENALTY 1000

class Router
{
    public:
        Router(const std::string &osmFile);

        Graph &getGraph() { return *mGraph; }
        Quadtree &getQuadtree() { return *mQuadtree; }

        std::vector<std::tuple<uint64_t, Coordinates>> aStar(uint64_t startId, uint64_t goalId, uint8_t snapToRoads = 0);
        
        std::vector<std::tuple<uint64_t, Coordinates>> aStar(Coordinates startCoords, Coordinates goalCoords, uint8_t snapToRoads = 0);

        std::vector<Edge> aStarEdges(uint64_t startId, uint64_t goalId);
        
        std::tuple<Coordinates, uint64_t, uint8_t> getEdgeSplit(Coordinates coords) const;
        
        std::tuple<uint64_t, uint8_t> getClosestSegment(Coordinates coords) const;
        
        // Returns the closest point on the segment to the given edge and the index of the segment
        Coordinates getClosestPointOnEdge(Coordinates coords, uint64_t edgeId, uint8_t segmentIndex) const;

    private:
        std::unique_ptr<Graph> mGraph;
        std::unique_ptr<Quadtree> mQuadtree;
        
        static double heuristic(const Node &a, const Node &b);
        void aStarRouting(uint64_t &startId, uint64_t &goalId, uint8_t snapToRoads = 0);
};