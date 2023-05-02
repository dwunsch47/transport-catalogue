#define _USE_MATH_DEFINES
#include "geo.h"

#include <cmath>

namespace geo {

double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    if (from == to) {
        return 0;
    }
    static const double dr = M_PI / 180.;
    static const auto earth_r = 6371000;
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
        * earth_r;
}

size_t CoordinatesHasher::operator()(const Coordinates& coords) const {
    size_t lat_hash = d_hasher_(coords.lat);
    size_t lng_hash = d_hasher_(coords.lng);
    return lat_hash * 37 + lng_hash;
}
    
}  // namespace geo