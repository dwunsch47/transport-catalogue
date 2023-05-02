#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include "json.h"
#include "domain.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

using namespace std;

namespace io {
void JsonReader::LoadBaseQueries(istream& input, ostream& /*output*/, tcat::TransportCatalogue& catalogue, map_r::MapRenderer& m_r) {
    doc_ = json::Load(input);
    const json::Dict dict = doc_.GetRoot().AsMap();
    
    const auto base_reqs = dict.find("base_requests"s);
    if (base_reqs != dict.end()) {
        ParseBaseRequests(base_reqs->second.AsArray(), catalogue);
    }
    
    const auto render_reqs = dict.find("render_settings"s);
    if (render_reqs != dict.end()) {
        ParseRenderRequests(render_reqs->second.AsMap(), m_r);
    }
}
    
void JsonReader::LoadStatQueries(istream& /*input*/, ostream& output, tcat::TransportCatalogue& catalogue, map_r::MapRenderer& m_r) {
    const json::Dict dict = doc_.GetRoot().AsMap();
    
    const auto stat_reqs = dict.find("stat_requests"s);
    if (stat_reqs != dict.end()) {
        ParseStatRequests(stat_reqs->second.AsArray(), output, catalogue, m_r);
    } 
}
    
void JsonReader::ParseBaseRequests(json::Array base_requests, tcat::TransportCatalogue& catalogue) const {
    vector<tcat::Stop> stops;
    vector<tcat::PreBus> buses;
    vector<tcat::StopDistances> stop_distances;
    for (const auto& request : base_requests) {
        json::Dict req_root = request.AsMap();
        if (req_root.at("type"s).AsString() == "Stop"s) {
            stops.push_back(ParseStop(req_root));
            stop_distances.push_back(ParseStopDistances(req_root));
        } else if (req_root.at("type"s).AsString() == "Bus"s) {
            buses.push_back(ParsePreBus(req_root));
        }
    }
    for (const auto& stop : stops) {
        catalogue.AddStop(stop);
    }
    
    for (const auto& stop_to_distances : stop_distances) {
        catalogue.AddDistanceBetweenStops(stop_to_distances);
    }
    for (const auto& pre_bus : buses) {
        catalogue.AddBus(pre_bus);
    }
}
    
tcat::Stop JsonReader::ParseStop(const json::Dict& dict) const {
    tcat::Stop complete_stop;
    complete_stop.name = dict.at("name"s).AsString();
    complete_stop.coordinates = {dict.at("latitude"s).AsDouble(), dict.at("longitude"s).AsDouble()};
    return complete_stop;
}
    
tcat::PreBus JsonReader::ParsePreBus(const json::Dict& dict) const {
    tcat::PreBus pre_bus;
    vector<string> pre_stops;
    pre_bus.name = dict.at("name"s).AsString();
    pre_bus.type = (dict.at("is_roundtrip"s).AsBool() ? "circular"s : "usual"s);
    for (const auto& stop : dict.at("stops"s).AsArray()) {
        pre_stops.push_back(stop.AsString());
    }
    if (pre_bus.type == "usual"s) {
        vector<string> pre(pre_stops.begin(), pre_stops.end() - 1);
        reverse(pre.begin(), pre.end());
        for (const auto& stop : pre) {
            pre_stops.push_back(stop);
        }
    }
    pre_bus.stops = move(pre_stops);
    return pre_bus;
}
    
tcat::StopDistances JsonReader::ParseStopDistances(const json::Dict& dict) const {
    tcat::StopDistances stop_with_dists;
    stop_with_dists.name = dict.at("name"s).AsString();
    for (const auto& [name, distance] : dict.at("road_distances"s).AsMap()) {
        stop_with_dists.stop_to_distance.emplace(name, distance.AsInt());
    }
    return stop_with_dists;
}
    
void JsonReader::ParseRenderRequests(json::Dict render_requests, map_r::MapRenderer& m_r) const {
    vector<svg::Color> complete_color_palette;
    for (const json::Node& color : render_requests.at("color_palette"s).AsArray()) {
        complete_color_palette.push_back(ParseColorData(color));
    }
    
    map_r::RenderSettings render_data {
        render_requests.at("width"s).AsDouble(),
        render_requests.at("height"s).AsDouble(),
        render_requests.at("padding"s).AsDouble(),
        render_requests.at("line_width"s).AsDouble(),
        render_requests.at("stop_radius"s).AsDouble(),
        render_requests.at("bus_label_font_size"s).AsInt(),
        {render_requests.at("bus_label_offset"s).AsArray().at(0).AsDouble(), render_requests.at("bus_label_offset"s).AsArray().at(1).AsDouble()},
        render_requests.at("stop_label_font_size"s).AsInt(),
        {render_requests.at("stop_label_offset"s).AsArray().at(0).AsDouble(), render_requests.at("stop_label_offset"s).AsArray().at(1).AsDouble()},
        ParseColorData(render_requests.at("underlayer_color"s)),
        render_requests.at("underlayer_width"s).AsDouble(),
        move(complete_color_palette)
    };
    m_r.LoadRenderSettings(render_data);
    //svg::Document map = m_r.RenderMap(catalogue);
    //map.Render(output);
}
   
svg::Color JsonReader::ParseColorData(const json::Node& color) const {
    if (color.IsString()) {
        return color.AsString();
    }
    else if (color.IsArray()) {
        if (color.AsArray().size() == 3) {
            return svg::Rgb {
                static_cast<uint8_t>(color.AsArray().at(0).AsInt()),
                static_cast<uint8_t>(color.AsArray().at(1).AsInt()),
                static_cast<uint8_t>(color.AsArray().at(2).AsInt())
            };
        } else if (color.AsArray().size() == 4) {
            return svg::Rgba {
                static_cast<uint8_t>(color.AsArray().at(0).AsInt()),
                static_cast<uint8_t>(color.AsArray().at(1).AsInt()),
                static_cast<uint8_t>(color.AsArray().at(2).AsInt()),
                color.AsArray().at(3).AsDouble()
            };
        }
    }
    return svg::Color();
}

void JsonReader::ParseStatRequests(json::Array stat_requests, ostream& output, tcat::TransportCatalogue& catalogue, map_r::MapRenderer& m_r) const {
    json::Array queries;
    for (const auto& request : stat_requests) {
        json::Dict stat_root = request.AsMap();
        //string name = stat_root.at("name"s).AsString();
        int id = stat_root.at("id"s).AsInt();
        string type = stat_root.at("type"s).AsString();
        if (type == "Stop"s) {
            queries.push_back(OutputStopInfo(id, catalogue.GetStopInfo(stat_root.at("name"s).AsString())));
        } else if (type == "Bus"s) {
            queries.push_back(OutputBusInfo(id, catalogue.GetBusInfo(stat_root.at("name"s).AsString())));
        } else if (type == "Map"s) {
            queries.push_back(OutputMap(id, catalogue, m_r));
        }
    }
    json::Print(json::Document{queries}, output);
}
    
json::Node JsonReader::OutputStopInfo(int id, const tcat::StopInfo& stop_info) const {
    if (stop_info.status == tcat::StopInfoStatus::NOT_FOUND) {
        return json::Dict {
            {"request_id"s, id},
            {"error_message"s, "not found"s}
        };
    } else {
        json::Array routes;
        for (const auto& bus : stop_info.buses) {
            routes.push_back(json::Node(string(bus)));
        }
        return json::Dict {
            {"buses"s, routes},
            {"request_id"s, id}
        };
    }
}
    
json::Node JsonReader::OutputBusInfo(int id, const tcat::BusInfo& bus_info) const {
    if (bus_info.stops == 0) {
        return json::Dict {
            {"request_id"s, id},
            {"error_message"s, "not found"s}
        };
    } else {
        return json::Dict {
            {"curvature"s, bus_info.curvature},
            {"request_id"s, id},
            {"route_length"s, bus_info.route_length},
            {"stop_count"s, bus_info.stops},
            {"unique_stop_count"s, bus_info.unique_stops}
        };
    }
}
    
json::Node JsonReader::OutputMap(int id, tcat::TransportCatalogue& catalogue, map_r::MapRenderer& m_r) const {
    svg::Document map = m_r.RenderMap(catalogue);
    ostringstream out;
    map.Render(out);
    return json::Dict {
        {"map"s, out.str()},
        {"request_id"s, id}
    };
}
    
}