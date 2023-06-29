#include <vector>
#include <deque>
#include <unordered_map>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <iostream>
#include <set>
#include <algorithm>
#include <map>

#include "transport_catalogue.h"
#include "geo.h"
#include "domain.h"

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
    bus.is_circular = pre_bus.is_circular;
    
    for (const std::string_view stop : pre_bus.stops) {
        Stop* stop_ptr = FindStop(stop);
        bus.stops.push_back(stop_ptr);
        stop_to_buses_[stop_ptr].insert(bus.name);
    }
    if (!pre_bus.is_circular) {
        vector<string> pre(pre_bus.stops.begin(), pre_bus.stops.end() - 1);
        reverse(pre.begin(), pre.end());
        for (const auto& stop : pre) {
            Stop* stop_ptr = FindStop(stop);
            bus.stops.push_back(stop_ptr);
            stop_to_buses_[stop_ptr].insert(bus.name);
        }
    }
    buses_.push_back(move(bus));
    Bus* bus_ptr = &buses_.back();
    bus_ptr->number_of_stops = CalculateStops(bus_ptr);
    bus_ptr->unique_stops = CalculateUniqueStops(bus_ptr);
    auto pair_dist_curvature = CalculateRouteLength(bus_ptr);
    bus_ptr->route_length = move(pair_dist_curvature.first);
    bus_ptr->curvature = move(pair_dist_curvature.second);
    
    busname_to_bus_.emplace(buses_.back().name, bus_ptr);
}
    
Bus* TransportCatalogue::FindBus(string_view bus_name) const {
    auto it = busname_to_bus_.find(bus_name);
    if (it != busname_to_bus_.end()) {
    }
    return (it != busname_to_bus_.end() ? it->second : nullptr);
}
    
BusInfo TransportCatalogue::GetBusInfo(string_view bus_name) const {
    const Bus* bus_ptr = FindBus(bus_name);
    if (!bus_ptr) {
        return BusInfo(string(bus_name),
                      0,
                      0,
                      0,
                      0);
    }
    return BusInfo(bus_ptr->name,
                  bus_ptr->number_of_stops,
                  bus_ptr->unique_stops,
                  bus_ptr->route_length,
                  bus_ptr->curvature);
}
    
void TransportCatalogue::AddDistanceBetweenStops(const StopDistances& stop_distances) {
    Stop* primary_ptr = FindStop(stop_distances.name);
    for (const auto& [name, dist] : stop_distances.stop_to_distance) {
        Stop* current_ptr = FindStop(name);
        auto pair = make_pair(primary_ptr, current_ptr);
        distance_between_stops_.emplace(pair, dist);
    }
}
    
void TransportCatalogue::AddDistance(Stop* from, Stop* to, size_t distance) {
    auto pair = make_pair(from, to);
    distance_between_stops_.emplace(pair, distance);
}
    
map<string, RenderData> TransportCatalogue::GetAllRoutes() const {
    map<string, RenderData> result;
    for(const auto& bus : buses_) {
        if (bus.number_of_stops > 0) {
            RenderData render_data;
            for (const auto& stop : bus.stops) {
                render_data.stop_coords.push_back(stop->coordinates);
                render_data.stop_names.push_back(stop->name);
            }
            render_data.is_circular = bus.is_circular;
            result.emplace(bus.name, render_data);
        }
    }
    return result;
}
    
size_t TransportCatalogue::GetAllStopsCount() const {
    return stops_.size();
}
    
const unordered_map<string_view, Stop*>& TransportCatalogue::GetAllStops() const {
    return stopname_to_stop_;
}
    
const unordered_map<string_view, Bus*>& TransportCatalogue::GetAllBuses() const {
    return busname_to_bus_;
}
    
const unordered_map<pair<Stop*, Stop*>, size_t, detail::StopPairHasher>& TransportCatalogue::GetAllDistances() const {
    return distance_between_stops_;
}

    
size_t TransportCatalogue::GetDistance(Stop* from, Stop* to) const {
    if (distance_between_stops_.find({from, to}) == distance_between_stops_.end()) {
        if (distance_between_stops_.find({to, from}) == distance_between_stops_.end()) {
            return 0;
        }
        return distance_between_stops_.at({to, from});
    }
    return distance_between_stops_.at({from, to});
}
    
int TransportCatalogue::CalculateStops(const Bus* bus_ptr) const {
        return bus_ptr->stops.size();
}
    
int TransportCatalogue::CalculateUniqueStops(const Bus* bus_ptr) const {
    vector<Stop*> tmp = bus_ptr->stops;
    sort(tmp.begin(), tmp.end());
    auto last = unique(tmp.begin(), tmp.end());
    return (last != tmp.end() ? distance(tmp.begin(), last) : tmp.size());
}
    
pair<int, double> TransportCatalogue::CalculateRouteLength(const Bus* bus_ptr) const {
    int route_length = 0;
    double coord_length = 0;
    for (size_t i = 0; i < bus_ptr->stops.size() - 1; ++i) {
        Stop* first = FindStop(bus_ptr->stops[i]->name);
        Stop* second = FindStop(bus_ptr->stops[i + 1]->name);
        if (distance_between_stops_.find({first, second}) == distance_between_stops_.end()) {
            route_length += distance_between_stops_.at({second, first});
        } else {
            route_length += distance_between_stops_.at({first, second});
        }
        coord_length += geo::ComputeDistance(first->coordinates, second->coordinates);
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