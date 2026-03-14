#include "router.hpp"

#include <ankerl/unordered_dense.h>
#include <algorithm>

#include "library.hpp"

Router::Router(std::string osmFile)
{
    ankerl::unordered_dense::map<uint64_t, std::shared_ptr<OsmNode>> nodes;
    ankerl::unordered_dense::map<uint64_t, std::unique_ptr<OsmWay>> ways;

    mGraph = std::make_unique<Graph>();

    HelperFunctions::readOSMFile(osmFile, nodes, ways);
    Box boundary = HelperFunctions::createGraph(*mGraph, nodes, ways);

    mQuadtree = std::make_unique<Quadtree>(*mGraph, boundary);
}

std::vector<std::tuple<uint64_t, Coordinates>> Router::aStar(uint64_t startId, uint64_t goalId, uint8_t snapToRoads)
{
    // Alle Knoten zurücksetzen (für wiederholte Nutzung)
    for (auto &[id, node] : mGraph->getNodes())
    {
        node->g = std::numeric_limits<double>::infinity();
        node->f = std::numeric_limits<double>::infinity();
        node->visited = false;
        node->parent = 0;
        node->parentEdge = nullptr;
        node->parentEdgeReversed = false;
    }

    std::shared_ptr<Node> start = mGraph->getNodes().at(startId);
    std::shared_ptr<Node> goal = mGraph->getNodes().at(goalId);

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

            if (neighbor->visited)
                continue;

            double tentativeG = current->g + edge->getWayLength();

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

std::vector<std::tuple<uint64_t, Coordinates>> Router::aStar(Coordinates startCoords, Coordinates goalCoords, uint8_t snapToRoads)
{
    auto [closestStartPoint, closestStartEdgeId, startSegment] = getEdgeSplit(startCoords);
    uint64_t newNodeIdStart = mGraph->addSplit(closestStartPoint, closestStartEdgeId, startSegment);

    auto [closestEndPoint, closestEndEdgeId, endSegment] = getEdgeSplit(goalCoords);
    uint64_t newNodeIdEnd = mGraph->addSplit(closestEndPoint, closestEndEdgeId, endSegment);

    std::vector<std::tuple<uint64_t, Coordinates>> path = aStar(newNodeIdStart, newNodeIdEnd, snapToRoads);

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
    double minDistance = std::numeric_limits<double>::infinity();
    uint64_t closestEdgeId = 0;
    uint8_t segmentIndex;

    Edge *closestEdge = mQuadtree->getClosestEdges(coords).front().edge;

    const auto &path = closestEdge->getPath();
    for (size_t i = 0; i < path.size() - 1; ++i)
    {
        double distance = HelperFunctions::distancePointToSegment(coords, path[i], path[i + 1]);
        if (distance < minDistance)
        {
            minDistance = distance;
            closestEdgeId = closestEdge->getId();
            segmentIndex = i;
        }
    }

    return std::tuple(closestEdgeId, segmentIndex);
}