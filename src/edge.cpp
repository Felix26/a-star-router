#include "edge.hpp"

#include <vector>
#include <memory>

Edge::Edge(uint64_t id, double waylength, std::shared_ptr<Node> from, std::shared_ptr<Node> to, std::vector<Coordinates> path)  : mId(id), mWaylength(waylength), mNodes{from, to}, mPath(std::move(path))
{
}
