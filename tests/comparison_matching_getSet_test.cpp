#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <cassert>

#include "router.hpp"
#include "routes.hpp"
#include "library.hpp"
#include "gpxparser.hpp"

int main()
{
    try
    {
        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/Karlsruhe_Region.osm";
        Router router(osmPath, "weights_empty.csv");

        Routes routeHandler(router);

        GPXParser parser;

        Path path210625_31_handmatched = HelperFunctions::getGPXTrackPoints("C:\\Users\\Felix\\Desktop\\router\\a-star-router\\testdata\\handmatched\\210625-31_handmatched.gpx")[0];

        auto edgeSet1 = routeHandler.getEdgeSet(parser.mapMatching(router, path210625_31_handmatched));
        auto edgeSet1m = routeHandler.getEdgeSet(path210625_31_handmatched);

        double coefficient1 = routeHandler.getJaccardCoefficient(edgeSet1, edgeSet1m);

        std::cout << coefficient1 << std::endl;

        HelperFunctions::saveEdgesAsGeoJSON(edgeSet1);
        HelperFunctions::saveEdgesAsGeoJSON(edgeSet1m);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
}
