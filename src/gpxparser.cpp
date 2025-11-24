#include "gpxparser.hpp"

#include <filesystem>
#include <iostream>
#include <pugixml.hpp>

#include "library.hpp"

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

    std::cout << "Number of points: " << mTrackPoints.size() << std::endl;
}

void GPXParser::fillEdgeIDs(const Graph &graph)
{
    uint64_t index = 0;
    for(Coordinates point : mTrackPoints)
    {
        auto [coords, edgeID, _] = graph.getEdgeSplit(point);
        if(HelperFunctions::haversine(point, coords) < 10)
        {
            mEdgeIDs.emplace_back(edgeID);
            std::cout << index << ": " << edgeID << std::endl;
        }
        else
        {
            std::cout << index << ": not found. " << point << std::endl;
        }
        index++;
    }
}

void GPXParser::parseGPXFile(const std::filesystem::directory_entry &file)
{
    pugi::xml_document doc;
    
    if(doc.load_file(file.path().string().c_str()))
    {
        for(const auto &trackpoint : doc.select_nodes("/gpx/trk/trkseg/trkpt"))
        {
            Coordinates point(trackpoint.node().attribute("lat").as_double(), trackpoint.node().attribute("lon").as_double());
            mTrackPoints.emplace_back(point);
        }
    }
}
