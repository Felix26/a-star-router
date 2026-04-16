#include "edge.hpp"

#include <vector>
#include <memory>

#include "library.hpp"

Edge::Edge(uint64_t id, double waylength, std::shared_ptr<Node> from, std::shared_ptr<Node> to, std::vector<Coordinates> path, Tags tags)  : mId(id), mWaylength(waylength), mNodes{from, to}, mPath(std::move(path)), tags(tags)
{
}

double Edge::calculateWayLength() const
{
    double waylength = 0;
    for(uint8_t i = 0; i < mPath.size() - 1; i++)
    {
        waylength += HelperFunctions::haversine(mPath[i], mPath[i + 1]);
    }

    return waylength;
}
