#include "graph.hpp"

#include <iostream>
#include <queue>
#include <vector>
#include <limits>
#include <unordered_map>
#include <algorithm>
#include <memory>

#include "library.hpp"
#include "edge.hpp"

void Graph::addOsmNode(std::shared_ptr<OsmNode> node)
{
    if(node->isEdge)
    {
        mNodes.emplace(node->getId(), Node(*node));
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
            Node &fromNode = mNodes.at(way->getNodes().at(startIndex)->getId());
            Node &toNode = mNodes.at(way->getNodes().at(index)->getId());

            std::vector<Coordinates> path;
            for(size_t pathIndex = startIndex; pathIndex <= index; pathIndex++)
            {
                path.push_back(way->getNodes()[pathIndex]->getCoordinates());
            }

            double waylength = HelperFunctions::calculatePathLength(path);

            // First sub-way gets to keep original ID; subsequent IDs use 8 bits for sub-way index, 56 bits for way ID are copied
            uint64_t wayId = way->getId() | (subWayId++ << 56);

            mEdges.emplace(wayId, Edge(wayId, waylength, fromNode, toNode, path));
            fromNode.edges.push_back(mEdges.at(wayId));
            toNode.edges.push_back(mEdges.at(wayId));

            startIndex = index;
        }
    }
}

void Graph::printGraph()
{
    std::cout << "EDGES:\n";

    for(const auto &edge : mEdges)
    {
        std::cout << "Edge ID: " << edge.first << ", Length: " << edge.second.calculateWayLength() << " mm\n";
    }

    std::cout << "\n\n\nNODES:\n";
    
    for(const auto &node : mNodes)
    {
        std::cout << "Node ID: " << node.first << ", Coordinates: " << node.second.getCoordinates() << ", Connected Edges: ";
        for(const auto &edge : node.second.edges)
        {
            std::cout << edge.get().getId() << " ";
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
        node.g = std::numeric_limits<double>::infinity();
        node.f = std::numeric_limits<double>::infinity();
        node.visited = false;
        node.parent = 0;
        node.parentEdge = nullptr;
        node.parentEdgeReversed = false;
    }

    Node &start = mNodes.at(startId);
    Node &goal = mNodes.at(goalId);

    start.g = 0.0;
    start.f = Graph::heuristic(start, goal);

    using PQItem = std::pair<double, uint64_t>; // (f, nodeId)
    std::priority_queue<PQItem, std::vector<PQItem>, std::greater<PQItem>> openSet;
    openSet.emplace(start.f, start.getId());

    while (!openSet.empty())
    {
        auto [currentF, currentId] = openSet.top();
        openSet.pop();

        Node &current = mNodes.at(currentId);

        if (current.visited)
            continue;
        current.visited = true;

        // Ziel erreicht?
        if (currentId == goalId)
            break;

        // Alle Nachbarn durchsuchen
        for (Edge &edge : current.edges)
        {
            // Nächster Nachbar bestimmen
            Node &neighbor = (edge.from().get().getId() == currentId)
                                 ? edge.to().get()
                                 : edge.from().get();

            if (neighbor.visited)
                continue;

            double tentativeG = current.g + edge.calculateWayLength();

            if (tentativeG < neighbor.g)
            {
                neighbor.parent = currentId;
                neighbor.parentEdge = &edge;
                neighbor.parentEdgeReversed = (edge.to().get().getId() == currentId);
                neighbor.g = tentativeG;
                neighbor.f = tentativeG + Graph::heuristic(neighbor, goal);
                openSet.emplace(neighbor.f, neighbor.getId());
            }
        }
    }

    // Pfad rekonstruieren
    std::vector<std::tuple<uint64_t, Coordinates>> path;
    for (uint64_t nodeId = goalId;; nodeId = mNodes.at(nodeId).parent)
    {
        if (nodeId == 0)
        {
            std::cerr << "No path found from " << startId << " to " << goalId << "\n";
            return std::vector<std::tuple<uint64_t, Coordinates>>(); // Kein Pfad gefunden
        }
        const Node &currentNode = mNodes.at(nodeId);
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
