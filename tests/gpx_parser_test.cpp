#include "gpxparser.hpp"

#include <iostream>
#include <format>

#include "router.hpp"
#include "library.hpp"

int main()
{
    try
    {
        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/Karlsruhe_Region.osm";

        Router router(osmPath);

        auto node = router.getGraph().getNodes().find(1668593983);
        
        GPXParser parser;

        parser.loadGPXFiles(std::string(PROJECT_SOURCE_DIR) + "/testdata/gpxdata");

        for(const auto &track : parser.getTracks())
        {
            std::cout << std::format("Track: {}, Number of points: {}\n", std::get<0>(track), std::get<1>(track).size());
        }

        auto &trackPoints = parser.getTracks().at("240713-50.gpx");

        const auto &projections = parser.fillEdgeIDs(router, trackPoints);

        std::cout << "Closest edges to track points:\n";
        for(const auto &edgeId : parser.getEdgeIDs())
        {
            //std::cout << std::format("Edge ID: {}, Distance: {}\n", std::get<0>(edgeId), std::get<1>(edgeId));
        }

        std::cout << HelperFunctions::exportPathToGeoJSON(projections, std::string(PROJECT_SOURCE_DIR) + "/testdata/projections.geojson") << std::endl;

        // Eingabe per std::cin: Koordinaten, Ausgabe: Nächstgelegene Kante, Kantengewicht, Kantenlänge, Bonusfaktor
        while(0)
        {
            std::cout << "Enter Coordinates lat, lon to find the closest edge and to get its weight, length and bonus factor (or 'exit' to quit): ";
            std::string input;
            std::cin >> input;

            if(input == "exit")
                break;

            try
            {
                Coordinates coords(input);
                const Edge *edge = router.getQuadtree().getClosestEdges(coords)[0].edge;
                double weight = edge->getWayLength();;
                double length = edge->calculateWayLength();
                double bonusFactor = length / weight;

                std::cout << std::format("Edge ID: {} ({}), Weight: {}, Length: {}, Bonus Factor: {}\n", edge->getId(), edge->getId() & 0x00FFFFFFFFFFFFFF, weight, length, bonusFactor);
            }
            catch(const std::exception &e)
            {
                std::cerr << "Invalid input or edge not found. Please try again.\n";
            }
        }

    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
    return 0;
}