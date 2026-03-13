#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <cassert>

#include <ankerl/unordered_dense.h>

#include "router.hpp"
#include "library.hpp"
#include "osmnode.hpp"
#include "osmway.hpp"
#include "coordinates.hpp"

int main()
{
    try
    {
        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/neureut.osm";

        Router router(osmPath);

        Coordinates testCoordinates = {49.055824, 8.385704};

        std::cout << (std::get<1>(router.getEdgeSplit(testCoordinates)) & 0x00FFFFFFFFFFFFFF);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
}
