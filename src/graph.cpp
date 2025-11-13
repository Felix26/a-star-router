#include "graph.hpp"

#include <iostream>
#include <queue>
#include <vector>
#include <limits>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <tuple>

#include "library.hpp"
#include "edge.hpp"

void Graph::addOsmNode(std::shared_ptr<OsmNode> node)
{
    if(node->isEdge)
    {
        mNodes.emplace(node->getId(), std::make_shared<Node>(*node));
    }
}

void Graph::addOsmWay(const OsmWay *way)
{
    size_t startIndex = 0;
    size_t subWayId = 0;

    // check all nodes in way if there are junctions; ignore first and last node
    for(size_t index = 1; index < way->getNodes().size(); index++)
    {
        // find nodes that are in the graph
        if(way->getNodes()[index]->isEdge)
        {
            // create edge between startIndex and index
            std::shared_ptr<Node> fromNode = mNodes.at(way->getNodes().at(startIndex)->getId());
            std::shared_ptr<Node> toNode = mNodes.at(way->getNodes().at(index)->getId());

            std::vector<Coordinates> path;
            for(size_t pathIndex = startIndex; pathIndex <= index; pathIndex++)
            {
                path.push_back(way->getNodes()[pathIndex]->getCoordinates());
            }

            double waylength = HelperFunctions::calculatePathLength(path);

            // First sub-way gets to keep original ID; subsequent IDs use 8 bits for sub-way index, 56 bits for way ID are copied
            uint64_t wayId = way->getId() | (subWayId++ << 56);

            mEdges.emplace(wayId, std::make_shared<Edge>(wayId, waylength, fromNode, toNode, path));
            fromNode->edges.push_back(mEdges.at(wayId));
            toNode->edges.push_back(mEdges.at(wayId));

            startIndex = index;
        }
    }
}

uint64_t Graph::addSplit(Coordinates closestCoords, uint64_t edgeId, uint8_t segmentIndex)
{
    const auto edge = mEdges.at(edgeId);
    const auto &path = edge->getPath();

    // Create new node at closest point
    uint64_t newNodeId = (uint64_t) - ++mSplitItemCount; // Generate unique ID for the new node
    auto newNode = std::make_shared<Node>(OsmNode(newNodeId, closestCoords.getLatitude(), closestCoords.getLongitude()));
    mNodes.emplace(newNodeId, newNode);
    mSplitItemIds.push_back(newNodeId);

    // Create two new edges by splitting the original polyline at the segment index
    std::vector<Coordinates> path1(path.begin(), path.begin() + segmentIndex + 1);
    path1.push_back(closestCoords);
    double waylength1 = HelperFunctions::calculatePathLength(path1);
    uint64_t edgeId1 = edgeId | ((uint64_t)++mSplitItemCount << 62); // New sub-way ID
    auto edge1 = std::make_shared<Edge>(edgeId1, waylength1, edge->from(), newNode, path1);
    mEdges.emplace(edgeId1, edge1);
    mSplitItemIds.push_back(edgeId1);
    edge->from()->edges.push_back(edge1);
    newNode->edges.push_back(edge1);

    std::vector<Coordinates> path2;
    path2.push_back(closestCoords);
    path2.insert(path2.end(), path.begin() + segmentIndex + 1, path.end());
    double waylength2 = HelperFunctions::calculatePathLength(path2);
    uint64_t edgeId2 = edgeId | ((uint64_t)++mSplitItemCount << 62); // New sub-way ID
    auto edge2 = std::make_shared<Edge>(edgeId2, waylength2, newNode, edge->to(), path2);
    mEdges.emplace(edgeId2, edge2);
    mSplitItemIds.push_back(edgeId2);
    newNode->edges.push_back(edge2);
    edge->to()->edges.push_back(edge2);

    return newNodeId;
}

void Graph::removeSplitItems()
{
    for (const auto &id : mSplitItemIds)
    {
        // Remove edges
        if (mEdges.find(id) != mEdges.end())
        {
            mEdges.erase(id);
            continue;
        }

        // Remove nodes
        if (mNodes.find(id) != mNodes.end())
        {
            mNodes.erase(id);
        }
    }
    mSplitItemIds.clear();
}

void Graph::printGraph()
{
    std::cout << "EDGES:\n";

    for(const auto &edge : mEdges)
    {
        std::cout << "Edge ID: " << edge.first << ", Length: " << edge.second->calculateWayLength() << " mm\n";
    }

    std::cout << "\n\n\nNODES:\n";
    
    for(const auto &node : mNodes)
    {
        std::cout << "Node ID: " << node.first << ", Coordinates: " << node.second->getCoordinates() << ", Connected Edges: ";
        for(const auto &edge : node.second->edges)
        {
            std::cout << edge->getId() << " ";
        }
        std::cout << "\n";
    }

    std::cout << "Edge Count: " << mEdges.size() << "\n";
    std::cout << "Node Count: " << mNodes.size() << "\n";
}

