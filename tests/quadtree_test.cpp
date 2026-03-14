#include "gpxparser.hpp"

#include <iostream>
#include <cassert>
#include <cstdlib>

#include "router.hpp"
#include "library.hpp"

#include "quadtree.hpp"

// BBox: box
Coordinates randomCoordinateGenerator(Box box)
{
    double minLat = box.getMinLatitudeLongitude().getLatitude();
    double maxLat = box.getMaxLatitudeLongitude().getLatitude();
    double minLon = box.getMinLatitudeLongitude().getLongitude();
    double maxLon = box.getMaxLatitudeLongitude().getLongitude();

    double lat = minLat + static_cast<double>(rand()) / RAND_MAX * (maxLat - minLat);
    double lon = minLon + static_cast<double>(rand()) / RAND_MAX * (maxLon - minLon);

    return Coordinates(lat, lon);
}


int main()
{
    srand(0);
    try
    {
        const std::string osmPath = std::string(PROJECT_SOURCE_DIR) + "/testdata/neureut.osm";
        Router router(osmPath);

        Quadtree &quadtree = router.getQuadtree();
        Graph &graph = router.getGraph();

        Box boundary = quadtree.getBoundary();

        std::cout << quadtree << "\n";

        // Check every edge bounding box
        for(Quadtree *subtree : quadtree.getAllSubtrees())
        {
            for(const auto &[edge, subwayId] : subtree->getEdgeSubwayIDs())
            {
                if(edge == nullptr) continue;

                const Box &box = edge->getBoundingBox(subwayId);
                assert(subtree->getBoundary().contains(box));
            }
        }

        Coordinates testPoint(49.053357, 8.384253);

        auto closestEdges = quadtree.getClosestEdges(testPoint, 10, false);
        std::cout << "Closest edges to point " << testPoint << ":\n";
        for(const auto &ce : closestEdges)
        {
            std::cout << "Edge ID: " << (ce.edge->getId() & 0x00FFFFFFFFFFFFFF) << ", Subway ID: " << static_cast<int>(ce.subwayId) << ", Distance: " << ce.distance << " m\n";
        }

        assert(closestEdges.size() == 10);
        assert(closestEdges[0].distance < closestEdges[1].distance);
        assert(closestEdges[0].distance < 10);

        for(int i = 0; i < 1000; i++)
        {
            Coordinates randomPoint = randomCoordinateGenerator(boundary);
            auto closestEdges = quadtree.getClosestEdges(randomPoint, 1);
            auto closestEdgeNotQuadtree = router.getEdgeSplit(randomPoint);
            Edge closestEdgeRef = *graph.getEdges().at(std::get<1>(closestEdgeNotQuadtree)).get();

            uint64_t closestEdgeIdQuadtree = closestEdges[0].edge->getId() & 0x00FFFFFFFFFFFFFF;
            uint64_t closestEdgeIdGraph = std::get<1>(closestEdgeNotQuadtree) & 0x00FFFFFFFFFFFFFF;

            double distanceQuadtree = closestEdges[0].distance;
            double distanceGraph = HelperFunctions::distancePointToSegment(randomPoint, closestEdgeRef.getPath()[std::get<2>(closestEdgeNotQuadtree)], closestEdgeRef.getPath()[std::get<2>(closestEdgeNotQuadtree) + 1]);

            if(closestEdgeIdQuadtree != closestEdgeIdGraph || std::abs(distanceQuadtree - distanceGraph) >= 0.01)
            {
                std::cout << "Random Point: " << randomPoint 
                << ", Closest Quadtree Edge: " << closestEdgeIdQuadtree
                << ", Closest Graph Edge: " << closestEdgeIdGraph
                << ", Distance Quadtree: " << distanceQuadtree << " m"
                << ", Distance Graph: " << distanceGraph <<"\n";
            }
            
            assert(closestEdgeIdQuadtree == closestEdgeIdGraph || std::abs(distanceQuadtree - distanceGraph) < 0.01);
        }


    }
    catch (const std::exception &e)
    {
        std::cerr << "Fehler im Test: " << e.what() << "\n";
        return 1;
    }
    return 0;
}