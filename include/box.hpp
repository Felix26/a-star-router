#pragma once

#include "coordinates.hpp"

class Box
{
    public:
        Box(const Coordinates &topLeft, const Coordinates &bottomRight);

        bool overlaps(const Box &other) const;

        bool contains(const Coordinates &point) const;

        bool contains(const Box &other) const;

        double getDistance(const Coordinates &point) const;

        double getEstDistanceSquared(const Coordinates &point) const;

        const Coordinates getTopLeft() const { return Coordinates(mMinLatitude, mMinLongitude); }
        const Coordinates getTopRight() const { return Coordinates(mMinLatitude, mMaxLongitude); }
        const Coordinates getBottomLeft() const { return Coordinates(mMaxLatitude, mMinLongitude); }
        const Coordinates getBottomRight() const { return Coordinates(mMaxLatitude, mMaxLongitude); }

        const Coordinates getCenter() const { return Coordinates((mMinLatitude + mMaxLatitude) / 2.0, (mMinLongitude + mMaxLongitude) / 2.0); }
        const Coordinates getTopMiddle() const { return Coordinates(mMinLatitude, (mMinLongitude + mMaxLongitude) / 2.0); }
        const Coordinates getBottomMiddle() const { return Coordinates(mMaxLatitude, (mMinLongitude + mMaxLongitude) / 2.0); }
        const Coordinates getLeftMiddle() const { return Coordinates((mMinLatitude + mMaxLatitude) / 2.0, mMinLongitude); }
        const Coordinates getRightMiddle() const { return Coordinates((mMinLatitude + mMaxLatitude) / 2.0, mMaxLongitude); }

    private:
        double mMinLatitude;
        double mMinLongitude;

        double mMaxLatitude;
        double mMaxLongitude;
};