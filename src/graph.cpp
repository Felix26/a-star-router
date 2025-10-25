#include "graph.hpp"

#include <iostream>

#include "library.hpp"
#include "edge.hpp"

void Graph::addOsmNode(OsmNode &node)
{
    if(node.isEdge)
    {
        mNodes.emplace(node.getId(), Node(node));
    }
}

void Graph::addOsmWay(OsmWay &way)
{
    size_t startIndex = 0;
    size_t subWayId = 0;

    // check all nodes in way if there are junctions; ignore first and last node
    for(size_t index = 1; index < way.getNodes().size(); index++)
    {
        // find nodes that are in the graph
        if(way.getNodes().at(index).get().isEdge)
        {
            // create edge between startIndex and index
            Node &fromNode = mNodes.at(way.getNodes().at(startIndex).get().getId());
            Node &toNode = mNodes.at(way.getNodes().at(index).get().getId());

            std::vector<Coordinates> path;
            for(size_t pathIndex = startIndex; pathIndex <= index; pathIndex++)
            {
                path.push_back(way.getNodes().at(pathIndex).get().getCoordinates());
            }

            double waylength = HelperFunctions::calculatePathLength(path);

            // First sub-way gets to keep original ID; subsequent IDs use 4 bits for sub-way index, 60 bits for way ID are copied
            uint64_t wayId = way.getId() | (subWayId++ << 60);

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
