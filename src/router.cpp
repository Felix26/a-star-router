#include "router.hpp"

#include <ankerl/unordered_dense.h>
#include <algorithm>

#include "library.hpp"
#include "weights.hpp"

Router::Router(const std::string &osmFile)
{
    ankerl::unordered_dense::map<uint64_t, std::shared_ptr<OsmNode>> nodes;
    ankerl::unordered_dense::map<uint64_t, std::unique_ptr<OsmWay>> ways;

    mGraph = std::make_unique<Graph>();

    HelperFunctions::readOSMFile(osmFile, nodes, ways);
    Box boundary = HelperFunctions::createGraph(*mGraph, nodes, ways);

    mQuadtree = std::make_unique<Quadtree>(*mGraph, boundary);
}

std::vector<std::tuple<uint64_t, Coordinates>> Router::aStar(uint64_t startId, uint64_t goalId, uint8_t snapToRoads, bool useWeighting)
{
    aStarRouting(startId, goalId, snapToRoads, useWeighting);

    // Pfad rekonstruieren
    std::vector<std::tuple<uint64_t, Coordinates>> path;
    for (uint64_t nodeId = goalId;; nodeId = mGraph->getNodes().at(nodeId)->parent)
    {
        if (nodeId == 0)
        {
            std::cerr << "No path found from " << startId << " to " << goalId << "\n";
            return std::vector<std::tuple<uint64_t, Coordinates>>(); // Kein Pfad gefunden
        }
        const Node &currentNode = *mGraph->getNodes().at(nodeId);
        path.emplace_back(nodeId, currentNode.getCoordinates());

        if (nodeId == startId)
            break;

        const Edge *edge = currentNode.parentEdge;
        if (edge == nullptr)
            continue;

        const auto &edgePath = edge->getPath();
        if (edgePath.size() <= 2)
            continue;

        if (!currentNode.parentEdgeReversed)
        {
            for (size_t pathIndex = edgePath.size() - 2; pathIndex > 0; --pathIndex)
            {
                path.emplace_back(0, edgePath.at(pathIndex));
            }
        }
        else
        {
            for (size_t pathIndex = 1; pathIndex < edgePath.size() - 1; ++pathIndex)
            {
                path.emplace_back(0, edgePath.at(pathIndex));
            }
        }
    }

    std::reverse(path.begin(), path.end());
    return path;
}

std::vector<Edge *> Router::aStarEdges(uint64_t startId, uint64_t goalId, bool useWeighting)
{
    aStarRouting(startId, goalId, 1, useWeighting);

    // Pfad rekonstruieren
    std::vector<Edge *> path;

    for (uint64_t nodeId = goalId;; nodeId = mGraph->getNodes().at(nodeId)->parent)
    {
        if (nodeId == 0)
        {
            std::cerr << "No path found from " << startId << " to " << goalId << "\n";
            return {};
        }

        if (nodeId == startId)
            break;

        Node &currentNode = *mGraph->getNodes().at(nodeId);
        Edge *edge = currentNode.parentEdge;

        if (edge != nullptr)
        {
            path.push_back(edge); 
        }
    }

    // Da wir den Pfad rückwärts (vom Ziel zum Start) abgelaufen sind,
    // müssen wir die Liste am Ende einmal umdrehen.
    std::reverse(path.begin(), path.end());

    return path;
}

