#include "box.hpp"

#include <algorithm>

Box::Box(const Coordinates &topLeft, const Coordinates &bottomRight)
{
    mMinLatitude = std::min(topLeft.getLatitude(), bottomRight.getLatitude());
    mMinLongitude = std::min(topLeft.getLongitude(), bottomRight.getLongitude());
    mMaxLatitude = std::max(topLeft.getLatitude(), bottomRight.getLatitude());
    mMaxLongitude = std::max(topLeft.getLongitude(), bottomRight.getLongitude());
}

bool Box::overlaps(const Box &other) const
{
    return !(mMaxLatitude < other.mMinLatitude || mMinLatitude > other.mMaxLatitude
        || mMaxLongitude < other.mMinLongitude || mMinLongitude > other.mMaxLongitude);
}

bool Box::contains(const Coordinates &point) const
{
    return mMinLatitude <= point.getLatitude() && point.getLatitude() <= mMaxLatitude
        && mMinLongitude <= point.getLongitude() && point.getLongitude() <= mMaxLongitude;
}

bool Box::contains(const Box &other) const
{
    return contains(other.getTopLeft()) && contains(other.getBottomRight());
}
