#include "library.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>

namespace HelperFunctions
{
    namespace
    {
        constexpr double DEG_TO_RAD = 3.14159265358979323846 / 180.0;
    }

    // Haversine formula to calculate the great-circle distance between two points in meters
    double haversine(double lat1_deg, double lon1_deg, double lat2_deg, double lon2_deg)
    {
        const double lat1 = lat1_deg * DEG_TO_RAD;
        const double lon1 = lon1_deg * DEG_TO_RAD;
        const double lat2 = lat2_deg * DEG_TO_RAD;
        const double lon2 = lon2_deg * DEG_TO_RAD;

        const double dlat = lat2 - lat1;
        const double dlon = lon2 - lon1;

        const double sin_lat = std::sin(dlat / 2.0);
        const double sin_lon = std::sin(dlon / 2.0);

        const double a = (sin_lat * sin_lat) +
                         std::cos(lat1) * std::cos(lat2) * (sin_lon * sin_lon);

        const double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));

        return EARTH_RADIUS_KM * c * 1000.0; // return distance in meters
    }

    double haversine(const Coordinates &coord1, const Coordinates &coord2)
    {
        return haversine(coord1.getLatitude(), coord1.getLongitude(),
                         coord2.getLatitude(), coord2.getLongitude());
    }

    double calculatePathLength(const std::vector<Coordinates> &path)
    {
        double totalLength = 0.0;

        for (std::size_t i = 1; i < path.size(); ++i)
        {
            totalLength += haversine(path[i - 1], path[i]);
        }

        return totalLength;
    }

    double distancePointToSegment(const Coordinates &point, const Coordinates &segStart, const Coordinates &segEnd)
    {
        double lat1 = segStart.getLatitude() * DEG_TO_RAD;
        double lon1 = segStart.getLongitude() * DEG_TO_RAD;
        double lat2 = segEnd.getLatitude() * DEG_TO_RAD;
        double lon2 = segEnd.getLongitude() * DEG_TO_RAD;
        double latP = point.getLatitude() * DEG_TO_RAD;
        double lonP = point.getLongitude() * DEG_TO_RAD;

        double dLat = lat2 - lat1;
        double dLon = lon2 - lon1;

        if (dLat == 0 && dLon == 0)
        {
            return haversine(point, segStart);
        }

        double t = ((latP - lat1) * dLat + (lonP - lon1) * dLon) / (dLat * dLat + dLon * dLon);
        t = std::max(0.0, std::min(1.0, t));

        double projLat = lat1 + t * dLat;
        double projLon = lon1 + t * dLon;

        Coordinates projection(projLat / DEG_TO_RAD, projLon / DEG_TO_RAD);
        return haversine(point, projection);
    }

    void exportPathToGeoJSON(const std::vector<std::tuple<uint64_t, Coordinates>> &path, const std::string &filename)
    {
        std::ofstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Error opening file for writing: " << filename << "\n";
            return;
        }

        file << "{\n";
        file << "  \"type\": \"FeatureCollection\",\n";
        file << "  \"features\": [\n";
        file << "    {\n";
        file << "      \"type\": \"Feature\",\n";
        file << "      \"geometry\": {\n";
        file << "        \"type\": \"LineString\",\n";
        file << "        \"coordinates\": [\n";

        for (size_t i = 0; i < path.size(); ++i)
        {
            const auto &[id, coord] = path[i];
            file << "          [" << std::fixed << std::setprecision(7) << coord.getLongitude() << ", " << coord.getLatitude() << "]";
            if (i < path.size() - 1)
                file << ",";
            file << "\n";
        }

        file << "        ]\n";
        file << "      },\n";
        file << "      \"properties\": {}\n";
        file << "    }\n";
        file << "  ]\n";
        file << "}\n";

        file.close();
    }
} // namespace HelperFunctions
