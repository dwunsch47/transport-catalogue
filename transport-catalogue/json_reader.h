#pragma once
#include <iostream>
#include <memory>
#include <string_view>

#include "json.h"
#include "transport_catalogue.h"
#include "domain.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace io {
class JsonReader {
public:
    JsonReader(tcat::TransportCatalogue& catalogue, map_r::MapRenderer& map_renderer_);
    void LoadBaseQueries(std::istream& input);
    void LoadStatQueries(std::ostream& output);
private:
    void ParseBaseRequests(const json::Array& base_requests) const;
    void ParseRenderRequests(const json::Dict& render_requests) const;
    void ParseRoutingRequests(const json::Dict& routing_requests) const;
    void ParseStatRequests(const json::Array& stat_requests, std::ostream& output) const;
    tcat::Stop ParseStop(const json::Dict& dict) const;
    tcat::PreBus ParsePreBus(const json::Dict& dict) const;
    tcat::StopDistances ParseStopDistances(const json::Dict& dict) const;
    svg::Color ParseColorData(const json::Node& color) const;
    json::Node OutputStopInfo(int id, const tcat::StopInfo& stop_info) const;
    json::Node OutputBusInfo(int id, const tcat::BusInfo& bus_info) const;
    json::Node OutputMap(int id) const;
    json::Node OutputRoute(int id, std::string_view from_stop, std::string_view to_stop) const;
    
    
    json::Document doc_{nullptr};
    tcat::TransportCatalogue& catalogue_;
    map_r::MapRenderer& map_renderer_;
    std::unique_ptr<router::TransportRouter> tr_ = nullptr;
};
}