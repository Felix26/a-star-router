#include "gpxparser.hpp"

#include <iostream>
#include <ankerl/unordered_dense.h>
#include <cstdlib>
#include "router.hpp"

#include "quadtree.hpp"

Coordinates randomCoordinateGenerator()
{
    double lat = 47.5338000528 + static_cast<double>(rand()) / (static_cast<double>(RAND_MAX/(49.7913749328 - 47.5338000528)));
    double lon = 7.5113934084 + static_cast<double>(rand()) / (static_cast<double>(RAND_MAX/(10.4918239143 - 7.5113934084)));
    return Coordinates(lat, lon);
}

int main()
{
    srand(0);

    try
    {
        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/karlsruhe_roads_min.osm";
        
        Router router(osmPath);
        Quadtree &quadtree = router.getQuadtree();
        Graph &graph = router.getGraph();
        //std::cout << quadtree;

        // time to test performance
        auto start = std::chrono::high_resolution_clock::now();

        const uint32_t testCount = 10000;

        for(int i = 0; i < testCount; i++)
        {
            Coordinates testPoint = randomCoordinateGenerator();

            auto closestEdges = quadtree.getClosestEdges(testPoint, 3);
            //td::cout << "Closest edges to point " << testPoint << ":\n";
            //for(const auto &ce : closestEdges)
            //{
                //std::cout << "Edge ID: " << (ce.edgeId % 0x00FFFFFFFFFFFFFF) << ", Subway ID: " << static_cast<int>(ce.subwayId) << ", Distance: " << ce.distance << " m\n";
            //}
        }

        auto end =  std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        std::cout << "Time taken for " << testCount << " nearest edge searches: " << duration.count() << " ms\n";
        std::cout << "Points per second: " << (testCount / (duration.count() / 1000.0)) << " points/s\n";

        std::cout << "Graph has " << graph.getEdges().size() << " edges.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
    return 0;
}