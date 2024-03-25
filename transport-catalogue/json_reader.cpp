#include "json_reader.h"

json::Document LoadJSON(const std::string& s) {
    std::istringstream strm(s);
    return json::Load(strm);
}

void BaseRequest(catalogue::TransportCatalogue& catalogue, std::istream& input, std::ostream& output) {
    json::Document command = json::Load(input);
    ApplyCommands(command, catalogue);
    //ApplyRenderSettings(command, catalogue, output);
    ParseAndPrintStat(command, catalogue, output);
}

std::string Print(const json::Node& node) {
    std::ostringstream out;
    json::Print(json::Document{ node }, out);
    return out.str();
}

std::unordered_map < std::string_view, int> ParseStopDistances(const json::Dict& stops) {
    std::unordered_map<std::string_view, int> result;
    for (const auto& stop : stops) {
        result.emplace(stop.first, stop.second.AsInt());
    }
    return result;
}

std::vector<std::string_view> ParseRoute(const json::Array& route, const bool& is_roundtrip) {
    std::vector<std::string_view> result;
    for (const auto& stop : route) {
        result.push_back(stop.AsString());
    }
    if (!is_roundtrip) {
        for (size_t i = (route.size() - 2); i > 0; i--) {
            result.push_back(route[i].AsString());
        }
        result.push_back(route[0].AsString());
    }
    return result;
}

void ApplyCommands(json::Document& commands, catalogue::TransportCatalogue& catalogue) {
    if (!commands.GetRoot().IsMap()) {
        return;
    }
    std::unordered_map<std::string, json::Dict> stop_distances;
    json::Array buses;
    json::Array stops;
    const auto& com = commands.GetRoot().AsMap().at("base_requests").AsArray();
    //const auto& visio = commands.GetRoot().AsMap().at("render_settings");
    
    for (const auto& info : com) {
        if (info.AsMap().at("type").AsString() == "Stop") {
            stops.push_back(info);
            stop_distances[info.AsMap().at("name").AsString()] = info.AsMap().at("road_distances").AsMap();
//            std::unordered_map<std::string_view, int> stops_length;
//            for(auto& [key, value]:info.AsMap().at("road_distances").AsMap()){
//                stops_length.insert({key, value.AsDouble()});
//                //std::cout<<std::endl;
//            }
            catalogue.AddStop(info.AsMap().at("name").AsString(), geo::Coordinates{ info.AsMap().at("latitude").AsDouble(), info.AsMap().at("longitude").AsDouble() });
        }
        else if (info.AsMap().at("type").AsString() == "Bus") {
            buses.push_back(info);
        }
    }

    for (const auto& stop : stop_distances) {
        catalogue.AddStopDistances(stop.first, ParseStopDistances(stop.second));
    }

    for (const auto& bus : buses) {
        catalogue.AddRoute(bus.AsMap().at("name").AsString(), ParseRoute(bus.AsMap().at("stops").AsArray(), bus.AsMap().at("is_roundtrip").AsBool()));
    }
    
    for (const auto& stop : stops) {

        catalogue.AddStop(stop.AsMap().at("name").AsString(), geo::Coordinates{ stop.AsMap().at("latitude").AsDouble(), stop.AsMap().at("longitude").AsDouble() });
    }
    
}



json::Array MakeArray(std::set<std::string_view> original) {
    json::Array result;
    for (auto& element : original) {
        result.push_back(std::string(element));
    }
    return result;
}

svg::Color FillColor(json::Node colors) {
    if (colors.IsArray()) {
        if (colors.AsArray().size() == 3) {
            return svg::Color(svg::Rgb(colors.AsArray()[0].AsInt(), colors.AsArray()[1].AsInt(), colors.AsArray()[2].AsInt()));
        }
        else {
            return svg::Color(svg::Rgba(colors.AsArray()[0].AsInt(), colors.AsArray()[1].AsInt(), colors.AsArray()[2].AsInt(), colors.AsArray()[3].AsDouble()));
        }
    }
    return svg::Color(colors.AsString());
}

render::RenderSettings FillRenderSettings(const json::Dict& setting) {
    render::RenderSettings renderer_settings;
    renderer_settings.width = setting.at("width").AsDouble();
    renderer_settings.height = setting.at("height").AsDouble();
    renderer_settings.padding = setting.at("padding").AsDouble();
    renderer_settings.line_width = setting.at("line_width").AsDouble();
    renderer_settings.stop_radius = setting.at("stop_radius").AsDouble();
    renderer_settings.bus_label_font_size = setting.at("bus_label_font_size").AsInt();
    for (const auto& elem : setting.at("bus_label_offset").AsArray()) {
        renderer_settings.bus_label_offset.push_back(elem.AsDouble());
    }
    renderer_settings.stop_label_font_size = setting.at("stop_label_font_size").AsInt();
    for (const auto& elem : setting.at("stop_label_offset").AsArray()) {
        renderer_settings.stop_label_offset.push_back(elem.AsDouble());
    }
    renderer_settings.underlayer_color = FillColor(setting.at("underlayer_color"));
    renderer_settings.underlayer_width = setting.at("underlayer_width").AsDouble();
    for (const auto& color : setting.at("color_palette").AsArray()) {
        renderer_settings.color_palette.emplace_back(FillColor(color));
    }
    return renderer_settings;

}

