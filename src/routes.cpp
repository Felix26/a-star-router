#include "routes.hpp"

#include <algorithm>

#include "library.hpp"

std::vector<uint64_t> Routes::getEdgeSet(std::vector<Coordinates> &coordinates)
{
    std::vector<uint64_t> routingIds;
    std::vector<uint64_t> edgeIds;

    for(auto coords : coordinates)
    {
        auto closestEdge = mRouter.getQuadtree().getClosestEdges(coords)[0];
        auto edge = closestEdge.edge;

        double distanceFrom = HelperFunctions::haversine(coords, edge->from()->getCoordinates());
        double distanceTo = HelperFunctions::haversine(coords, edge->to()->getCoordinates());

        if(std::min(distanceFrom, distanceTo) < 2) routingIds.emplace_back(distanceFrom < distanceTo ? edge->from()->getId() : edge->to()->getId());
    }

    for(uint32_t i = 0; i < routingIds.size() - 1; i++)
    {
        const auto &path = mRouter.aStarEdges(routingIds[i], routingIds[i + 1]);
        edgeIds.insert(edgeIds.end(), path.begin(), path.end());
    }

    // emplace first and last edge because they are not included in the routing process
    edgeIds.emplace_back(mRouter.getQuadtree().getClosestEdges(coordinates[0])[0].edge->getId());
    edgeIds.emplace_back(mRouter.getQuadtree().getClosestEdges(coordinates[coordinates.size() - 1])[0].edge->getId());

    std::sort(edgeIds.begin(), edgeIds.end());
    edgeIds.erase(std::unique(edgeIds.begin(), edgeIds.end()), edgeIds.end());

    return edgeIds;
}
