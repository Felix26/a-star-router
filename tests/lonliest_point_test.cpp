#include "gpxparser.hpp"

#include <iostream>
#include <cstdlib>
#include "router.hpp"

#include "quadtree.hpp"

// BBox: 8.334750,48.461023,9.699801,49.078304
Coordinates randomCoordinateGenerator()
{
    double lat = 48.461023 + static_cast<double>(rand()) / (static_cast<double>(RAND_MAX/(49.078304 - 48.461023)));
    double lon = 8.334750 + static_cast<double>(rand()) / (static_cast<double>(RAND_MAX/(9.699801 - 8.334750)));
    return Coordinates(lat, lon);
}

int main()
{
    srand(5);

    try
    {
        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/bw_min.osm";
        
        Router router(osmPath);
        Quadtree &quadtree = router.getQuadtree();
        Graph &graph = router.getGraph();
        //std::cout << quadtree;

        double maxMinDistance = std::numeric_limits<double>::min();
        uint32_t iterationIndex;

        // time to test performance
        auto start = std::chrono::high_resolution_clock::now();

        const uint32_t testCount = 1000000;

        for(int i = 0; i < testCount; i++)
        {
            Coordinates testPoint = randomCoordinateGenerator();

            auto closestEdges = quadtree.getClosestEdges(testPoint, 1);
            
            if(closestEdges[0].distance > maxMinDistance)
            {
                maxMinDistance = closestEdges[0].distance;
                iterationIndex = i;
                std::cout << i << ", " << maxMinDistance << "," << testPoint << std::endl;
            }
        }

        std::cout << testCount << ", " << maxMinDistance << std::endl;

        auto end =  std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        std::cout << "Time taken for " << testCount << " nearest edge searches: " << duration.count() << " ms\n";
        std::cout << "Points per second: " << (testCount / (duration.count() / 1000.0)) << " points/s\n";

        std::cout << "Graph has " << graph.getEdges().size() << " edges.\n";
        std::cout << "Quadtree Box: " << quadtree.getBoundary().getTopLeft() << " to " << quadtree.getBoundary().getBottomRight() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
    return 0;
}