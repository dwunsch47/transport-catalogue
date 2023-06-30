#include "serialization.h"
#include "transport_catalogue.pb.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

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
    SerializeRenderSettings();
    //SerializeRouterSettings();
    
    proto_tc_.SerializeToOstream(&ofs);
}
    
void Serializer::DeserializeFromFile(const string& file) {
    ifstream ifs(file, ios::binary);
    proto_tc_.Clear();
    proto_tc_.ParseFromIstream(&ifs);
    
    DeserializeCatalogue();
    DeserializeRenderSettings();
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
    
    
} //namespace serialization