void Router::aStarRouting(uint64_t &startId, uint64_t &goalId, uint8_t snapToRoads, bool useWeighting)
{
    currentEpoch++;

    auto prepareNode = [&](std::shared_ptr<Node>& node)
    {
        if (node->searchEpoch != currentEpoch)
        {
            node->g = std::numeric_limits<double>::infinity();
            node->f = std::numeric_limits<double>::infinity();
            node->visited = false;
            node->parent = 0;
            node->parentEdge = nullptr;
            node->parentEdgeReversed = false;
            node->searchEpoch = currentEpoch;
        }
    };

    std::shared_ptr<Node> start = mGraph->getNodes().at(startId);
    std::shared_ptr<Node> goal = mGraph->getNodes().at(goalId);

    prepareNode(start);
    prepareNode(goal);

    start->g = 0.0;
    start->f = Router::heuristic(*start, *goal) / (snapToRoads * (NO_EDGE_SNAP_PENALTY - 1) + 1);

    using PQItem = std::pair<double, uint64_t>; // (f, nodeId)
    std::priority_queue<PQItem, std::vector<PQItem>, std::greater<PQItem>> openSet;
    openSet.emplace(start->f, start->getId());

    while (!openSet.empty())
    {
        auto [currentF, currentId] = openSet.top();
        openSet.pop();

        std::shared_ptr<Node> current = mGraph->getNodes().at(currentId);

        if (current->visited)
            continue;
        current->visited = true;

        // Ziel erreicht?
        if (currentId == goalId)
            break;

        // Alle Nachbarn durchsuchen
        for (const auto &edge : current->edges)
        {
            auto fromNode = edge->from();
            auto toNode = edge->to();

            if (!fromNode || !toNode)
                continue;

            // Nächster Nachbar bestimmen
            std::shared_ptr<Node> neighbor = (fromNode->getId() == currentId) ? toNode : fromNode;
            prepareNode(neighbor);

            if (neighbor->visited)
                continue;

            double tentativeG = current->g + edge->getWeight();
            if (tentativeG < neighbor->g)
            {
                neighbor->parent = currentId;
                neighbor->parentEdge = edge.get();
                neighbor->parentEdgeReversed = (toNode->getId() == currentId);
                neighbor->g = tentativeG;
                neighbor->f = tentativeG + Router::heuristic(*neighbor, *goal) / (snapToRoads * (NO_EDGE_SNAP_PENALTY - 1) + 1);
                openSet.emplace(neighbor->f, neighbor->getId());
            }
        }
    }
}

std::vector<std::tuple<uint64_t, Coordinates>> Router::aStar(Coordinates startCoords, Coordinates goalCoords, uint8_t snapToRoads, bool useWeighting)
{
    auto [closestStartPoint, closestStartEdgeId, startSegment] = getEdgeSplit(startCoords);
    uint64_t newNodeIdStart = mGraph->addSplit(closestStartPoint, closestStartEdgeId, startSegment);

    auto [closestEndPoint, closestEndEdgeId, endSegment] = getEdgeSplit(goalCoords);
    uint64_t newNodeIdEnd = mGraph->addSplit(closestEndPoint, closestEndEdgeId, endSegment);

    std::vector<std::tuple<uint64_t, Coordinates>> path = aStar(newNodeIdStart, newNodeIdEnd, snapToRoads, useWeighting);

    mGraph->removeSplitItems();

    return path;
}

double Router::heuristic(const Node &a, const Node &b)
{
    return HelperFunctions::haversine(a.getCoordinates(), b.getCoordinates());
}

Coordinates Router::getClosestPointOnEdge(Coordinates coords, uint64_t edgeId, uint8_t segmentIndex) const
{
    const auto edge = mGraph->getEdges().at(edgeId);
    const auto &path = edge->getPath();

    return HelperFunctions::getProjectionOnSegment(coords, path[segmentIndex], path[segmentIndex + 1]);
}

std::tuple<Coordinates, uint64_t, uint8_t> Router::getEdgeSplit(Coordinates coords) const
{
    const auto [edgeId, segmentIndex] = getClosestSegment(coords);
    Coordinates closestPoint = getClosestPointOnEdge(coords, edgeId, segmentIndex);
    return std::tuple<Coordinates, uint64_t, uint8_t>(closestPoint, edgeId, segmentIndex);
}

std::tuple<uint64_t, uint8_t> Router::getClosestSegment(Coordinates coords) const
{
    auto closestEdges = mQuadtree->getClosestEdges(coords);

    for(uint64_t id : mGraph->getSplitItemIds())
    {

        if((id & 0x00FFFFFFFFFFFFFF) == (closestEdges[0].edge->getId() & 0x00FFFFFFFFFFFFFF))
        {
            double minDistance = std::numeric_limits<double>::infinity();
            auto &path = mGraph->getEdge(id)->getPath();
            uint8_t segmentIndex = 0;
            for (size_t i = 0; i < path.size() - 1; ++i)
            {
                double distance = HelperFunctions::distancePointToSegment(coords, path[i], path[i + 1]);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    segmentIndex = i;
                }
            }

            // If the closest edge is a split item and the distance is approximately the same as the original edge, return the split item
            if(std::abs(minDistance - closestEdges[0].distance) < 1e-4)
            {
                return std::tuple(id, segmentIndex);
            }
        }
    }

    // If the closest edge is not a split item, return the original edge and segment index
    return std::tuple(closestEdges[0].edge->getId(), closestEdges[0].subwayId);
}