#include "edge.hpp"

#include "library.hpp"



double Edge::calculateWayLength() const
{
    std::vector<Coordinates> path;

    for(const auto node : mNodes)
    {
        path.push_back(node.get().getCoordinates());
    }

    return HelperFunctions::calculatePathLength(path);
}