#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <cstdint>
#include <chrono>
#include <format>
#include <unordered_map>

#include "coordinates.hpp"
#include "quadtree.hpp"

// Track: Filename and vector of coordinates
typedef std::unordered_map<std::string, std::vector<Coordinates>> Tracks;

class GPXParser
{
    public:
        GPXParser();

        void loadGPXFiles(const std::string &directory);

        std::vector<std::tuple<uint64_t, Coordinates>> fillEdgeIDs(const Quadtree &quadtree, const std::vector<Coordinates> &trackPoints);

        const std::vector<std::tuple<uint64_t, double>>& getEdgeIDs() { return mEdgeIDs; }

        const Tracks& getTracks() { return mTracks; }
    private:
        // Each GPX track is stored as a tuple of (filename, track points)
        Tracks mTracks;
        std::vector<std::tuple<uint64_t, double>> mEdgeIDs;

        void parseGPXFile(const std::filesystem::directory_entry &file);
};