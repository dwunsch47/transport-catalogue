#pragma once
#include <iostream>

#include "json.h"
#include "transport_catalogue.h"
#include "domain.h"
#include "map_renderer.h"

namespace io {
class JsonReader {
public:
    JsonReader() = default;
    void LoadBaseQueries(std::istream& input, tcat::TransportCatalogue& catalogue, map_r::MapRenderer& m_r);
    void LoadStatQueries(std::ostream& output, tcat::TransportCatalogue& catalogue, map_r::MapRenderer& m_r);
private:
    void ParseBaseRequests(json::Array base_requests, tcat::TransportCatalogue& catalogue) const;
    void ParseRenderRequests(json::Dict render_requests, map_r::MapRenderer& m_r) const;
    void ParseStatRequests(json::Array stat_requests, std::ostream& output, tcat::TransportCatalogue& catalogue, map_r::MapRenderer& m_r) const;
    tcat::Stop ParseStop(const json::Dict& dict) const;
    tcat::PreBus ParsePreBus(const json::Dict& dict) const;
    tcat::StopDistances ParseStopDistances(const json::Dict& dict) const;
    svg::Color ParseColorData(const json::Node& color) const;
    json::Node OutputStopInfo(int id, const tcat::StopInfo& stop_info) const;
    json::Node OutputBusInfo(int id, const tcat::BusInfo& bus_info) const;
    json::Node OutputMap(int id, tcat::TransportCatalogue& catalogue, map_r::MapRenderer& m_r) const;
    json::Document doc_{nullptr};
};
}