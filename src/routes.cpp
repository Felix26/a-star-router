#include "routes.hpp"

#include <algorithm>

#include "library.hpp"

std::vector<Edge *> Routes::getEdgeSet(const std::vector<Coordinates> &coordinates)
{
    std::vector<uint64_t> routingIds;
    std::vector<Edge *> edges;
    std::vector<Edge *> modifiedEdges;

    for(auto coords : coordinates)
    {
        auto closestEdge = mRouter.getQuadtree().getClosestEdges(coords)[0];
        auto edge = closestEdge.edge;

        double distanceFrom = HelperFunctions::haversine(coords, edge->from()->getCoordinates());
        double distanceTo = HelperFunctions::haversine(coords, edge->to()->getCoordinates());

        if(std::min(distanceFrom, distanceTo) < 2)
        {
            routingIds.emplace_back(distanceFrom < distanceTo ? edge->from()->getId() : edge->to()->getId());
            
            if(routingIds.size() > 1)
            {
                const auto &path = mRouter.aStarEdges(routingIds[routingIds.size() - 2], routingIds[routingIds.size() - 1]);
                edges.insert(edges.end(), path.begin(), path.end());

                for(const auto edge : modifiedEdges)
                {
                    edge->setWeight(edge->calculateWayLength());
                }
                modifiedEdges.clear();
            }
        }
        else
        {
            edge->setWeight(edge->getWeight() / NO_EDGE_SNAP_PENALTY);
            modifiedEdges.emplace_back(edge);
        }
    }

    // emplace first and last edge because they are not included in the routing process
    edges.push_back(mRouter.getQuadtree().getClosestEdges(coordinates[0])[0].edge);
    edges.push_back(mRouter.getQuadtree().getClosestEdges(coordinates[coordinates.size() - 1])[0].edge);

    prepareEdgeSet(edges);

    return edges;
}

void Routes::prepareEdgeSet(std::vector<Edge *> &edges)
{
    std::sort(edges.begin(), edges.end(), [](const Edge* a, const Edge* b)
    {
        return a->getId() < b->getId();
    });

    
    edges.erase(std::unique(edges.begin(), edges.end(), [](const Edge* a, const Edge* b)
    {
        return a->getId() == b->getId();
    }), edges.end());
}

double Routes::getJaccardCoefficient(const std::vector<Edge *> &edges1, const std::vector<Edge *> &edges2)
{
    if (edges1.empty() && edges2.empty()) 
        return 1.0; 

    double intersectionLength = 0.0;
    double unionLength = 0.0;
    
    auto it1 = edges1.begin();
    auto it2 = edges2.begin();

    while (it1 != edges1.end() && it2 != edges2.end())
    {
        if ((*it1)->getId() < (*it2)->getId()) 
        { 
            /*std::cout << "--- Abweichung gefunden ---\n"
                      << "Route 1 schaut auf: Edge " << (*it1)->getId() << " (Länge: " << (*it1)->getWeight() << ")\n"
                      << "Route 2 schaut auf: Edge " << (*it2)->getId() << " (Länge: " << (*it2)->getWeight() << ")\n"
                      << "-> Edge " << (*it1)->getId() << " fehlt in Route 2. Wird zur Union addiert.\n\n";*/
                      
            unionLength += (*it1)->getWeight();
            ++it1;
        }
        else if ((*it2)->getId() < (*it1)->getId()) 
        { 
            /*std::cout << "--- Abweichung gefunden ---\n"
                      << "Route 1 schaut auf: Edge " << (*it1)->getId() << " (Länge: " << (*it1)->getWeight() << ")\n"
                      << "Route 2 schaut auf: Edge " << (*it2)->getId() << " (Länge: " << (*it2)->getWeight() << ")\n"
                      << "-> Edge " << (*it2)->getId() << " fehlt in Route 1. Wird zur Union addiert.\n\n";*/
                      
            unionLength += (*it2)->getWeight();
            ++it2; 
        }
        else // id1 == id2
        {
            intersectionLength += (*it1)->getWeight();
            unionLength += (*it1)->getWeight();
            
            ++it1;
            ++it2;
        }
    }

    // Falls ein Vektor länger war als der andere, die restlichen Elemente zur Union addieren
    while (it1 != edges1.end()) 
    {
        unionLength += (*it1)->getWeight();
        ++it1;
    }
    
    while (it2 != edges2.end()) 
    {
        unionLength += (*it2)->getWeight();
        ++it2;
    }

    if (unionLength <= 0.0) 
        return 0.0;

    return intersectionLength / unionLength;
}

std::unordered_map<std::string, std::unordered_map<std::string, double>> Routes::getParameterLengthMap(const std::vector<Edge *> &edges)
{
    std::unordered_map<std::string, std::unordered_map<std::string, double>> parameterLengthMap;

    for(const auto edge : edges)
    {
        const auto &parameters = edge->getParameters().getParameters();
        double edgeLength = edge->getWeight();

        for(const auto &[key, value] : parameters)
        {
            std::string parameterKeyValue = key + "=" + value;
            parameterLengthMap[key][value] += edgeLength;
        }
    }

    return parameterLengthMap;
}