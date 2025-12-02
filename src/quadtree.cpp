#include "quadtree.hpp"

#include <queue>
#include <algorithm>
#include <array>
#include <cmath>

#include "library.hpp"

Quadtree::Quadtree(const Graph &graph, const Box &boundary, uint8_t level)
    : mGraph(graph), mBoundary(boundary), mLevel(level)
{
    if(level == 0) initQuadTree();
}

void Quadtree::initQuadTree()
{
    for(const auto &[edgeId, edge] : mGraph.getEdges())
    {
        const auto &path = edge->getPath();
        for(size_t subWayId = 0; subWayId < path.size() - 1; subWayId++)
        {
            insert(edge.get(), subWayId);
        }
    }
}

const std::vector<Quadtree *> Quadtree::getAllSubtrees() const
{
    std::vector<Quadtree *> subtrees;
    subtrees.push_back(const_cast<Quadtree *>(this));

    if (mNorthWest) {
        auto nwSubtrees = mNorthWest->getAllSubtrees();
        subtrees.insert(subtrees.end(), nwSubtrees.begin(), nwSubtrees.end());
    }
    if (mNorthEast) {
        auto neSubtrees = mNorthEast->getAllSubtrees();
        subtrees.insert(subtrees.end(), neSubtrees.begin(), neSubtrees.end());
    }
    if (mSouthWest) {
        auto swSubtrees = mSouthWest->getAllSubtrees();
        subtrees.insert(subtrees.end(), swSubtrees.begin(), swSubtrees.end());
    }
    if (mSouthEast) {
        auto seSubtrees = mSouthEast->getAllSubtrees();
        subtrees.insert(subtrees.end(), seSubtrees.begin(), seSubtrees.end());
    }

    return subtrees;
}

void Quadtree::insert(Edge *edge, uint8_t subwayId)
{
    const Box edgeBox = edge->getBoundingBox(subwayId);

    if (!mBoundary.contains(edgeBox))
    {
        return; // The edge does not belong in this quadtree node
    }

    /*
    Hat der Knoten Kinder?
    Nein -> Item hier ablegen
    Ja ->
        Passt das Item vollständig in GENAU EIN Child?
            Ja  -> dort einfügen
            Nein -> Item hier behalten
    */
    if(mNorthWest == nullptr)
    {
        mEdgeSubwayIDs.emplace_back(edge, subwayId);

        if(mEdgeSubwayIDs.size() > MAX_ITEMS)
        {
            if(subdivide())
            {
                // Alle Items neu verteilen
                auto items = mEdgeSubwayIDs;
                mEdgeSubwayIDs.clear();

                for(const auto &[eId, sId] : items)
                {
                    insert(eId, sId);
                }
            }
        }
    }
    else
    {
        if(mNorthWest->getBoundary().contains(edgeBox))
        {
            mNorthWest->insert(edge, subwayId);
            return;
        }
        if(mNorthEast->getBoundary().contains(edgeBox))
        {
            mNorthEast->insert(edge, subwayId);
            return;
        }
        if(mSouthWest->getBoundary().contains(edgeBox))
        {
            mSouthWest->insert(edge, subwayId);
            return;
        }
        if(mSouthEast->getBoundary().contains(edgeBox))
        {
            mSouthEast->insert(edge, subwayId);
            return;
        }

        // Passt in keines der Kinder, also hier behalten
        mEdgeSubwayIDs.emplace_back(edge, subwayId);
    }
}

std::vector<ClosestEdges> Quadtree::getClosestEdges(const Coordinates &point, uint8_t resultCount) const
{
    std::priority_queue<ClosestEdges, std::vector<ClosestEdges>, std::less<ClosestEdges>> closestEdges;
    closestEdges.push(ClosestEdges{std::numeric_limits<double>::max(), 0, 0});

    findClosestEdges(point, resultCount, closestEdges);

    std::vector<ClosestEdges> result;
    while(!closestEdges.empty())
    {
        result.push_back(ClosestEdges{std::sqrt(closestEdges.top().distance), closestEdges.top().edge, closestEdges.top().subwayId});
        closestEdges.pop();
    }

    // The priority queue returns elements in descending order (largest distance first),
    // so we reverse the result to get ascending order (smallest distance first).
    std::reverse(result.begin(), result.end());
    return result;
}

