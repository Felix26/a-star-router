#include "coordinates.hpp"

#include <ostream>
#include <iomanip>

std::ostream &operator<<(std::ostream &os, const Coordinates &coords)
{
    os << "Coordinates(" << std::fixed << std::setprecision(7) << coords.getLatitude() << ", " << coords.getLongitude() << ")";
    return os;
}