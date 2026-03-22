#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <cstdint>
#include <chrono>
#include <format>
#include <unordered_set>

#include "coordinates.hpp"
#include "router.hpp"

// Track: Filename and vector of coordinates
typedef std::unordered_map<std::string, std::vector<Coordinates>> Tracks;

class GPXParser
{
    public:
        GPXParser();

        void loadGPXFiles(const std::string &directory);

        std::vector<std::tuple<uint64_t, Coordinates>> fillEdgeIDs(Router &router, const std::vector<Coordinates> &trackPoints);

        void calculateSnapPenalties();
        bool doRouting(const Coordinates &start, const Coordinates &end, Router &router, std::vector<std::tuple<uint64_t, Coordinates>> &pathContainer) const;

        std::unordered_set<Edge *> getEdges() { return mEdges; }

        const Tracks& getTracks() { return mTracks; }
    private:
        // Each GPX track is stored as a tuple of (filename, track points)
        Tracks mTracks;
        std::unordered_set<Edge *> mEdges;

        void parseGPXFile(const std::filesystem::directory_entry &file);
        bool checkRoutingTrackPoints(const std::vector<ClosestEdges> &edges, std::vector<Coordinates> &routingTrackPoints, uint16_t pointIndex, const Coordinates &coordinates);
        void resetRoutingPoints();
        void reset();
        

        Coordinates bestPointCoords{0, 0};
        double bestPointMetric = 0;
        uint16_t bestPointIndex = 0;
        uint16_t lastPointIndex = 0;
};