#pragma once

#include <vector>
#include <deque>
#include <unordered_map>
#include <string>
#include <string_view>
#include <set>
#include <utility>

namespace tcat {
    
    struct Stop {
        std::string name;
        double latitude;
        double longtitude;
    };
    
    struct PreBus {
        std::string name;
        std::vector<std::string> stops;
        std::string type;
    };
    
    struct Bus {
        std::string name;
        std::vector<Stop*> stops;
        size_t number_of_stops = 0;
        size_t unique_stops = 0;
        size_t route_length = 0;
        double curvature = 0;
        std::string type;
    };
    
    struct BusInfo {
        explicit BusInfo(std::string_view name, size_t stops, size_t unique_stops, size_t route_length, double curvature) :
                        name(name), stops(stops), unique_stops(unique_stops), route_length(route_length), curvature(curvature) {}
        std::string name;
        size_t stops = 0;
        size_t unique_stops = 0;
        size_t route_length = 0;
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
        std::unordered_map<std::string, size_t> stop_to_distance;
    };
    
namespace detail {
    struct StopPairHasher {
        size_t operator()(const std::pair<Stop*, Stop*>& pair_of_stops) const {
            size_t first_hash = p_hasher(pair_of_stops.first);
            size_t second_hash = p_hasher(pair_of_stops.second);
            return first_hash * 53 + second_hash * (53*53*53);
        }
        private:
            std::hash<const void*> p_hasher;
    };
}
    
class TransportCatalogue {
public:
    
    void AddStop(const Stop& stop);
    
    Stop* FindStop(std::string_view stop_name) const;
    
    void AddBus(const PreBus& pre_bus);
    
    Bus* FindBus(std::string_view bus_name) const;
    
    BusInfo GetBusInfo(std::string_view bus_name) const;
    
    StopInfo GetStopInfo(std::string_view stop_name) const;
    
    void AddDistanceBetweenStops(const StopDistances& stop_distances);
    
private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;
    std::unordered_map<Stop*, std::set<std::string>> stop_to_buses_;
    std::unordered_map<std::pair<Stop*, Stop*>, size_t, detail::StopPairHasher> distance_between_stops_;
    
    size_t CalculateStops(const Bus* bus_ptr) const;
    
    size_t CalculateUniqueStops(const Bus* bus_ptr) const;
    
    std::pair<size_t, double> CalculateRouteLength(const Bus* bus_ptr) const;
};
}