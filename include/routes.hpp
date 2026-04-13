#pragma once

#include <vector>

#include "router.hpp"
#include "coordinates.hpp"

class Routes
{
    public:
        Routes(Router &router) : mRouter(router) {}

        std::vector<Edge *> getEdgeSet(const std::vector<Coordinates> &coordinates);

        double getJaccardCoefficient(const std::vector<Edge *> &edges1, const std::vector<Edge *> &edges2);

    private:
        Router &mRouter;
};