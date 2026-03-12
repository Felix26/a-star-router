#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <cstdint>
#include <chrono>
#include <format>

#include "coordinates.hpp"
#include "quadtree.hpp"

typedef std::tuple<std::chrono::system_clock::time_point, std::vector<Coordinates>> Track;
typedef std::vector<Track> Tracks;

class GPXParser
{
    public:
        GPXParser();

        void loadGPXFiles(const std::string &directory);

        void fillEdgeIDs(const Quadtree &quadtree);

        const std::vector<uint64_t>& getEdgeIDs() { return mEdgeIDs; }

        const Tracks& getTracks() { return mTracks; }
    private:
        // Each GPX track is stored as a tuple of (timestamp, track points)
        Tracks mTracks;
        std::vector<uint64_t> mEdgeIDs;

        void parseGPXFile(const std::filesystem::directory_entry &file);
};