#pragma once

#include <vector>
#include <deque>
#include <unordered_map>
#include <string>
#include <string_view>
#include <set>
#include <utility>
#include <map>

#include "domain.h"

namespace tcat {
    
class TransportCatalogue {
public:
    
    void AddStop(const Stop& stop);
    
    Stop* FindStop(std::string_view stop_name) const;
    
    void AddBus(const PreBus& pre_bus);
    
    Bus* FindBus(std::string_view bus_name) const;
    
    BusInfo GetBusInfo(std::string_view bus_name) const;
    
    StopInfo GetStopInfo(std::string_view stop_name) const;
    
    std::map<std::string, RenderData> GetAllRoutes() const;
    
    void AddDistanceBetweenStops(const StopDistances& stop_distances);
    
private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;
    std::unordered_map<Stop*, std::set<std::string>> stop_to_buses_;
    std::unordered_map<std::pair<Stop*, Stop*>, size_t, detail::StopPairHasher> distance_between_stops_;
    
    int CalculateStops(const Bus* bus_ptr) const;
    
    int CalculateUniqueStops(const Bus* bus_ptr) const;
    
    std::pair<int, double> CalculateRouteLength(const Bus* bus_ptr) const;
};
}