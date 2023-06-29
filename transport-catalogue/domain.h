#pragma once
#include <string>
#include <vector>
#include <string_view>
#include <unordered_map>
#include <set>
#include <functional>

#include "geo.h"

namespace tcat {
 
struct Stop {
    std::string name;
    geo::Coordinates coordinates;
};
    
struct PreBus {
    std::string name;
    std::vector<std::string> stops;
    bool is_circular;
};
    
struct Bus {
    std::string name;
    std::vector<Stop*> stops;
    int number_of_stops = 0;
    int unique_stops = 0;
    int route_length = 0;
    double curvature = 0;
    bool is_circular;
};
    
struct BusInfo {
    explicit BusInfo(std::string_view name, int stops, int unique_stops, int route_length, double curvature);
    std::string name;
    int stops = 0;
    int unique_stops = 0;
    int route_length = 0;
    double curvature = 0;
};
    
enum class StopInfoStatus {
    NOT_FOUND,
    NO_BUSES,
    NORMAL
};
    
struct StopInfo {
    std::string name;
    std::set<std::string_view> buses;
    StopInfoStatus status =  StopInfoStatus::NORMAL;
};
    
struct StopDistances {
    std::string name;
    std::unordered_map<std::string, int> stop_to_distance;
};
    
struct RenderData {
    std::vector<geo::Coordinates> stop_coords;
    std::vector<std::string> stop_names;
    bool is_circular = false;
};
    
namespace detail {
struct StopPairHasher {
    size_t operator()(const std::pair<Stop*, Stop*>& pair_of_stops) const;
    
private:
    std::hash<const void*> p_hasher;
};
}
}