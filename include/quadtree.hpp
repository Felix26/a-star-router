#pragma once

#include <memory>

#include "coordinates.hpp"

class Quadtree
{
    public:
        Quadtree();

        

    private:
        std::unique_ptr<Quadtree> mNorthWest;
        std::unique_ptr<Quadtree> mNorthEast;
        std::unique_ptr<Quadtree> mSouthWest;
        std::unique_ptr<Quadtree> mSouthEast;

        Coordinates mTopLeft;
        Coordinates mBottomRight;
};