std::vector<geo::Coordinates> GetVectorOfCoordinates(const std::vector<std::string_view> stops, const catalogue::TransportCatalogue& catalogue) {
    std::vector<geo::Coordinates> result;
    for (const auto& stop : stops) {
        result.push_back(geo::Coordinates{catalogue.SearchStop(stop).value()->lanlong.lat,catalogue.SearchStop(stop).value()->lanlong.lng });
    }
    return result;
}



std::vector<geo::Coordinates> AllCoordinates(std::set<std::string> stops,const  catalogue::TransportCatalogue& catalogue) {
    std::vector<geo::Coordinates> result;
    for (const auto& stop : stops) {
        if(catalogue.GetStopBuses(stop) != std::nullopt && !catalogue.GetStopBuses(stop).value().buses_to_stop.empty())
            result.push_back(geo::Coordinates{ catalogue.SearchStop(stop).value()->lanlong.lat, catalogue.SearchStop(stop).value()->lanlong.lng });
    }
    return result;
}

void ApplyRenderSettings(json::Document& commands,const catalogue::TransportCatalogue& catalogue, std::ostream& out) {
    render::RenderSettings render_settings = FillRenderSettings(commands.GetRoot().AsMap().at("render_settings").AsMap());
    svg::Document map;
    std::vector<svg::Text> bus_label, stop_label;
    std::map<std::string, bool> buses;
    std::set<std::string> stops;
    const auto& com = commands.GetRoot().AsMap().at("base_requests").AsArray();
    for (const auto& info : com) {
        if (info.AsMap().at("type").AsString() == "Bus") {
            buses.insert({ info.AsMap().at("name").AsString(), info.AsMap().at("is_roundtrip").AsBool() });
        }
        else if (info.AsMap().at("type").AsString() == "Stop") {
            stops.insert(info.AsMap().at("name").AsString());
        }
    }
    std::vector<geo::Coordinates> coordinates = AllCoordinates(stops,catalogue);
    const render::SphereProjector proj{ coordinates.begin(),coordinates.end(), render_settings.width, render_settings.height, render_settings.padding };
    int counter = 0;
    for (auto& bus : buses) {
        render::FillMap(GetVectorOfCoordinates(catalogue.SearchRoute(bus.first).value()->route_stops_, catalogue), map, render_settings,proj, counter, bus.second,bus.first, bus_label);
    }
    for (auto& bus : bus_label) {
        map.Add(bus);
    }

    for (auto& stop : stops) {
        if (catalogue.GetStopBuses(stop) != std::nullopt && !catalogue.GetStopBuses(stop).value().buses_to_stop.empty() ) {
            render::FillStops(render_settings, map, proj, stop, geo::Coordinates{ catalogue.SearchStop(stop).value()->lanlong.lat, catalogue.SearchStop(stop).value()->lanlong.lng }, stop_label);
        }
    }
    for (auto& stop : stop_label) {
        map.Add(stop);
    }

    map.Render(out);
}


void ParseAndPrintStat(json::Document& commands, const catalogue::TransportCatalogue& catalogue, std::ostream& output) {
    using namespace std::literals;
    if (!commands.GetRoot().IsMap()) {
        return;
    }
    const auto& requests = commands.GetRoot().AsMap().at("stat_requests").AsArray();
    json::Array all_stat;
    for (const auto& req : requests) {
        if (req.AsMap().at("type").AsString() == "Bus") {
            if (catalogue.SearchRoute(req.AsMap().at("name").AsString()) == nullptr) {
                json::Node not_found{json::Dict{{"request_id", req.AsMap().at("id").AsInt()}, {"error_message", "not found"s}}};
                all_stat.push_back(not_found);
            }
            else {
//                auto [all_stops, unique_stops, actual_distance, curvature] = catalogue.GetRouteInfo(req.AsMap().at("name").AsString());
                auto bus = catalogue.GetRouteInfo(req.AsMap().at("name").AsString());
                json::Node bus_stat{ json::Dict{{"curvature", (bus->real_l/bus->l)}, {"request_id",req.AsMap().at("id").AsInt()}, {"route_length", bus->real_l},{"stop_count", bus->r},{"unique_stop_count", bus->u}}};
                all_stat.push_back(bus_stat);
            }
        }
        else if (req.AsMap().at("type").AsString() == "Stop") {
            if (catalogue.SearchStop(req.AsMap().at("name").AsString()) == nullptr) {
                json::Node not_found{ json::Dict{{"request_id", req.AsMap().at("id").AsInt()}, {"error_message", "not found"s}} };
                all_stat.push_back(not_found);
            }
            else {
                std::set<std::string_view> buses_for_stop = (catalogue.GetStopBuses(req.AsMap().at("name").AsString())).value().buses_to_stop;
                json::Array ar = MakeArray(buses_for_stop);
                json::Node stop_stat{ json::Dict{{"buses", ar},{"request_id", req.AsMap().at("id").AsInt()} }};
                all_stat.push_back(stop_stat);
            }
        }
        else if (req.AsMap().at("type").AsString() == "Map") {
            std::ostringstream map;
            ApplyRenderSettings(commands,catalogue, map);
            json::Node map_stat{ json::Dict{{"map", map.str()}, {"request_id", req.AsMap().at("id").AsInt() }}};
            all_stat.push_back(map_stat);
        }
    }
    json::Print(json::Document{ json::Node{all_stat} }, output);
    //json::Print(json::Document{ json::Node{all_stat} }, std::cout);
    //std::cout<<std::endl;
}

