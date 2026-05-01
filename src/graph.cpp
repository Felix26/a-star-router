#include "graph.hpp"

#include <iostream>
#include <queue>
#include <vector>
#include <limits>
#include <unordered_map>
#include <memory>
#include <tuple>
#include <algorithm>

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
    double waylength1percentage = HelperFunctions::calculatePathLength(path1) / edge->calculateWayLength();
    double waylength1 = waylength1percentage * edge->getWeight();

    uint64_t edgeId1 = edgeId | ((uint64_t)++mSplitItemCount << 60); // New sub-way ID
    auto edge1 = std::make_shared<Edge>(edgeId1, waylength1, edge->from(), newNode, path1);
    mEdges.emplace(edgeId1, edge1);
    mSplitItemIds.push_back(edgeId1);

    edge->from()->edges.push_back(edge1);
    newNode->edges.push_back(edge1);


    std::vector<Coordinates> path2;
    path2.push_back(closestCoords);
    path2.insert(path2.end(), path.begin() + segmentIndex + 1, path.end());
    double waylength2percentage = 1 - waylength1percentage;
    double waylength2 = waylength2percentage * edge->getWeight();

    uint64_t edgeId2 = edgeId | ((uint64_t)++mSplitItemCount << 60); // New sub-way ID
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
        // 1. Edges aufräumen
        auto edgeIt = mEdges.find(id);
        if (edgeIt != mEdges.end())
        {
            auto edge = edgeIt->second; // Den shared_ptr kurz sichern

            // Kante aus Startknotens (from) entfernen
            if (edge->from()) {
                auto &fromEdges = edge->from()->edges;
                fromEdges.erase(std::remove(fromEdges.begin(), fromEdges.end(), edge), fromEdges.end());
            }

            // Kante aus Zielknotens (to) entfernen
            if (edge->to()) {
                auto &toEdges = edge->to()->edges;
                toEdges.erase(std::remove(toEdges.begin(), toEdges.end(), edge), toEdges.end());
            }

            mEdges.erase(edgeIt);
            continue;
        }

        // 2. Nodes aufräumen
        auto nodeIt = mNodes.find(id);
        if (nodeIt != mNodes.end())
        {
            nodeIt->second->edges.clear(); 
            
            mNodes.erase(nodeIt);
        }
    }
    
    mSplitItemIds.clear();
    mSplitItemCount = 0;
}

void Graph::printGraph()
{
    std::cout << "EDGES:\n";

    for(const auto &edge : mEdges)
    {
        std::cout << "Edge ID: " << edge.first << ", Length: " << edge.second->getWeight() << " mm\n";
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
