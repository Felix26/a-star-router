#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include <unordered_dense.h>

#include "coordinates.hpp"

class Graph;
class OsmNode;
class OsmWay;

namespace HelperFunctions
{
    inline constexpr double EARTH_RADIUS_KM = 6371.0;

    double haversine(double lat1_deg, double lon1_deg, double lat2_deg, double lon2_deg);

    double haversine(const Coordinates &coord1, const Coordinates &coord2);

    double euclideanDistanceSquared(const Coordinates &c1, const Coordinates &c2);

    double calculatePathLength(const std::vector<Coordinates> &path);

    double distancePointToSegment(const Coordinates &point, const Coordinates &segStart, const Coordinates &segEnd);

    Coordinates getProjectionOnSegment(const Coordinates &point, const Coordinates &segStart, const Coordinates &segEnd);

    std::string exportPathToGeoJSON(const std::vector<std::tuple<uint64_t, Coordinates>> &path, const std::string &filename);

    void readOSMFile(const std::string &filepath,
                     ankerl::unordered_dense::map<u_int64_t, std::shared_ptr<OsmNode>> &nodes,
                     ankerl::unordered_dense::map<uint64_t, std::unique_ptr<OsmWay>> &ways);

    void createGraph(Graph &graph,
                     ankerl::unordered_dense::map<u_int64_t, std::shared_ptr<OsmNode>> &nodes,
                     ankerl::unordered_dense::map<uint64_t, std::unique_ptr<OsmWay>> &ways);
} // namespace HelperFunctions
