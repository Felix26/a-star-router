#include "osmway.hpp"

#include <vector>

#include "coordinates.hpp"
#include "library.hpp"

void OsmWay::addNode(std::shared_ptr<OsmNode> node)
{
    mNodes.push_back(node);
}
