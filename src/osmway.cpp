#include "osmway.hpp"

#include <vector>

#include "coordinates.hpp"
#include "library.hpp"

void OsmWay::addNode(OsmNode &node)
{
    mNodes.push_back(node);
}

double OsmWay::calculateWayLength() const
{
    std::vector<Coordinates> path;

    for(const auto node : mNodes)
    {
        path.push_back(node.get().getCoordinates());
    }

    return HelperFunctions::calculatePathLength(path);
}
