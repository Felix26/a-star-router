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
        checkRoutingTrackPoints(closestEdges, routingPoints, i, trackPoints[i]);
        closestEdges[0].edge->bestSnapPointCounter++;
        for(const auto &edge : closestEdges)
        {
            if(edge.distance < 20)
            {
                mEdges.insert(edge.edge);

                double oldWayLength = edge.edge->calculateWayLength();
                double newWayLength = (oldWayLength * (edge.distance + 1)) / NO_EDGE_SNAP_PENALTY;

                if(edge.edge->snapPointCounter == 0)
                {
                    edge.edge->setWayLength(newWayLength);
                    edge.edge->snapPointCounter++;
                }
                else
                {
                    edge.edge->setWayLength((edge.edge->getWayLength() * edge.edge->snapPointCounter + newWayLength) / (edge.edge->snapPointCounter + 1));
                    edge.edge->snapPointCounter++;
                }
            }
        }
    }
    routingPoints.emplace_back(trackPoints[trackPoints.size() - 1]);

    double expectedMetersPerPoint = 3;

    for(const auto &edge : mEdges)
    {
        double oldWayLength = edge->calculateWayLength();
        double snapPenaltyFactor = std::max(1.0, oldWayLength / (std::max<double>(edge->bestSnapPointCounter, 0.01) * expectedMetersPerPoint));
        if(oldWayLength < 10 * expectedMetersPerPoint)
        {
            continue;
        }
        edge->setWayLength(edge->getWayLength() * snapPenaltyFactor);
    }

    for(uint32_t i = 0; i < routingPoints.size() - 1; i++)
    {
        auto path = router.aStar(routingPoints[i], routingPoints[i + 1], 1);
        std::cout << routingPoints[i] << std::endl;
        projections.insert(projections.end(), path.begin(), path.end());
    }

    return projections;
}

void GPXParser::parseGPXFile(const std::filesystem::directory_entry &file)
{
    pugi::xml_document doc;
    
    if(doc.load_file(file.path().string().c_str()))
    {
        std::vector<Coordinates> trackPoints;
        for(const auto &trackpoint : doc.select_nodes("/gpx/trk/trkseg/trkpt"))
        {
            Coordinates point(trackpoint.node().attribute("lat").as_double(), trackpoint.node().attribute("lon").as_double());
            trackPoints.emplace_back(point);
        }

        mTracks.insert({file.path().filename().string(), trackPoints});
    }
}

void GPXParser::checkRoutingTrackPoints(const std::vector<ClosestEdges> &edges, std::vector<Coordinates> &routingTrackPoints, uint16_t pointIndex, const Coordinates &coordinates)
{
    if((pointIndex - lastPointIndex) < 50) return;

    double metric = 1 - edges[0].distance / std::max(edges[1].distance, 0.001);

    // If the second closest edge is very close, we reduce the metric to avoid snapping to wrong edges in dense areas
    metric *=  std::min(1.0, edges[1].distance / 20);

    if(metric > bestPointMetric)
    {
        bestPointMetric = metric;
        bestPointCoords = coordinates;
        bestPointIndex = pointIndex;
    }

    if((pointIndex - lastPointIndex) > 250 && bestPointMetric > 0.95)
    {
        std::cout << "Added routing point: " << bestPointCoords << " at index: " << bestPointIndex << " with metric: " << bestPointMetric << std::endl;
        routingTrackPoints.emplace_back(bestPointCoords);
        lastPointIndex = bestPointIndex;
        bestPointMetric = 0;
    }
}

void GPXParser::resetRoutingPoints()
{
    bestPointMetric = 0;
    bestPointIndex = 0;
    lastPointIndex = 0;
}
