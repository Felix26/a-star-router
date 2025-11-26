#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <cstdint>

#include "coordinates.hpp"
#include "quadtree.hpp"

class GPXParser
{
    public:
        GPXParser();

        void loadGPXFiles(const std::string &directory);

        void fillEdgeIDs(const Quadtree &quadtree);

        const std::vector<uint64_t>& getEdgeIDs() { return mEdgeIDs; }
    private:
        std::vector<Coordinates> mTrackPoints;
        std::vector<uint64_t> mEdgeIDs;

        void parseGPXFile(const std::filesystem::directory_entry &file);
};