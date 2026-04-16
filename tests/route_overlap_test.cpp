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

        Path gpx1path = HelperFunctions::getGPXTrackPoints(std::string(PROJECT_SOURCE_DIR) + "/testdata/Daten/230305-30_matched_hand.gpx")[0];
        Path gpx2path = HelperFunctions::getGPXTrackPoints(std::string(PROJECT_SOURCE_DIR) + "/testdata/Daten/230305-30_matched_auto.gpx")[0];

        auto edgeSet1 = routeHandler.getEdgeSet(gpx1path);
        auto edgeSet2 = routeHandler.getEdgeSet(gpx2path);

        double coefficientIdentical = routeHandler.getJaccardCoefficient(edgeSet1, edgeSet1);
        double coefficientDifferent = routeHandler.getJaccardCoefficient(edgeSet1, edgeSet2);

        assert(std::abs(coefficientIdentical - 1) < 0.0001);
        assert(std::abs(coefficientDifferent - 1) > 0.0001);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
}
