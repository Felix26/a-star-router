#pragma once

#include "coordinates.hpp"

#include <cstdint>

class OsmNode
{
    public:
        OsmNode(uint64_t id, double latitude, double longitude)
            : id(id), coordinates(latitude, longitude) {}

        const uint64_t getId() const { return id; }
        Coordinates getCoordinates() const { return coordinates; }

        bool isVisited = false;
        bool isEdge = false;
        
    protected:
        uint64_t id;
        Coordinates coordinates;
};