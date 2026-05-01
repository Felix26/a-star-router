#pragma once

#include <vector>

#include "router.hpp"
#include "coordinates.hpp"

class Routes
{
    public:
        Routes(Router &router) : mRouter(router) {}

        std::vector<Edge *> getEdgeSet(const std::vector<Coordinates> &coordinates);
        void prepareEdgeSet(std::vector<Edge *> &edges);

        double getJaccardCoefficient(const std::vector<Edge *> &edges1, const std::vector<Edge *> &edges2);

        std::unordered_map<std::string, std::unordered_map<std::string, double>> getParameterLengthMap(const std::vector<Edge *> &edges);

    private:
        Router &mRouter;
};