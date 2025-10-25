#include "way.hpp"

#include <vector>

#include "coordinates.hpp"
#include "library.hpp"

void Way::addNode(Node &node)
{
    mNodes.push_back(node);
}

double Way::calculateWayLength() const
{
    std::vector<Coordinates> path;

    for(const auto node : mNodes)
    {
        path.push_back(node.get().getCoordinates());
    }

    return HelperFunctions::calculatePathLength(path);
}
