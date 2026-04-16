#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <vector>
#include <filesystem>
#include <pugixml.hpp>

#include <ankerl/unordered_dense.h>

#include "coordinates.hpp"
#include "box.hpp"
#include "edge.hpp"

class Graph;
class OsmNode;
class OsmWay;

typedef std::vector<Coordinates> Path;

namespace HelperFunctions
{
    inline constexpr double EARTH_RADIUS_KM = 6371.0;

    double haversine(double lat1_deg, double lon1_deg, double lat2_deg, double lon2_deg);

    double haversine(const Coordinates &coord1, const Coordinates &coord2);

    double euclideanDistanceSquared(const Coordinates &c1, const Coordinates &c2);

    double calculatePathLength(const std::vector<Coordinates> &path);

    double distancePointToSegment(const Coordinates &point, const Coordinates &segStart, const Coordinates &segEnd);

    Coordinates getProjectionOnSegment(const Coordinates &point, const Coordinates &segStart, const Coordinates &segEnd);

    std::filesystem::path exportPathToGeoJSON(const std::vector<std::tuple<uint64_t, Coordinates>> &path, const std::filesystem::path &filename);

    void readOSMFile(const std::string &filepath,
                     ankerl::unordered_dense::map<uint64_t, std::shared_ptr<OsmNode>> &nodes,
                     ankerl::unordered_dense::map<uint64_t, std::unique_ptr<OsmWay>> &ways);

    const Box createGraph(Graph &graph,
                     ankerl::unordered_dense::map<uint64_t, std::shared_ptr<OsmNode>> &nodes,
                     ankerl::unordered_dense::map<uint64_t, std::unique_ptr<OsmWay>> &ways);

    double logisticFunction(double x, double lowerBound = 0, double upperBound = 1, double steepness = 1, double maxGrowthX = 0);

    std::vector<std::vector<Coordinates>> getGPXTrackPoints(const std::filesystem::path &file);

    void saveEdgesAsGeoJSON(const std::vector<Edge *> &edges);
} // namespace HelperFunctions
