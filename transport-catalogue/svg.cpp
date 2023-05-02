#include "svg.h"

namespace svg {

using namespace std::literals;
    
std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap) {
    switch(line_cap) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
    }
    return out;
}
std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join) {
    switch(line_join) {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
    }
    return out;
}
    
Rgb::Rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}
    
Rgba::Rgba(uint8_t r, uint8_t g, uint8_t b, double o) : red(r), green(g), blue(b), opacity(o) {}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}
    

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}
    
//----------------------Polyline---------------//
    
Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}
    
void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool is_first = true;
    for (const Point& point : points_) {
        if (is_first) {
            out << point.x << ',' << point.y;
            is_first = false;
        } else {
        out << ' ' << point.x << ',' << point.y;
        }
    }
    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}
    
//------------------------Text------------------------
    
Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

    // Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

    // Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = move(font_family);
    return *this;
}

    // Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = move(font_weight);
    return *this;
}

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data) {
    data_ = move(data);
    return *this;
}
    
void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;
    RenderAttrs(out);
    out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\""sv;
    out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
    out << " font-size=\"" << font_size_ << "\""sv;
    if (font_family_.size() > 0)
    {
        out << " font-family=\"" << font_family_ << "\""sv;
    }
    if (font_weight_.size() > 0)
    {
        out << " font-weight=\"" << font_weight_ << "\""sv;
    }
     out << ">"sv;
    for (const char& symb : data_) {
        if (symb == '"') {
            out << "&quot;"sv;
        } else 
        if (symb == '\'') {
            out << "&apos;"sv;
        } else 
        if (symb == '<') {
            out << "&lt;"sv;
        } else
        if (symb == '>') {
            out << "&gt;"sv;
        } else
        if (symb == '&') {
            out << "&amp;"sv;
        } else {
            out << symb;
        }
    }
    out << "</text>"sv;
}
    
// Добавляет в svg-документ объект-наследник svg::Object
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(move(obj));
}

    // Выводит в ostream svg-представление документа
void Document::Render(std::ostream& out) const {
    RenderContext ctx(out, 2, 2);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    
    for (const auto& obj : objects_) {
        obj->Render(ctx);
    }
    
    out << "</svg>"sv;
}

}  // namespace svg