double Graph::heuristic(const Node &a, const Node &b)
{
    return HelperFunctions::haversine(a.getCoordinates(), b.getCoordinates());
}

std::vector<std::tuple<uint64_t, Coordinates>> Graph::aStar(uint64_t startId, uint64_t goalId)
{
    // Alle Knoten zurücksetzen (für wiederholte Nutzung)
    for (auto &[id, node] : mNodes)
    {
        node->g = std::numeric_limits<double>::infinity();
        node->f = std::numeric_limits<double>::infinity();
        node->visited = false;
        node->parent = 0;
        node->parentEdge = nullptr;
        node->parentEdgeReversed = false;
    }

    std::shared_ptr<Node> start = mNodes.at(startId);
    std::shared_ptr<Node> goal = mNodes.at(goalId);

    start->g = 0.0;
    start->f = Graph::heuristic(*start, *goal);

    using PQItem = std::pair<double, uint64_t>; // (f, nodeId)
    std::priority_queue<PQItem, std::vector<PQItem>, std::greater<PQItem>> openSet;
    openSet.emplace(start->f, start->getId());

    while (!openSet.empty())
    {
        auto [currentF, currentId] = openSet.top();
        openSet.pop();

        std::shared_ptr<Node> current = mNodes.at(currentId);

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

            double tentativeG = current->g + edge->calculateWayLength();

            if (tentativeG < neighbor->g)
            {
                neighbor->parent = currentId;
                neighbor->parentEdge = edge.get();
                neighbor->parentEdgeReversed = (toNode->getId() == currentId);
                neighbor->g = tentativeG;
                neighbor->f = tentativeG + Graph::heuristic(*neighbor, *goal);
                openSet.emplace(neighbor->f, neighbor->getId());
            }
        }
    }

    // Pfad rekonstruieren
    std::vector<std::tuple<uint64_t, Coordinates>> path;
    for (uint64_t nodeId = goalId;; nodeId = mNodes.at(nodeId)->parent)
    {
        if (nodeId == 0)
        {
            std::cerr << "No path found from " << startId << " to " << goalId << "\n";
            return std::vector<std::tuple<uint64_t, Coordinates>>(); // Kein Pfad gefunden
        }
        const Node &currentNode = *mNodes.at(nodeId);
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

std::vector<std::tuple<uint64_t, Coordinates>> Graph::aStar(Coordinates startCoords, Coordinates goalCoords)
{
    auto [closestStartPoint, closestStartEdgeId, startSegment] = getEdgeSplit(startCoords);
    uint64_t newNodeIdStart = addSplit(closestStartPoint, closestStartEdgeId, startSegment);

    auto [closestEndPoint, closestEndEdgeId, endSegment] = getEdgeSplit(goalCoords);
    uint64_t newNodeIdEnd = addSplit(closestEndPoint, closestEndEdgeId, endSegment);

    std::vector<std::tuple<uint64_t, Coordinates>> path = aStar(newNodeIdStart, newNodeIdEnd);

    removeSplitItems();

    return path;
}

std::tuple<Coordinates, uint64_t, uint8_t> Graph::getEdgeSplit(Coordinates coords)
{
    const auto [edgeId, segmentIndex] = getClosestSegment(coords);
    Coordinates closestPoint = getClosestPointOnEdge(coords, edgeId, segmentIndex);
    return std::tuple<Coordinates, uint64_t, uint8_t>(closestPoint, edgeId, segmentIndex);
}

Coordinates Graph::getClosestPointOnEdge(Coordinates coords, uint64_t edgeId, uint8_t segmentIndex)
{
    const auto edge = mEdges.at(edgeId);
    const auto &path = edge->getPath();

    return HelperFunctions::getProjectionOnSegment(coords, path[segmentIndex], path[segmentIndex + 1]);
}

std::tuple<uint64_t, uint8_t> Graph::getClosestSegment(Coordinates coords)
{
    double minDistance = std::numeric_limits<double>::infinity();
    uint64_t closestEdgeId = 0;
    uint8_t segmentIndex;

    for (const auto &[edgeId, edge] : mEdges)
    {
        const auto &path = edge->getPath();
        for (size_t i = 0; i < path.size() - 1; ++i)
        {
            double distance = HelperFunctions::distancePointToSegment(coords, path[i], path[i + 1]);
            if (distance < minDistance)
            {
                minDistance = distance;
                closestEdgeId = edgeId;
                segmentIndex = i;
            }
        }
    }

    return std::tuple(closestEdgeId, segmentIndex);
}
