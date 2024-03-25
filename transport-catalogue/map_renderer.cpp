#include "map_renderer.h"


namespace render {
inline const double EPSILON = 1e-6;
bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

void FillText(RenderSettings& render_settings, const geo::Coordinates& point, svg::Text& text, const render::SphereProjector& proj, std::string bus_name) {
    text.SetPosition(proj(point)).SetOffset(svg::Point(render_settings.bus_label_offset[0], render_settings.bus_label_offset[1])).SetFontSize(render_settings.bus_label_font_size).SetFontFamily("Verdana").SetData(bus_name).SetFontWeight("bold").SetFillColor("black");

    }

void FillStops(RenderSettings& render_settings, svg::Document& map, const render::SphereProjector& proj, std::string stop_name, geo::Coordinates point, std::vector<svg::Text>& stop) {
    svg::Circle circle;
    circle.SetCenter(proj(point)).SetRadius(render_settings.stop_radius).SetFillColor("white");
    map.Add(circle);

    svg::Text name_stop;
    name_stop.SetPosition(proj(point)).SetOffset(svg::Point(render_settings.stop_label_offset[0], render_settings.stop_label_offset[1])).SetFontSize(render_settings.stop_label_font_size).SetFontFamily("Verdana").SetData(stop_name);
    svg::Text cover = name_stop;
    cover.SetFillColor(render_settings.underlayer_color).SetStrokeColor(render_settings.underlayer_color).SetStrokeWidth(render_settings.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    name_stop.SetFillColor("black");
    stop.push_back(cover);
    stop.push_back(name_stop);
}

void FillMap(std::vector<geo::Coordinates> stops, svg::Document& map, RenderSettings& render_settings, const render::SphereProjector& proj, int& number, bool is_roundtrip, std::string bus_name, std::vector<svg::Text>& bus) {
    if (stops.size() == 0) {
        return;
    }

    svg::Polyline polyline;
    polyline.SetStrokeWidth(render_settings.line_width);
    polyline.SetStrokeColor(render_settings.color_palette[number % render_settings.color_palette.size()]);
    polyline.SetFillColor(svg::Color());
    polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    for (const auto& point : stops) {
        polyline.AddPoint(proj(point));
    }
    map.Add(polyline);

    svg::Text cover;
    FillText(render_settings, stops[0], cover, proj, bus_name);
    cover.SetFillColor(render_settings.underlayer_color).SetStrokeColor(render_settings.underlayer_color).SetStrokeWidth(render_settings.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    bus.push_back(cover);

    svg::Text name_bus;
    FillText(render_settings, stops[0], name_bus, proj, bus_name);
    name_bus.SetFillColor(render_settings.color_palette[number % render_settings.color_palette.size()]);
    bus.push_back(name_bus);

    if (!is_roundtrip && (stops[0]!= stops[stops.size()/2])) {
        svg::Text underlayer_copy = cover;
        underlayer_copy.SetPosition(proj(stops[stops.size() / 2]));
        bus.push_back(underlayer_copy);
        svg::Text bus_label_copy = name_bus;
        bus_label_copy.SetPosition(proj(stops[stops.size() / 2]));
        bus.push_back(bus_label_copy);
    }
    number += 1;
}
}
