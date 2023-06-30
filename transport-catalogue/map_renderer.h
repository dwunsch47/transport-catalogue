#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <utility>
#include <map>
#include <string>
#include <memory>

#include "geo.h"
#include "svg.h"
#include "transport_catalogue.h"

namespace map_r {
    
struct RenderSettings {
    double width;
    double height;
    double padding;
    double line_width;
    double stop_radius;
    int bus_label_font_size;
    svg::Point bus_label_offset;
    int stop_label_font_size;
    svg::Point stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width;
    std::vector<svg::Color> color_palette;
};
    
inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:
    SphereProjector() = default;
    
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};
    
class BusRoute : public svg::Drawable {
public:
    BusRoute(std::vector<svg::Point>& stops, const svg::Color& color, const RenderSettings& settings);
    void Draw(svg::ObjectContainer& container) const override;
private:
    std::vector<svg::Point> stops_;
    svg::Color color_;
    const RenderSettings& settings_;
};
    
class BusName : public svg::Drawable {
public:
    BusName(svg::Point bus_name_pos, std::string bus_name, svg::Color color, const RenderSettings& settings);
    void Draw(svg::ObjectContainer& container) const override;
private:
    svg::Point bus_name_pos_;
    std::string bus_name_;
    svg::Color color_;
    const RenderSettings& settings_;
};
    
class StopSymbol : public svg::Drawable {
public:
    StopSymbol(svg::Point center, const RenderSettings& settings);
    void Draw(svg::ObjectContainer& container) const override;
private:
    svg::Point center_;
    const RenderSettings& settings_;
};
    
class StopName : public svg::Drawable {
public:
    StopName(svg::Point stop_name_pos, std::string stop_name, const RenderSettings& settings);
    void Draw(svg::ObjectContainer& container) const override;
private:
    svg::Point stop_name_pos_;
    std::string stop_name_;
    const RenderSettings& settings_;
};

class MapRenderer {
public:
    MapRenderer() = default;
    
    void LoadSettings(RenderSettings settings);
    const RenderSettings& GetSettings() const;
    void MakeBusRoutes(const std::map<std::string, tcat::RenderData>& bus_to_stop_coords,
                        std::vector<std::unique_ptr<svg::Drawable>>& picture);
    void MakeBusNames(const std::map<std::string, tcat::RenderData>& bus_to_stop_coords,
                        std::vector<std::unique_ptr<svg::Drawable>>& picture);
    void MakeStopSymbols(const std::map<std::string, geo::Coordinates>& unique_stops, 
                        std::vector<std::unique_ptr<svg::Drawable>>& picture);
    void MakeStopNames(const std::map<std::string, geo::Coordinates>& unique_stops, 
                        std::vector<std::unique_ptr<svg::Drawable>>& picture);
    
    svg::Document RenderMap(const tcat::TransportCatalogue& catalogue);
    
    template <typename DrawableIterator>
    void DrawPicture(DrawableIterator begin, DrawableIterator end, svg::ObjectContainer& target)
    {
        for (auto it = begin; it != end; ++it)
        {
            (*it)->Draw(target);
        }
    }

    template <typename Container>
    void DrawPicture(const Container& container, svg::ObjectContainer& target)
    {
        DrawPicture(begin(container), end(container), target);
    }
    
private:
    uint32_t current_color_ = 0;
    const svg::Color GetCurrentColor();
    void ResetCurrentColor();
    RenderSettings settings_;
    SphereProjector sp_;
};
}