#include "gpxparser.hpp"

#include <filesystem>
#include <iostream>
#include <pugixml.hpp>

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

std::vector<std::tuple<uint64_t, Coordinates>> GPXParser::fillEdgeIDs(const Quadtree &quadtree, const std::vector<Coordinates> &trackPoints)
{
    std::vector<std::tuple<uint64_t, Coordinates>> projections;

    for(Coordinates point : trackPoints)
    {
        auto closestEdges = quadtree.getClosestEdges(point, 1);
        for(const auto &edge : closestEdges)
        {
            if(edge.distance < 50)
            {
                mEdgeIDs.emplace_back(edge.edge->getId(), edge.distance);

                Coordinates segmentStart = edge.edge->getPath()[edge.subwayId];
                Coordinates segmentEnd = edge.edge->getPath()[edge.subwayId + 1];

                projections.emplace_back(edge.edge->getId(), HelperFunctions::getProjectionOnSegment(point, segmentStart, segmentEnd));
            }
        }
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
