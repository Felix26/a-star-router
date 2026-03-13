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

std::vector<std::tuple<uint64_t, Coordinates>> GPXParser::fillEdgeIDs(Router &router, const std::vector<Coordinates> &trackPoints)
{
    std::vector<std::tuple<uint64_t, Coordinates>> projections;

    for(Coordinates point : trackPoints)
    {
        auto closestEdges = router.getQuadtree().getClosestEdges(point, 3);
        for(const auto &edge : closestEdges)
        {
            if(edge.distance < 50)
            {
                mEdgeIDs.emplace_back(edge.edge->getId(), edge.distance);

                edge.edge->setWayLength((edge.edge->calculateWayLength() + edge.distance * 5) / NO_EDGE_SNAP_PENALTY);
            }
        }
    }

    // calculate path every 1000 track points
    for(uint32_t i = 0; i < trackPoints.size(); i += 1000)
    {
        auto path = router.aStar(trackPoints[i], trackPoints[std::min(i + 999, static_cast<uint32_t>(trackPoints.size() - 1))], 1);
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
