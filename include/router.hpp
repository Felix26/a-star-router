#pragma once

#include <string>
#include <memory>

#include "graph.hpp"
#include "quadtree.hpp"

class Router
{
    public:
        Router(std::string osmFile);

        Graph &getGraph() { return *mGraph; }
        Quadtree &getQuadtree() { return *mQuadtree; }


    private:
        std::unique_ptr<Graph> mGraph;
        std::unique_ptr<Quadtree> mQuadtree;
};