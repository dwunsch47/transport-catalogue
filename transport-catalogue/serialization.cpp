#include "serialization.h"
#include "transport_catalogue.pb.h"

#include <string>
#include <fstream>
#include <algorithm>
#include <vector>

using namespace std;

namespace serialization {

Serializer::Serializer(tcat::TransportCatalogue& catalogue, std::shared_ptr<router::TransportRouter> tr, map_r::MapRenderer& map_renderer)
        : tc_(catalogue), tr_ptr_(tr), mr_(map_renderer) {}
    
void Serializer::SerializeToFile(const string& file) {
    ofstream ofs(file, ios::binary);
    proto_tc_.Clear();
    SerializeStops();
    SerializeBuses();
    SerializeDistances();
    //SerializeRenderSettings();
    //SerializeRouterSettings();
    
    proto_tc_.SerializeToOstream(&ofs);
}
    
void Serializer::DeserializeFromFile(const string& file) {
    ifstream ifs(file, ios::binary);
    proto_tc_.Clear();
    proto_tc_.ParseFromIstream(&ifs);
    
    DeserializeCatalogue();
    //DeserializeRenderSettings();
}
    
void Serializer::SerializeStops() {
    for (const auto& [name, stop_ptr] : tc_.GetAllStops()) {
        proto_serialization::Stop stop;
        stop.set_name(stop_ptr->name);
        proto_serialization::Coordinates coords;
        coords.set_lat(stop_ptr->coordinates.lat);
        coords.set_lng(stop_ptr->coordinates.lng);
        *stop.mutable_coords() = coords;
        
        *proto_tc_.add_stops() = stop; 
    }
}
    
void Serializer::SerializeBuses() {
    for (const auto& [name, bus_ptr] : tc_.GetAllBuses()) {
        proto_serialization::Bus bus;
        bus.set_name(bus_ptr->name);
        bus.set_is_circular(bus_ptr->is_circular);
        int num_of_ops = (bus_ptr->is_circular ? bus_ptr->stops.size() : bus_ptr->stops.size() / 2 + 1);
        for (int i = 0; i < num_of_ops; ++i) {
            proto_serialization::Stop stop;
            stop.set_name(bus_ptr->stops[i]->name);
            proto_serialization::Coordinates coords;
            coords.set_lat(bus_ptr->stops[i]->coordinates.lat);
            coords.set_lng(bus_ptr->stops[i]->coordinates.lng);
            *stop.mutable_coords() = coords;
            *bus.add_stops() = stop;
        }
        *proto_tc_.add_buses() = bus;
    }
}
    
void Serializer::SerializeDistances() {
    for (const auto& [stops_pair, dist] : tc_.GetAllDistances()) {
        proto_serialization::Distance distance;
        distance.set_from(stops_pair.first->name);
        distance.set_to(stops_pair.second->name);
        distance.set_distance(dist);
        *proto_tc_.add_distances() = distance;
    }
}
    
void Serializer::DeserializeCatalogue() {
    for (const auto& stop : proto_tc_.stops()) {
        tc_.AddStop({stop.name(), {stop.coords().lat(), stop.coords().lng()}});
    }
    
    for (const auto& distance : proto_tc_.distances()) {
        tcat::Stop* from = tc_.FindStop(distance.from());
        tcat::Stop* to = tc_.FindStop(distance.to());
        tc_.AddDistance(from, to, distance.distance());
    }
    
    for (const auto& bus : proto_tc_.buses()) {
        vector<string> stop_names;
        for (const auto& stop : bus.stops()) {
            stop_names.push_back(stop.name());
        }
        
        tc_.AddBus(
            {
                bus.name(),
                stop_names,
                bus.is_circular()
            });
    }
}
    
    
} //namespace serialization