#pragma once

#include "coordinates.hpp"

class Box
{
    public:
        Box(const Coordinates &topLeft, const Coordinates &bottomRight);

        bool overlaps(const Box &other) const;

        bool contains(const Coordinates &point) const;

        const Coordinates getTopLeft() const { return Coordinates(mMinLatitude, mMinLongitude); }
        const Coordinates getTopRight() const { return Coordinates(mMinLatitude, mMaxLongitude); }
        const Coordinates getBottomLeft() const { return Coordinates(mMaxLatitude, mMinLongitude); }
        const Coordinates getBottomRight() const { return Coordinates(mMaxLatitude, mMaxLongitude); }

    private:
        double mMinLatitude;
        double mMinLongitude;

        double mMaxLatitude;
        double mMaxLongitude;
};