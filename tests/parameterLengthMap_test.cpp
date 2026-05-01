#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <cassert>

#include "router.hpp"
#include "routes.hpp"

int main()
{
    try
    {
        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/karlsruhe_region.osm";
        Router router(osmPath);
        Routes routes(router);

        auto route1 = router.aStar(Coordinates(49.02583, 8.35493), Coordinates(48.98776, 8.47973));
        auto route2 = router.aStar(Coordinates(49.02583, 8.35493), Coordinates(49.03404, 8.45346));

        std::vector<Coordinates> route1Coords;
        for (const auto &[nodeId, coords] : route1)
        {
            route1Coords.push_back(coords);
        }
        std::vector<Coordinates> route2Coords;
        for (const auto &[nodeId, coords] : route2)
        {
            route2Coords.push_back(coords);
        }

        auto edges1 = routes.getEdgeSet(route1Coords);
        auto edges2 = routes.getEdgeSet(route2Coords);

        auto parameterLengthMap1 = routes.getParameterLengthMap(edges1);
        auto parameterLengthMap2 = routes.getParameterLengthMap(edges2);

        std::cout << "Parameter Length Map for Route 1:\n";
        for (const auto &[key, valueMap] : parameterLengthMap1)
        {
            for (const auto &[value, length] : valueMap)
            {
                std::cout << "  " << key << " = " << value << ": " << length << "\n";
            }
        }

        std::cout << "\nParameter Length Map for Route 2:\n";
        for (const auto &[key, valueMap] : parameterLengthMap2)
        {
            for (const auto &[value, length] : valueMap)
            {
                std::cout << "  " << key << " = " << value << ": " << length << "\n";
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
}