void Quadtree::findClosestEdges(const Coordinates &point, uint8_t resultCount, std::priority_queue<ClosestEdges, std::vector<ClosestEdges>, std::less<ClosestEdges>> &closestEdges) const
{
    for(const auto &[edge, subwayId] : mEdgeSubwayIDs)
    {
        // Early skip if bounding box distance is already larger than the farthest closest edge found
        if(edge->getBoundingBox(subwayId).getEstDistanceSquared(point) >= closestEdges.top().distance && closestEdges.size() >= resultCount)
        {
            continue;
        }

        double distance = HelperFunctions::distancePointToSegment(point,edge->getPath()[subwayId],edge->getPath()[subwayId + 1]);
        distance = distance * distance;

        if(distance >= closestEdges.top().distance && closestEdges.size() >= resultCount) continue;
        closestEdges.push(ClosestEdges{distance, edge, subwayId});
        if(closestEdges.size() > resultCount) closestEdges.pop();
    }

    // Recurse into children
    std::array<std::pair<double, Quadtree *>, 4> children = {{
        {(mNorthWest != nullptr) ? mNorthWest->getBoundary().getEstDistanceSquared(point) : std::numeric_limits<double>::max(), mNorthWest.get()},
        {(mNorthEast != nullptr) ? mNorthEast->getBoundary().getEstDistanceSquared(point) : std::numeric_limits<double>::max(), mNorthEast.get()},
        {(mSouthWest != nullptr) ? mSouthWest->getBoundary().getEstDistanceSquared(point) : std::numeric_limits<double>::max(), mSouthWest.get()},
        {(mSouthEast != nullptr) ? mSouthEast->getBoundary().getEstDistanceSquared(point) : std::numeric_limits<double>::max(), mSouthEast.get()} }};

    std::sort(children.begin(), children.end(),
              [](const std::pair<double, Quadtree *> &a, const std::pair<double, Quadtree *> &b)
              {
                  return a.first < b.first;
              });

    for(auto child : children)
    {
        if(child.second == nullptr || child.first >= closestEdges.top().distance)
        {
            break;
        }

        child.second->findClosestEdges(point, resultCount, closestEdges);
    }
}


std::ostream& operator<<(std::ostream& os, const Quadtree& qt)
{
    if(qt.mEdgeSubwayIDs.size() > 0) os << "Level: " << static_cast<int>(qt.mLevel)
       << ", Boundary: [" << qt.mBoundary.getTopLeft()
       << " to " << qt.mBoundary.getBottomRight()
       << "], Items: " << qt.mEdgeSubwayIDs.size() << "\n";

    if (qt.mNorthWest != nullptr)
    {
        os << (qt.mNorthWest->mEdgeSubwayIDs.size() > 0 ? "NW -> " : "") << *qt.mNorthWest;
        os << (qt.mNorthEast->mEdgeSubwayIDs.size() > 0 ? "NE -> " : "") << *qt.mNorthEast;
        os << (qt.mSouthWest->mEdgeSubwayIDs.size() > 0 ? "SW -> " : "") << *qt.mSouthWest;
        os << (qt.mSouthEast->mEdgeSubwayIDs.size() > 0 ? "SE -> " : "") << *qt.mSouthEast;
    }

    return os;
}

bool Quadtree::subdivide()
{
    if(mLevel + 1 > MAX_LEVELS) return false;

    mNorthWest = std::make_unique<Quadtree>(mGraph, Box(mBoundary.getTopLeft(), mBoundary.getCenter()), mLevel + 1);
    mNorthEast = std::make_unique<Quadtree>(mGraph, Box(mBoundary.getTopMiddle(), mBoundary.getRightMiddle()), mLevel + 1);
    mSouthWest = std::make_unique<Quadtree>(mGraph, Box(mBoundary.getLeftMiddle(), mBoundary.getBottomMiddle()), mLevel + 1);
    mSouthEast = std::make_unique<Quadtree>(mGraph, Box(mBoundary.getCenter(), mBoundary.getBottomRight()), mLevel + 1);

    return true;
}
