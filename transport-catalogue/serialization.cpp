#include "serialization.h"
#include "transport_catalogue.pb.h"
#include "transport_router.pb.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "graph.h"

#include <string>
#include <fstream>
#include <algorithm>
#include <vector>
#include <memory>

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
    SerializeRenderSettings();
    SerializeRouterSettings();
    SerializeGraph();
    SerializeTransportRouter();
    
    proto_tc_.SerializeToOstream(&ofs);
}
    
shared_ptr<router::TransportRouter> Serializer::DeserializeFromFile(const string& file) {
    ifstream ifs(file, ios::binary);
    proto_tc_.Clear();
    proto_tc_.ParseFromIstream(&ifs);
    
    DeserializeCatalogue();
    DeserializeGraph();
    DeserializeRenderSettings();
    return DeserializeTransportRouter();
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
    
void Serializer::SerializeRenderSettings() {
    const map_r::RenderSettings& settings = mr_.GetSettings();
    proto_serialization::RenderSettings proto_settings;
    
    proto_settings.set_width(settings.width);
    proto_settings.set_height(settings.height);
    proto_settings.set_padding(settings.padding);
    proto_settings.set_line_width(settings.line_width);
    proto_settings.set_stop_radius(settings.stop_radius);
    proto_settings.set_bus_label_font_size(settings.bus_label_font_size);
    
    proto_serialization::Point bus_label_offset;
    bus_label_offset.set_x(settings.bus_label_offset.x);
    bus_label_offset.set_y(settings.bus_label_offset.y);
    *proto_settings.mutable_bus_label_offset() = bus_label_offset;
    
    proto_settings.set_stop_label_font_size(settings.stop_label_font_size);
    
    proto_serialization::Point stop_label_offset;
    stop_label_offset.set_x(settings.stop_label_offset.x);
    stop_label_offset.set_y(settings.stop_label_offset.y);
    *proto_settings.mutable_stop_label_offset() = stop_label_offset;
    
    *proto_settings.mutable_underlayer_color() = SerializeColor(settings.underlayer_color);
    proto_settings.set_underlayer_width(settings.underlayer_width);
    for (const svg::Color& color : settings.color_palette) {
        *proto_settings.add_color_palette() = SerializeColor(color);
    }
    
    *proto_tc_.mutable_render_settings() = proto_settings;
}
    
proto_serialization::Color Serializer::SerializeColor(const svg::Color& color) const {
    proto_serialization::Color result;
    if (holds_alternative<string>(color)) {
        result.set_color(get<string>(color));
    } else if (holds_alternative<svg::Rgb>(color)) {
        svg::Rgb rgb = get<svg::Rgb>(color);
        proto_serialization::Rgb proto_rgb;
        
        proto_rgb.set_red(rgb.red);
        proto_rgb.set_green(rgb.green);
        proto_rgb.set_blue(rgb.blue);

        *result.mutable_rgb() = proto_rgb;
    } else if (holds_alternative<svg::Rgba>(color)) {
        svg::Rgba rgba = get<svg::Rgba>(color);
        proto_serialization::Rgba proto_rgba;
        
        proto_rgba.set_red(rgba.red);
        proto_rgba.set_green(rgba.green);
        proto_rgba.set_blue(rgba.blue);
        proto_rgba.set_opacity(rgba.opacity);

        *result.mutable_rgba() = proto_rgba;
    }
    return result;
}
    
void Serializer::SerializeRouterSettings() {
    proto_serialization::RouterSettings proto_settings;
    router::RouterSettings settings = tr_ptr_->GetSettings();
    
    proto_settings.set_bus_wait_time(settings.bus_wait_time);
    proto_settings.set_bus_velocity(settings.bus_velocity);
    *proto_tc_.mutable_router_settings() = proto_settings;
}
    
void Serializer::SerializeGraph() {
    proto_serialization::Graph proto_graph;
    vector<graph::Edge<double>> edges = tr_ptr_->GetGraph().GetAllEdges();
    vector<vector<size_t>> incidence_lists = tr_ptr_->GetGraph().GetAllIncidenceLists();
    for (const auto& edge : edges) {
        proto_serialization::Edge proto_edge;
        proto_edge.set_from(edge.from);
        proto_edge.set_to(edge.to);
	proto_edge.set_name(edge.name);
        proto_edge.set_type(edge.type == graph::EdgeType::WAIT ? proto_serialization::Edge::WAIT : proto_serialization::Edge::TRAVEL);
        proto_edge.set_span_count(edge.span_count);
        proto_edge.set_weight(edge.weight);
        *proto_graph.add_edges() = proto_edge;
    }
    for (const auto& incidence_list : incidence_lists) {
        proto_serialization::IncidenceList list;
        for (const auto& id : incidence_list) {
            list.add_edge_ids(id);
        }
        *proto_graph.add_incidence_lists() = list;
    }
    *proto_tc_.mutable_transport_router()->mutable_graph() = proto_graph;
}
    
void Serializer::SerializeTransportRouter() {
    for (const auto& [name, id] : tr_ptr_->GetWaitVertexes()) {
        (*proto_tc_.mutable_transport_router()->mutable_wait_vertexes())[name] = id;
    }
    for (const auto& [name, id] : tr_ptr_->GetTravelVertexes()) {
        (*proto_tc_.mutable_transport_router()->mutable_travel_vertexes())[name] = id;
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
    
void Serializer::DeserializeRenderSettings() {
    map_r::RenderSettings settings;
    proto_serialization::RenderSettings proto_settings = proto_tc_.render_settings();
    
    settings.width = proto_settings.width();
    settings.height = proto_settings.height();
    settings.padding = proto_settings.padding();
    settings.line_width = proto_settings.line_width();
    settings.stop_radius = proto_settings.stop_radius();
    settings.bus_label_font_size = proto_settings.bus_label_font_size();
    settings.bus_label_offset = svg::Point{proto_settings.bus_label_offset().x(), proto_settings.bus_label_offset().y()};
    settings.stop_label_font_size = proto_settings.stop_label_font_size();
    settings.stop_label_offset = svg::Point{proto_settings.stop_label_offset().x(), proto_settings.stop_label_offset().y()};
    settings.underlayer_color = DeserializeColor(proto_settings.underlayer_color());
    settings.underlayer_width = proto_settings.underlayer_width();
    for (const auto& proto_color : proto_settings.color_palette()) {
        settings.color_palette.push_back(DeserializeColor(proto_color));
    }
    mr_.LoadSettings(settings);
}
    
svg::Color Serializer::DeserializeColor(const proto_serialization::Color& proto_color) const {
    if (proto_color.has_rgb()) {
        return svg::Rgb{
            proto_color.rgb().red(),
            proto_color.rgb().green(),
            proto_color.rgb().blue()
        };
    } else if (proto_color.has_rgba()) {
        return svg::Rgba{
            proto_color.rgba().red(),
            proto_color.rgba().green(),
            proto_color.rgba().blue(),
            proto_color.rgba().opacity()
        };
    } else {
        return {proto_color.color()};
    }
}
    
router::RouterSettings Serializer::DeserializeRouterSettings() {
    router::RouterSettings settings;
    proto_serialization::RouterSettings proto_settings = proto_tc_.router_settings();
    
    settings.bus_wait_time = proto_settings.bus_wait_time();
    settings.bus_velocity = proto_settings.bus_velocity();
    return settings;
}
    
graph::DirectedWeightedGraph<double> Serializer::DeserializeGraph() {
    vector<graph::Edge<double>> edges(proto_tc_.transport_router().graph().edges_size());
    vector<vector<graph::EdgeId>> incidence_lists(proto_tc_.transport_router().graph().incidence_lists_size());
    
    for (const auto& proto_edge : proto_tc_.transport_router().graph().edges()) {
        graph::Edge<double> edge {
            static_cast<graph::VertexId>(proto_edge.from()),
            static_cast<graph::VertexId>(proto_edge.to()),
	    proto_edge.name(),
            (proto_edge.type() == proto_serialization::Edge::WAIT ? graph::EdgeType::WAIT : graph::EdgeType::TRAVEL),
            proto_edge.span_count(),
            proto_edge.weight()
        };
        edges.push_back(edge);
    }
    
    for (const auto& proto_incidence_list : proto_tc_.transport_router().graph().incidence_lists()) {
        vector<graph::EdgeId> tmp_incidence_list;
        for (const auto& id : proto_incidence_list.edge_ids()) {
            tmp_incidence_list.push_back(static_cast<size_t>(id));
        }
        incidence_lists.push_back(tmp_incidence_list);
    }
    return graph::DirectedWeightedGraph<double>(edges, incidence_lists);
}
    
shared_ptr<router::TransportRouter> Serializer::DeserializeTransportRouter() {
    graph::DirectedWeightedGraph<double> graph = DeserializeGraph();
    unordered_map<string, size_t> wait_vertexes(proto_tc_.transport_router().wait_vertexes().begin(), proto_tc_.transport_router().wait_vertexes().end());
    unordered_map<string, size_t> travel_vertexes(proto_tc_.transport_router().travel_vertexes().begin(), proto_tc_.transport_router().travel_vertexes().end());
    /*unordered_map<string, size_t> wait_vertexes;
    for (const auto& [name, id] : proto_tc_.transport_router().wait_vertexes()) {
    	wait_vertexes[name] = id;
    }
    unordered_map<string, size_t> travel_vertexes;
    for (const auto& [name, id] : proto_tc_.transport_router().wait_vertexes()) {
    	travel_vertexes[name] = id;
    }*/

    shared_ptr<router::TransportRouter> tr = make_shared<router::TransportRouter>(tc_, graph, wait_vertexes, travel_vertexes);
    tr->LoadSettings(DeserializeRouterSettings());
    return tr;
}
    
    
} //namespace serialization
