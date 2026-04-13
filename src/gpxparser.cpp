#include "gpxparser.hpp"

#include <filesystem>
#include <iostream>
#include <pugixml.hpp>
#include <algorithm>

#include "library.hpp"
#include "quadtree.hpp"
#include "graph.hpp"

GPXParser::GPXParser()
{
}

void GPXParser::loadGPXFiles(const std::string &directory)
{
    std::filesystem::path directoryPath{directory};

    if(!std::filesystem::exists(directoryPath))
    {
        throw(std::runtime_error("Directory: " + directory + " does not exists."));
    }

    for(const auto &file : std::filesystem::directory_iterator(directoryPath))
    {
        if(file.is_regular_file() && file.path().extension() == ".gpx")
        {
            parseGPXFile(file);
        }
    }
}

std::vector<std::tuple<uint64_t, Coordinates>> GPXParser::fillEdgeIDs(Router &router, const std::vector<Coordinates> &trackPoints)
{
    std::vector<std::tuple<uint64_t, Coordinates>> projections;
    std::vector<Coordinates> routingPoints;
    resetRoutingPoints();

    routingPoints.emplace_back(trackPoints[0]);
    for(uint16_t i = 1; i < trackPoints.size() - 1; i++)
    {
        auto closestEdges = router.getQuadtree().getClosestEdges(trackPoints[i], 3, false);
        closestEdges[0].edge->bestSnapPointCounter++;

        for(const auto &edge : closestEdges)
        {
            if(edge.distance < 20)
            {
                mEdges.insert(edge.edge);

                double pathLength = edge.edge->calculateWayLength();
                double newWeight = (pathLength * (edge.distance + 1)) / NO_EDGE_SNAP_PENALTY;
                
                edge.edge->setWeight((edge.edge->getWeight() * edge.edge->snapPointCounter + newWeight) / (edge.edge->snapPointCounter + 1));
                edge.edge->snapPointCounter++;
            }
        }

        if(checkRoutingTrackPoints(closestEdges, routingPoints, i, trackPoints[i]))
        {
            calculateSnapPenalties();
            bool success = doRouting(routingPoints[routingPoints.size() - 2], routingPoints[routingPoints.size() - 1], router, projections);
            if(!success)
            {
                std::cerr << "Routing failed between points: " << routingPoints[routingPoints.size() - 2] << " and " << routingPoints[routingPoints.size() - 1] << std::endl;
                routingPoints.pop_back();
            }
            else
            {
                reset();
            }

            // MIN_ROUTING_LENGTH points can be skipped, but only to the current index (so all points will be checked at least once)
            // Rechecking points can lead to better routing points (otherwise all points from bestPointindex + MIN_ROUTING_LENGTH to i would be skipped without checking for routing points)
            i = std::min<uint16_t>(i, bestPointIndex + MIN_ROUTING_LENGTH);
        }
    }

    routingPoints.emplace_back(trackPoints[trackPoints.size() - 1]);

    calculateSnapPenalties();
    doRouting(routingPoints[routingPoints.size() - 2], routingPoints[routingPoints.size() - 1], router, projections);
    reset();

    return projections;
}

void GPXParser::calculateSnapPenalties()
{
    double expectedMetersPerPoint = 3;

    for (const auto &edge : mEdges)
    {
        double pathLength = edge->calculateWayLength();
        double snapPenaltyWeight = HelperFunctions::logisticFunction(pathLength, 0, 1, 0.15, 7 * expectedMetersPerPoint);

        double snapPenaltyFactor = std::max(1.0, pathLength / (std::max<double>(edge->bestSnapPointCounter, 1) * expectedMetersPerPoint));
        double weigtedSnapPenaltyFactor = snapPenaltyFactor * snapPenaltyWeight + (1 - snapPenaltyWeight);
        edge->setWeight(edge->getWeight() * weigtedSnapPenaltyFactor);
    }
}

bool GPXParser::doRouting(const Coordinates &start, const Coordinates &end, Router &router, std::vector<std::tuple<uint64_t, Coordinates>> &pathContainer) const
{
    auto path = router.aStar(start, end, 1);
    pathContainer.insert(pathContainer.end(), path.begin(), path.end());

    return !path.empty();
}

void GPXParser::parseGPXFile(const std::filesystem::directory_entry &file)
{
    auto trackPoints = HelperFunctions::getGPXTrackPoints(file);
    
    if(!trackPoints.empty())
    {
        mTracks.insert({file.path().filename().string(), trackPoints});
    }
}

bool GPXParser::checkRoutingTrackPoints(const std::vector<ClosestEdges> &edges, std::vector<Coordinates> &routingTrackPoints, uint16_t pointIndex, const Coordinates &coordinates)
{
    if((pointIndex - lastPointIndex) < MIN_ROUTING_LENGTH) return false;

    double metric = 1 - edges[0].distance / std::max(edges[1].distance, 0.001);

    // If the second closest edge is very close, we reduce the metric to avoid snapping to wrong edges in dense areas
    metric *=  std::min(1.0, edges[1].distance / 20);

    if(metric > bestPointMetric)
    {
        bestPointMetric = metric;
        bestPointCoords = coordinates;
        bestPointIndex = pointIndex;
    }

    if((pointIndex - lastPointIndex) > MAX_ROUTING_LENGTH && bestPointMetric > 0.95)
    {
        std::cout << "Added routing point: " << bestPointCoords << " at index: " << bestPointIndex << " with metric: " << bestPointMetric << std::endl;
        routingTrackPoints.emplace_back(bestPointCoords);
        lastPointIndex = bestPointIndex;
        bestPointMetric = 0;

        return true;
    }

    return false;
}

void GPXParser::resetRoutingPoints()
{
    bestPointMetric = 0;
    bestPointIndex = 0;
    lastPointIndex = 0;
}

void GPXParser::reset()
{
    for(const auto &edge : mEdges)
    {
        edge->setWeight(edge->calculateWayLength());
        edge->snapPointCounter = 0;
        edge->bestSnapPointCounter = 0;
    }

    mEdges.clear();
}