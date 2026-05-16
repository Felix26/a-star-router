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
        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/Karlsruhe_Region.osm";

        Router router(osmPath);

        std::cout << (router.getQuadtree().getBoundary().getBottomLeft()) << "; " << router.getQuadtree().getBoundary().getTopRight() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
}
