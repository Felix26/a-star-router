#include "gpxparser.hpp"

#include <iostream>
#include <format>

#include "router.hpp"
#include "library.hpp"

int main()
{
    try
    {
        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/karlsruhe_roads_min.osm";

        Box boundary(Coordinates(49.73600, 7.949946), Coordinates(48.31047, 9.605534));
        Router router(osmPath);
        
        GPXParser parser;

        parser.loadGPXFiles(std::string(PROJECT_SOURCE_DIR) + "/testdata/gpxdata");

        for(const auto &track : parser.getTracks())
        {
            std::cout << std::format("Track: {}, Number of points: {}\n", std::get<0>(track), std::get<1>(track).size());
        }

        auto &trackPoints = parser.getTracks().at("230305-30.gpx");

        const auto &projections = parser.fillEdgeIDs(router, trackPoints);

        std::cout << "Closest edges to track points:\n";
        for(const auto &edgeId : parser.getEdgeIDs())
        {
            std::cout << std::format("Edge ID: {}, Distance: {}\n", std::get<0>(edgeId), std::get<1>(edgeId));
        }

        std::cout << HelperFunctions::exportPathToGeoJSON(projections, std::string(PROJECT_SOURCE_DIR) + "/testdata/projections.geojson") << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
    return 0;
}