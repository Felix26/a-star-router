#pragma once

#include <memory>
#include <cstdint>
#include <vector>
#include <iostream>
#include <queue>

#include "box.hpp"
#include "edge.hpp"
#include "graph.hpp"

struct ClosestEdges;

class Quadtree
{
    public:
        Quadtree(const Graph &graph, const Box &boundary, uint8_t level = 0);

        void insert(Edge *edge, uint8_t subwayId);

        const Box &getBoundary() const { return mBoundary; }

        std::vector<ClosestEdges> getClosestEdges(const Coordinates &point, uint8_t resultCount = 1) const;
        
        friend std::ostream& operator<<(std::ostream& os, const Quadtree& qt);

        const std::vector<Quadtree *> getAllSubtrees() const;

        const std::vector<std::pair<Edge *, uint8_t>> &getEdgeSubwayIDs() const { return mEdgeSubwayIDs; }

    private:
        const Graph &mGraph;

        std::unique_ptr<Quadtree> mNorthWest = nullptr;
        std::unique_ptr<Quadtree> mNorthEast = nullptr;
        std::unique_ptr<Quadtree> mSouthWest = nullptr;
        std::unique_ptr<Quadtree> mSouthEast = nullptr;

        Box mBoundary;

        static const uint8_t MAX_ITEMS = 8;
        static const uint8_t MAX_LEVELS = 10;

        uint8_t mLevel;

        std::vector<std::pair<Edge *, uint8_t>> mEdgeSubwayIDs;

        void initQuadTree();

        bool subdivide();

        void findClosestEdges(const Coordinates &point, uint8_t resultCount,
                              std::priority_queue<ClosestEdges, std::vector<ClosestEdges>, std::less<ClosestEdges>> &closestEdges) const;
};

struct ClosestEdges
{
    double distance;
    Edge *edge;
    uint8_t subwayId;

    bool operator<(const ClosestEdges &other) const
    {
        return distance < other.distance;
    }
};