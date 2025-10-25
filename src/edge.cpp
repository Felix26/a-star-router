#include "edge.hpp"

#include <vector>

Edge::Edge(uint64_t id, double waylength, Node &from, Node &to, std::vector<Coordinates> path)  : mId(id), mWaylength(waylength), mNodes{std::ref(from), std::ref(to)}, mPath(std::move(path))
{
}
