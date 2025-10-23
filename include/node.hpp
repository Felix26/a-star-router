#pragma once

#include "coordinates.hpp"

class Node
{
    public:
        Node(int id, double latitude, double longitude)
            : id(id), coordinates(latitude, longitude) {}

        int getId() const { return id; }
        Coordinates getCoordinates() const { return coordinates; }

        int trackcount = 0;
        
    private:
        int id;
        Coordinates coordinates;
};