#include <functional>
#include <utility>
#include <string_view>

#include "domain.h"

namespace tcat {

BusInfo::BusInfo(std::string_view name, int stops, int unique_stops, int route_length, double curvature) :
                    name(name), stops(stops), unique_stops(unique_stops), route_length(route_length), curvature(curvature) {}
    
namespace detail {
        size_t StopPairHasher::operator()(const std::pair<Stop*, Stop*>& pair_of_stops) const {
            size_t first_hash = p_hasher(pair_of_stops.first);
            size_t second_hash = p_hasher(pair_of_stops.second);
            return first_hash * 53 + second_hash * (53*53*53);
}
}
}