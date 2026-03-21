#include "coordinates.hpp"

#include <ostream>
#include <iomanip>

std::ostream &operator<<(std::ostream &os, const Coordinates &coords)
{
    os << "Coordinates(" << std::fixed << std::setprecision(7) << coords.getLatitude() << ", " << coords.getLongitude() << ")";
    return os;
}

Coordinates::Coordinates(const std::string &coordString)
{
    std::stringstream ss(coordString);
    char comma;
    
    // read: Double -> Char -> Double
    // >> automatically skips whitespace, so "lat, lon" and "lat ,lon" or "lat,lon" are all valid
    if (!(ss >> mLatitude >> comma >> mLongitude) || comma != ',')
    {
        throw std::invalid_argument("Ungueltiges Format. Erwartet: 'lat, lon'");
    }
}
