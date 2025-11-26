#include "quadtree.hpp"

#include <queue>
#include <algorithm>

#include "library.hpp"

Quadtree::Quadtree(const Graph &graph, const Box &boundary, uint8_t level)
    : mGraph(graph), mBoundary(boundary), mLevel(level)
{
}

void Quadtree::insert(uint64_t edgeId, uint8_t subwayId)
{
    const Box edgeBox = mGraph.getEdge(edgeId)->getBoundingBox(subwayId);

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
        mEdgeSubwayIDs.emplace_back(edgeId, subwayId);

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
            mNorthWest->insert(edgeId, subwayId);
            return;
        }
        if(mNorthEast->getBoundary().contains(edgeBox))
        {
            mNorthEast->insert(edgeId, subwayId);
            return;
        }
        if(mSouthWest->getBoundary().contains(edgeBox))
        {
            mSouthWest->insert(edgeId, subwayId);
            return;
        }
        if(mSouthEast->getBoundary().contains(edgeBox))
        {
            mSouthEast->insert(edgeId, subwayId);
            return;
        }

        // Passt in keines der Kinder, also hier behalten
        mEdgeSubwayIDs.emplace_back(edgeId, subwayId);
    }
}

std::vector<ClosestEdges> Quadtree::getClosestEdges(const Coordinates &point, uint8_t resultCount) const
{
    using PQItem = ClosestEdges; // (distance, (edgeId, subwayId))
    std::priority_queue<PQItem, std::vector<PQItem>, std::less<PQItem>> closestEdges;
    closestEdges.push(PQItem{std::numeric_limits<double>::max(), 0, 0});

    for(const auto &[edgeId, subwayId] : mEdgeSubwayIDs)
    {
        auto edge = mGraph.getEdge(edgeId);
        double distance = HelperFunctions::distancePointToSegment(point,edge->getPath()[subwayId],edge->getPath()[subwayId + 1]);

        closestEdges.push(ClosestEdges{distance, edgeId, subwayId});
        if(closestEdges.size() > resultCount) closestEdges.pop();
    }

    // Recurse into children
    std::vector<Quadtree *> children = {mNorthWest.get(), mNorthEast.get(), mSouthWest.get(), mSouthEast.get()};
    for(auto child : children)
    {
        if(child != nullptr && child->getBoundary().getDistance(point) < closestEdges.top().distance)
        {
            auto childClosest = child->getClosestEdges(point, resultCount);
            for(const auto &ce : childClosest)
            {
                closestEdges.push(ce);
                if(closestEdges.size() > resultCount) closestEdges.pop();
            }
        }
    }

    std::vector<ClosestEdges> result;
    while(!closestEdges.empty())
    {
        result.push_back(closestEdges.top());
        closestEdges.pop();
    }

    std::reverse(result.begin(), result.end());
    return result;
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
