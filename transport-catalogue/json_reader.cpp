#include "json.h"
#include "domain.h"
#include "json_reader.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "serialization.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <string_view>

using namespace std;

namespace io {
JsonReader::JsonReader(tcat::TransportCatalogue& catalogue, map_r::MapRenderer& map_renderer) : catalogue_(catalogue), map_renderer_(map_renderer) {}
    
    
void JsonReader::LoadBaseQueries(istream& input) {
    doc_ = json::Load(input);
    const json::Dict& dict = doc_.GetRoot().AsDict();
    
    const auto base_reqs = dict.find("base_requests"s);
    if (base_reqs != dict.end()) {
        ParseBaseRequests(base_reqs->second.AsArray());
    }
    
    tr_ = make_shared<router::TransportRouter>(catalogue_);
    
    const auto render_reqs = dict.find("render_settings"s);
    if (render_reqs != dict.end()) {
        ParseRenderRequests(render_reqs->second.AsDict());
    }
    
    const auto routing_reqs = dict.find("routing_settings"s);
    if (routing_reqs != dict.end()) {
        ParseRoutingRequests(routing_reqs->second.AsDict());
    }
    
    const auto serialization_reqs = dict.find("serialization_settings"s);
    if (serialization_reqs != dict.end()) {
        const string serialization_filename = ParseSerializationRequests(serialization_reqs->second.AsDict());
        serialization::Serializer serializer(catalogue_, tr_, map_renderer_);
        serializer.SerializeToFile(serialization_filename);
    }
}
    
void JsonReader::LoadStatQueries(istream& input, ostream& output) {
    doc_ = json::Load(input);
    const json::Dict& dict = doc_.GetRoot().AsDict();
    
    const auto serialization_reqs = dict.find("serialization_settings"s);
    if (serialization_reqs != dict.end()) {
        const string serialization_filename = ParseSerializationRequests(serialization_reqs->second.AsDict());
	serialization::Serializer serializer(catalogue_, nullptr, map_renderer_);
        tr_ = serializer.DeserializeFromFile(serialization_filename);

	//tr_ = make_shared<router::TransportRouter>(catalogue_);
        
        const auto stat_reqs = dict.find("stat_requests"s);
        if (stat_reqs != dict.end()) {
            ParseStatRequests(stat_reqs->second.AsArray(), output);
        }
        
    }
}
    
void JsonReader::ParseBaseRequests(const json::Array& base_requests) const {
    vector<tcat::Stop> stops;
    vector<tcat::PreBus> buses;
    vector<tcat::StopDistances> stop_distances;
    for (const auto& request : base_requests) {
        json::Dict req_root = request.AsDict();
        if (req_root.at("type"s).AsString() == "Stop"s) {
            stops.push_back(ParseStop(req_root));
            stop_distances.push_back(ParseStopDistances(req_root));
        } else if (req_root.at("type"s).AsString() == "Bus"s) {
            buses.push_back(ParsePreBus(req_root));
        }
    }
    for (const auto& stop : stops) {
        catalogue_.AddStop(stop);
    }
    
    for (const auto& stop_to_distances : stop_distances) {
        catalogue_.AddDistanceBetweenStops(stop_to_distances);
    }
    for (const auto& pre_bus : buses) {
        catalogue_.AddBus(pre_bus);
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
    pre_bus.is_circular = dict.at("is_roundtrip"s).AsBool();
    for (const auto& stop : dict.at("stops"s).AsArray()) {
        pre_stops.push_back(stop.AsString());
    }
    pre_bus.stops = move(pre_stops);
    return pre_bus;
}
    
tcat::StopDistances JsonReader::ParseStopDistances(const json::Dict& dict) const {
    tcat::StopDistances stop_with_dists;
    stop_with_dists.name = dict.at("name"s).AsString();
    for (const auto& [name, distance] : dict.at("road_distances"s).AsDict()) {
        stop_with_dists.stop_to_distance.emplace(name, distance.AsInt());
    }
    return stop_with_dists;
}
    
void JsonReader::ParseRenderRequests(const json::Dict& render_requests) const {
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
    map_renderer_.LoadSettings(render_data);
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
    
void JsonReader::ParseRoutingRequests(const json::Dict& routing_requests) const {
    router::RouterSettings router_settings {
        routing_requests.at("bus_wait_time"s).AsInt(),
        routing_requests.at("bus_velocity"s).AsDouble()
    };
    tr_->LoadSettings(router_settings);
}
    
string JsonReader::ParseSerializationRequests(const json::Dict& serialization_requests) const {
    return serialization_requests.at("file"s).AsString();
}

void JsonReader::ParseStatRequests(const json::Array& stat_requests, ostream& output) const {
    json::Array queries;
    for (const auto& request : stat_requests) {
        json::Dict stat_root = request.AsDict();
        int id = stat_root.at("id"s).AsInt();
        string type = stat_root.at("type"s).AsString();
        if (type == "Stop"s) {
            const tcat::StopInfo& stop_info = catalogue_.GetStopInfo(stat_root.at("name"s).AsString());
            queries.push_back(OutputStopInfo(id, stop_info));
        } else if (type == "Bus"s) {
            const tcat::BusInfo& bus_info = catalogue_.GetBusInfo(stat_root.at("name"s).AsString());
            queries.push_back(OutputBusInfo(id, bus_info));
        } else if (type == "Map"s) {
            queries.push_back(OutputMap(id));
        } else if (type == "Route"s) {
            string from = stat_root.at("from"s).AsString();
            string to = stat_root.at("to"s).AsString();
            queries.push_back(OutputRoute(id, from, to));
        }
    }
    json::Print(json::Document{queries}, output);
}
    
json::Node JsonReader::OutputStopInfo(int id, const tcat::StopInfo& stop_info) const {
    if (stop_info.status == tcat::StopInfoStatus::NOT_FOUND) {
        return json::Builder{}.StartDict()
            .Key("request_id"s).Value(id)
            .Key("error_message"s).Value("not found"s)
            .EndDict()
            .Build();
    } else {
        json::Array routes;
        for (const auto& bus : stop_info.buses) {
            routes.push_back(json::Node(string(bus)));
        }
        
        return json::Builder{}.StartDict()
            .Key("buses"s).Value(routes)
            .Key("request_id"s).Value(id)
            .EndDict()
            .Build();
    }
}
    
json::Node JsonReader::OutputBusInfo(int id, const tcat::BusInfo& bus_info) const {
    if (bus_info.stops == 0) {
        return json::Builder{}.StartDict()
            .Key("request_id"s).Value(id)
            .Key("error_message"s).Value("not found"s)
            .EndDict()
            .Build();
        
    } else {
        return json::Builder{}.StartDict()
            .Key("curvature"s).Value(bus_info.curvature)
            .Key("request_id"s).Value(id)
            .Key("route_length"s).Value(bus_info.route_length)
            .Key("stop_count"s).Value(bus_info.stops)
            .Key("unique_stop_count"s).Value(bus_info.unique_stops)
            .EndDict()
            .Build();
    }
}
    
json::Node JsonReader::OutputMap(int id) const {
    svg::Document map = map_renderer_.RenderMap(catalogue_);
    ostringstream out;
    map.Render(out);
    return json::Builder{}.StartDict()
        .Key("map"s).Value(out.str())
        .Key("request_id"s).Value(id)
        .EndDict()
        .Build();
}
    
json::Node JsonReader::OutputRoute(int id, const string& from_stop, const string& to_stop) const {
    //string from = string(from_stop);
    //string to = string(to_stop);
    auto route = tr_->CalculateRoute(from_stop, to_stop);
    if (!route.is_found) {
        return json::Builder{}.StartDict()
            .Key("request_id"s).Value(id)
            .Key("error_message"s).Value("not found"s)
            .EndDict()
            .Build();
    }
    json::Array items;
	for (const auto& item : route.items)
	{
		json::Dict items_map;
		if (item.type == graph::EdgeType::TRAVEL)
		{
			items_map["type"] = "Bus"s;
			items_map["bus"] = item.name;
			items_map["span_count"] = item.span_count;
		}
		else if (item.type == graph::EdgeType::WAIT)
		{
			items_map["type"] = "Wait"s;
			items_map["stop_name"] = item.name;
		}
		items_map["time"] = item.time;
		items.push_back(items_map);
	}
	return json::Builder{}.StartDict()
		.Key("request_id").Value(id)
		.Key("total_time").Value(route.total_time)
		.Key("items").Value(items)
		.EndDict()
		.Build();
}
    
}
