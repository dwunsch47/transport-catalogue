#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <utility>
#include <memory>
#include <map>
#include <unordered_set>

#include "map_renderer.h"
#include "transport_catalogue.h"

using namespace std;

namespace map_r {

bool IsZero(double value) {
    return abs(value) < EPSILON;
}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
}
    
BusRoute::BusRoute(vector<svg::Point>& stops, const svg::Color& color, const RenderSettings& settings) :
    stops_(stops), color_(color), settings_(settings) {}
    
void BusRoute::Draw(svg::ObjectContainer& container) const {
    svg::Polyline route;
    for (const auto& stop : stops_) {
        route.AddPoint(stop);
    }
    route.SetStrokeColor(color_);
    route.SetFillColor(svg::NoneColor);
    route.SetStrokeWidth(settings_.line_width);
	route.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
	route.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	container.Add(route);
}
    
BusName::BusName(svg::Point bus_name_pos, string bus_name, svg::Color color, const RenderSettings& settings) :
    bus_name_pos_(bus_name_pos), bus_name_(move(bus_name)), color_(color), settings_(settings) {}

void BusName::Draw(svg::ObjectContainer& container) const {
    svg::Text busname;
    busname.SetPosition(bus_name_pos_);
    busname.SetOffset(settings_.bus_label_offset);
    busname.SetFontSize(settings_.bus_label_font_size);
    busname.SetFontFamily("Verdana"s);
    busname.SetFontWeight("bold"s);
    busname.SetData(bus_name_);
    
    svg::Text busname_base = busname;
    busname_base.SetFillColor(settings_.underlayer_color);
    busname_base.SetStrokeColor(settings_.underlayer_color);
    busname_base.SetStrokeWidth(settings_.underlayer_width);
    busname_base.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    busname_base.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    
    busname.SetFillColor(color_);
    
    container.Add(busname_base);
    container.Add(busname);
}
    
StopSymbol::StopSymbol(svg::Point center, const RenderSettings& settings) :
    center_(center), settings_(settings) {}
    
void StopSymbol::Draw(svg::ObjectContainer& container) const {
    svg::Circle symbol;
    symbol.SetCenter(center_);
    symbol.SetRadius(settings_.stop_radius);
    symbol.SetFillColor("white"s);
    container.Add(symbol);
}

StopName::StopName(svg::Point stop_name_pos, string stop_name, const RenderSettings& settings) :
    stop_name_pos_(stop_name_pos), stop_name_(move(stop_name)), settings_(settings) {}
    
void StopName::Draw(svg::ObjectContainer& container) const {
    svg::Text stopname;
    stopname.SetPosition(stop_name_pos_);
    stopname.SetOffset(settings_.stop_label_offset);
    stopname.SetFontSize(settings_.stop_label_font_size);
    stopname.SetFontFamily("Verdana"s);
    stopname.SetData(stop_name_);
    
    svg::Text stopname_base = stopname;
    stopname_base.SetFillColor(settings_.underlayer_color);
    stopname_base.SetStrokeColor(settings_.underlayer_color);
    stopname_base.SetStrokeWidth(settings_.underlayer_width);
    stopname_base.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    stopname_base.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    
    stopname.SetFillColor("black"s);
    
    container.Add(stopname_base);
    container.Add(stopname);
}

void MapRenderer::LoadSettings(RenderSettings settings) {
    settings_ = move(settings);
}
  
void MapRenderer::MakeBusRoutes(const map<string, tcat::RenderData>& bus_to_stop_coords, vector<unique_ptr<svg::Drawable>>& picture) {
    for (const auto& [bus_name, render_data] : bus_to_stop_coords) {
        vector<svg::Point> points;
        for (const auto& stop : render_data.stop_coords) {
            points.push_back(sp_(stop));
        }
        picture.push_back(make_unique<BusRoute>(points, GetCurrentColor(), settings_));
    }
}
    
void MapRenderer::MakeBusNames(const map<string, tcat::RenderData>& bus_to_stop_coords,
                              vector<unique_ptr<svg::Drawable>>& picture) {
    ResetCurrentColor();
    for (const auto& [bus_name, render_data] : bus_to_stop_coords) {
        svg::Point bus_name_pos = sp_(render_data.stop_coords.at(0));
        svg::Color current_color = GetCurrentColor();
        picture.push_back(make_unique<BusName>(bus_name_pos, bus_name, current_color, settings_));
        if (!render_data.is_circular && 
            render_data.stop_coords.at(0) != render_data.stop_coords.at((render_data.stop_coords.size() + 1) / 2 - 1)) {
            svg::Point bus_name_end_pos = sp_(render_data.stop_coords.at((render_data.stop_coords.size() + 1) / 2 - 1)); 
            picture.push_back(make_unique<BusName>(bus_name_end_pos, bus_name, current_color, settings_));
        }
    }
}
    
void MapRenderer::MakeStopSymbols(const map<string, geo::Coordinates>& unique_stops, 
                                 vector<unique_ptr<svg::Drawable>>& picture) {
    for (const auto& [_, coords] : unique_stops) {
        picture.push_back(make_unique<StopSymbol>(sp_(coords), settings_));
    }
}
    
void MapRenderer::MakeStopNames(const map<string, geo::Coordinates>& unique_stops,
                               vector<unique_ptr<svg::Drawable>>& picture) {
    for (const auto& [stop_name, coords] : unique_stops) {
        picture.push_back(make_unique<StopName>(sp_(coords), stop_name, settings_));
    }
}
    
const svg::Color MapRenderer::GetCurrentColor() {
    if (current_color_ == settings_.color_palette.size()) {
        current_color_ = 0;
    }
    return settings_.color_palette.at(current_color_++);
}
    
void MapRenderer::ResetCurrentColor() {
    current_color_ = 0;
}
    
svg::Document MapRenderer::RenderMap(const tcat::TransportCatalogue& catalogue) {
    map<string, tcat::RenderData> bus_to_stop_coords = move(catalogue.GetAllRoutes());
    unordered_set<geo::Coordinates, geo::CoordinatesHasher> all_coords;
    map<string, geo::Coordinates> unique_stops;
    for (const auto& [bus_name, render_data] : bus_to_stop_coords) {
        for (size_t i = 0; i < render_data.stop_coords.size(); ++i) {
            all_coords.insert(render_data.stop_coords.at(i));
            unique_stops[render_data.stop_names.at(i)] = render_data.stop_coords.at(i);
        }
    }
    SphereProjector sp {all_coords.begin(), all_coords.end(),
                       settings_.width, settings_.height, settings_.padding};
    sp_ = move(sp);
    
    vector<unique_ptr<svg::Drawable>> picture;
    
    MakeBusRoutes(bus_to_stop_coords, picture);
    MakeBusNames(bus_to_stop_coords, picture);
    MakeStopSymbols(unique_stops, picture);
    MakeStopNames(unique_stops, picture);
    
    svg::Document doc;
    DrawPicture(picture, doc);
    return doc;
}
    
}