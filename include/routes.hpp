#pragma once

#include <vector>

#include "router.hpp"
#include "coordinates.hpp"

class Routes
{
    public:
        Routes(Router &router) : mRouter(router) {}

        std::vector<uint64_t> getEdgeSet(std::vector<Coordinates> &coordinates);

    private:
        Router &mRouter;
};