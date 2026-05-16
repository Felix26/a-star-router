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
        Router router(osmPath, "weights.csv");

        Routes routeHandler(router);

        GPXParser parser;

        Path path210625_31 = HelperFunctions::getGPXTrackPoints("C:\\Users\\Felix\\Desktop\\router\\a-star-router\\testdata\\handmatched\\Originale\\210625-31.gpx")[0];
        Path path230305_30 = HelperFunctions::getGPXTrackPoints("C:\\Users\\Felix\\Desktop\\router\\a-star-router\\testdata\\handmatched\\Originale\\230305-30.gpx")[0];
        Path path231001_43 = HelperFunctions::getGPXTrackPoints("C:\\Users\\Felix\\Desktop\\router\\a-star-router\\testdata\\handmatched\\Originale\\231001-43.gpx")[0];
        Path path240127_07 = HelperFunctions::getGPXTrackPoints("C:\\Users\\Felix\\Desktop\\router\\a-star-router\\testdata\\handmatched\\Originale\\240127-07.gpx")[0];
        Path path241106_13 = HelperFunctions::getGPXTrackPoints("C:\\Users\\Felix\\Desktop\\router\\a-star-router\\testdata\\handmatched\\Originale\\241106-13.gpx")[0];

        auto edgeSet1 = routeHandler.getEdgeSet(parser.mapMatching(router, path210625_31));
        auto edgeSet2 = routeHandler.getEdgeSet(parser.mapMatching(router, path230305_30));
        auto edgeSet3 = routeHandler.getEdgeSet(parser.mapMatching(router, path231001_43));
        auto edgeSet4 = routeHandler.getEdgeSet(parser.mapMatching(router, path240127_07));
        auto edgeSet5 = routeHandler.getEdgeSet(parser.mapMatching(router, path241106_13));

        Path path210625_31_handmatched = HelperFunctions::getGPXTrackPoints("C:\\Users\\Felix\\Desktop\\router\\a-star-router\\testdata\\handmatched\\210625-31_handmatched.gpx")[0];
        Path path230305_30_handmatched = HelperFunctions::getGPXTrackPoints("C:\\Users\\Felix\\Desktop\\router\\a-star-router\\testdata\\handmatched\\230305-30_handmatched.gpx")[0];
        Path path231001_43_handmatched = HelperFunctions::getGPXTrackPoints("C:\\Users\\Felix\\Desktop\\router\\a-star-router\\testdata\\handmatched\\231001-43_handmatched.gpx")[0];
        Path path240127_07_handmatched = HelperFunctions::getGPXTrackPoints("C:\\Users\\Felix\\Desktop\\router\\a-star-router\\testdata\\handmatched\\240127-07_handmatched.gpx")[0];
        Path path241106_13_handmatched = HelperFunctions::getGPXTrackPoints("C:\\Users\\Felix\\Desktop\\router\\a-star-router\\testdata\\handmatched\\241106-13_handmatched.gpx")[0];

        auto edgeSet1m = routeHandler.getEdgeSet(path210625_31_handmatched);
        auto edgeSet2m = routeHandler.getEdgeSet(path230305_30_handmatched);
        auto edgeSet3m = routeHandler.getEdgeSet(path231001_43_handmatched);
        auto edgeSet4m = routeHandler.getEdgeSet(path240127_07_handmatched);
        auto edgeSet5m = routeHandler.getEdgeSet(path241106_13_handmatched);

        double coefficient1 = routeHandler.getJaccardCoefficient(edgeSet1, edgeSet1m);
        double coefficient2 = routeHandler.getJaccardCoefficient(edgeSet2, edgeSet2m);
        double coefficient3 = routeHandler.getJaccardCoefficient(edgeSet3, edgeSet3m);
        double coefficient4 = routeHandler.getJaccardCoefficient(edgeSet4, edgeSet4m);
        double coefficient5 = routeHandler.getJaccardCoefficient(edgeSet5, edgeSet5m);

        double average = coefficient1 + coefficient2 + coefficient3 + coefficient4 + coefficient5;
        average /= 5;

        std::cout << std::format("{:.3f}, {:.3f}, {:.3f}, {:.3f}, {:.3f}\n", coefficient1, coefficient2, coefficient3, coefficient4, coefficient5);
        std::cout << std::format("{:.4f}\n", average);

        HelperFunctions::saveEdgesAsGeoJSON(edgeSet3);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
}
