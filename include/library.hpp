#pragma once

#include <vector>
#include <string>
#include <tuple>
#include <cstdint>

#include "coordinates.hpp"

namespace HelperFunctions
{
    inline constexpr double EARTH_RADIUS_KM = 6371.0;

    double haversine(double lat1_deg, double lon1_deg, double lat2_deg, double lon2_deg);

    double haversine(const Coordinates &coord1, const Coordinates &coord2);

    double calculatePathLength(const std::vector<Coordinates> &path);

    void exportPathToGeoJSON(const std::vector<std::tuple<uint64_t, Coordinates>> &path, const std::string &filename);
} // namespace HelperFunctions
