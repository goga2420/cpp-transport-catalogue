#include "svg.h"

namespace svg {

using namespace std::literals;

struct RgbPrint {
    std::ostream& out;

    void operator()(std::monostate) const {
        out << "none"sv;
    }
    void operator()(std::string color) const {
        out << color;
    }
    void operator()(svg::Rgb color) const {
        //out << "rgb("<<color.GetRed()<< ","s<< color.GetGreen() <<","s << color.GetBlue() <<")"s;
        out << "rgb(" << static_cast<int>(color.red) << ","
               << static_cast<int>(color.green) << ","
               << static_cast<int>(color.blue) << ")";
    }
    void operator()(svg::Rgba color) const {
        out << "rgba(" << static_cast<int>(color.red) << ","
               << static_cast<int>(color.green) << ","
               << static_cast<int>(color.blue) << "," << static_cast<double>(color.opacity) <<")"s;
    }
};

std::ostream& operator<<(std::ostream& os,  Color color){
    std::visit(RgbPrint{os}, color);
    return os;
}

std::ostream& operator<<(std::ostream& os, StrokeLineCap line_cap) {
    switch (line_cap) {
        case StrokeLineCap::BUTT:
            os << "butt";
            break;
        case StrokeLineCap::ROUND:
            os << "round";
            break;
        case StrokeLineCap::SQUARE:
            os << "square";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, StrokeLineJoin line_join) {
    switch (line_join) {
        case StrokeLineJoin::ARCS:
            os << "arcs";
            break;
        case StrokeLineJoin::BEVEL:
            os << "bevel";
            break;
        case StrokeLineJoin::MITER:
            os << "miter";
            break;
        case StrokeLineJoin::MITER_CLIP:
            os << "miter-clip";
            break;
        case StrokeLineJoin::ROUND:
            os << "round";
            break;
    }
    return os;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

//    context.out << " " ;
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
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ------------------
Polyline& Polyline::AddPoint(Point point){
    points_.push_back(point);
    MakeStrPoint();
    return *this;
}

void Polyline::MakeStrPoint(){
    std::ostringstream oss;

    for(int i =0; i < static_cast<int>(points_.size()); i++)
    {
        if(i == static_cast<int>(points_.size())-1)
        {
            oss << points_[i].x << "," << points_[i].y << "";
            break;
        }

        oss << points_[i].x << "," << points_[i].y << " ";
    }
    points_str_ = oss.str();
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv << points_str_ << "\" "sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

//________Text___________
Text& Text::SetPosition(Point pos){
    pos_ = pos;
    return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset){
    offset_ = offset;
    return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size){
    size_ = size;
    return *this;
}

// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family){
    font_family_ = font_family;
    return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight){
    font_weight_ = font_weight;
    return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data){
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text ";
    RenderAttrs(context.out);
    out << " x=\"" << pos_.x << "\" y=\"" << pos_.y << "\" ";

    // Render offset if it's not zero
    //    if (offset_.x != 0 || offset_.y != 0) {
    out << "dx=\"" << offset_.x << "\" dy=\"" << offset_.y << "\" ";
    //}

    // Render font size
    if(!font_family_.empty())
        out << "font-size=\"" << size_ << "\" ";
    else
        out << "font-size=\"" << size_ << "\"";

    // Render font family if provided
    if (!font_family_.empty()) {
        if(font_weight_.empty())
            out << "font-family=\"" << font_family_ << "\"";
        else
            out << "font-family=\"" << font_family_ << "\" ";
    }

    // Render font weight if provided
    if (!font_weight_.empty()) {
        out << "font-weight=\"" << font_weight_ << "\"";
    }

    // End of attributes, closing the opening <text> tag
    out << ">";


    // Escape special characters in the data and render it
    for (char c : data_) {
        switch (c) {
            case '"':
                out << "&quot;";
                break;
            case '\'':
                out << "&apos;";
                break;
            case '<':
                out << "&lt;";
                break;
            case '>':
                out << "&gt;";
                break;
            case '&':
                out << "&amp;";
                break;
            default:
                out << c;
                break;
        }
    }

    // Close the <text> tag
    out << "</text>" ;
//    << std::endl;
}

//_____Document________
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << std::endl;

    for (const auto& obj : objects_) {
        obj->Render(RenderContext(out));
    }

    out << "</svg>" << std::endl;
}

//____ObjectContainer___
//void ObjectContainer::AddPtr(std::unique_ptr<Object>&& obj) {
//    objects_.emplace_back(std::move(obj));
//}

}  // namespace svg
