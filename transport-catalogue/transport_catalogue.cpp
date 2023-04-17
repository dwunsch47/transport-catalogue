#include <vector>
#include <deque>
#include <unordered_map>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <iostream>
#include <unordered_set>
#include <set>

#include "transport_catalogue.h"
#include "geo.h"

using namespace std;

namespace tcat {
void TransportCatalogue::AddStop(const Stop& stop) {
    stops_.push_back(move(stop));
    Stop* stop_ptr = &stops_.back();
    stopname_to_stop_.emplace(stop_ptr->name, stop_ptr);
    
}
    
Stop* TransportCatalogue::FindStop(string_view stop_name) const {
    auto it = stopname_to_stop_.find(stop_name);
    return (it != stopname_to_stop_.end() ? it->second : nullptr);
}
    
void TransportCatalogue::AddBus(const PreBus& pre_bus) {
    Bus bus;
    bus.name = move(pre_bus.name);
    bus.type = move(pre_bus.type);
    for (const std::string_view stop : pre_bus.stops) {
        Stop* stop_ptr = FindStop(stop);
        bus.stops.push_back(stop_ptr);
        stop_to_buses_[stop_ptr].insert(bus.name);
    }
    buses_.push_back(move(bus));
    Bus* bus_ptr = &buses_.back();
    busname_to_bus_.emplace(buses_.back().name, bus_ptr);
}
    
Bus* TransportCatalogue::FindBus(string_view bus_name) const {
    auto it = busname_to_bus_.find(bus_name);
    if (it != busname_to_bus_.end()) {
    }
    return (it != busname_to_bus_.end() ? it->second : nullptr);
}
    
BusInfo TransportCatalogue::GetBusInfo(string_view bus_name) const {
    BusInfo bus_info;
    bus_info.name = string(bus_name);
    const Bus* bus_ptr = FindBus(bus_name);
    if (!bus_ptr) {
        bus_info.stops = 0;
        return bus_info;
    }
    bus_info.stops = CalculateStops(bus_ptr);
    bus_info.unique_stops = CalculateUniqueStops(bus_ptr);
    auto pair_dist_curvature = CalculateRouteLength(bus_ptr);
    bus_info.route_length = move(pair_dist_curvature.first);
    bus_info.curvature = move(pair_dist_curvature.second);
    return bus_info;
}
    
void TransportCatalogue::AddDistanceBetweenStops(const StopDistances& stop_distances) {
    Stop* primary_ptr = FindStop(stop_distances.name);
    for (const auto& [name, dist] : stop_distances.stop_to_distance) {
        Stop* current_ptr = FindStop(name);
        auto pair = make_pair(primary_ptr, current_ptr);
        distance_between_stops_.emplace(move(pair), dist);
    }
}
    
size_t TransportCatalogue::CalculateStops(const Bus* bus_ptr) const {
        return bus_ptr->stops.size();
}
    
size_t TransportCatalogue::CalculateUniqueStops(const Bus* bus_ptr) const {
    return unordered_set<Stop*>(bus_ptr->stops.begin(), bus_ptr->stops.end()).size();
}
    
pair<size_t, double> TransportCatalogue::CalculateRouteLength(const Bus* bus_ptr) const {
    size_t route_length = 0;
    double coord_length = 0;
    for (int i = 0; i < bus_ptr->stops.size() - 1; ++i) {
        Stop* first = FindStop(bus_ptr->stops[i]->name);
        Stop* second = FindStop(bus_ptr->stops[i + 1]->name);
        if (distance_between_stops_.find({first, second}) == distance_between_stops_.end()) {
            route_length += distance_between_stops_.at({second, first});
        } else {
            route_length += distance_between_stops_.at({first, second});
        }
        coord_length += geo::ComputeDistance({first->latitude, first->longtitude}, {second->latitude, second->longtitude});
    }
    return {route_length, route_length / coord_length};
}
    
StopInfo TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
    StopInfo stop_info;
    stop_info.name = string(stop_name);
    Stop* stop_ptr = FindStop(stop_name);
    if (!stop_ptr) {
        stop_info.status = StopInfoStatus::NOT_FOUND;
    }
    else if (stop_to_buses_.find(stop_ptr) == stop_to_buses_.end()) {
        stop_info.status = StopInfoStatus::NO_BUSES;
    } else {
        set<string_view> eligible_buses(stop_to_buses_.at(stop_ptr).begin(), stop_to_buses_.at(stop_ptr).end());
        stop_info.buses = move(eligible_buses);
    }
    return stop_info;
}
}