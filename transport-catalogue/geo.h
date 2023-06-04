#pragma once
#include <functional>

namespace geo {

struct Coordinates {
    double lat;
    double lng;
    bool operator==(const Coordinates& other) const {
        return lat == other.lat && lng == other.lng;
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
};

double ComputeDistance(Coordinates from, Coordinates to);
    
struct CoordinatesHasher {
    size_t operator()(const Coordinates& coords) const;
private:
    std::hash<double> d_hasher_;
};

}