#pragma once

#include <vector>

#include "coordinates.hpp"

namespace HelperFunctions
{
    inline constexpr double EARTH_RADIUS_KM = 6371.0;

    double haversine(double lat1_deg, double lon1_deg, double lat2_deg, double lon2_deg);

    double haversine(const Coordinates &coord1, const Coordinates &coord2);

    double calculatePathLength(const std::vector<Coordinates> &path);
} // namespace HelperFunctions
