#pragma once

#include <memory>
#include <cstdint>
#include <vector>
#include <iostream>

#include "box.hpp"
#include "edge.hpp"
#include "graph.hpp"

class Quadtree
{
    public:
        Quadtree(const Graph &graph, const Box &boundary, uint8_t level = 0);

        void insert(uint64_t edgeId, uint8_t subwayId);

        const Box &getBoundary() const { return mBoundary; }

        std::ostream& operator<<(std::ostream& os) const;
        
        friend std::ostream& operator<<(std::ostream& os, const Quadtree& qt);

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

        std::vector<std::pair<uint64_t, uint8_t>> mEdgeSubwayIDs;

        bool subdivide();
};