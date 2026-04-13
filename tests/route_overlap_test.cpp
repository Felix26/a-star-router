#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <cassert>

#include "router.hpp"
#include "routes.hpp"
#include "library.hpp"

int main()
{
    try
    {
        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/Karlsruhe_Region.osm";
        Router router(osmPath);

        Routes routeHandler(router);

        Path gpx1path = HelperFunctions::getGPXTrackPoints("/home/felixm/Nextcloud/Studienarbeit/Daten/230305-30_matched_auto.gpx");
        Path gpx2path = HelperFunctions::getGPXTrackPoints("/home/felixm/Nextcloud/Studienarbeit/Daten/230305-30_matched_hand.gpx");

        auto edgeSet1 = routeHandler.getEdgeSet(gpx1path);
        auto edgeSet2 = routeHandler.getEdgeSet(gpx2path);

        double coefficient = routeHandler.getJaccardCoefficient(edgeSet1, edgeSet2);

        std::cout << "Jaccard Coefficient: " << coefficient << ", Number of Edges: " << edgeSet1.size() << ", " << edgeSet2.size() << std::endl;
        HelperFunctions::saveEdgesAsGeoJSON(edgeSet1);
        HelperFunctions::saveEdgesAsGeoJSON(edgeSet2);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
}
