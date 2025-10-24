#pragma once

#include "coordinates.hpp"

#include <cstdint>

class Node
{
    public:
        Node(uint64_t id, double latitude, double longitude)
            : id(id), coordinates(latitude, longitude) {}

        uint64_t getId() const { return id; }
        Coordinates getCoordinates() const { return coordinates; }

        int trackcount = 0;
        
    private:
        uint64_t id;
        Coordinates coordinates;
};