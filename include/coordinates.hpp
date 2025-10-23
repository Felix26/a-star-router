#pragma once

#include <ostream>

class Coordinates
{
    public:
        Coordinates(double latitude, double longitude)
            : mLatitude(latitude), mLongitude(longitude) {}

        double getLatitude() const { return mLatitude; }
        double getLongitude() const { return mLongitude; }

        friend std::ostream& operator<<(std::ostream& os, const Coordinates& coords);

    private:
        double mLatitude;
        double mLongitude;
};