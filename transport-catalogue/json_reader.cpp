#include "json_reader.h"

namespace json_reader{

json::Document JsonReader::LoadJSON(const std::string& s) {
    std::istringstream strm(s);
    return json::Load(strm);
}

void JsonReader::BaseRequest(catalogue::TransportCatalogue& catalogue, std::istream& input, std::ostream& output) {
    json::Document command = json::Load(input);
    ApplyCommands(command, catalogue);
    //ApplyRenderSettings(command, catalogue, output);
    ParseAndPrintStat(command, catalogue, output);
}

std::string JsonReader::Print(const json::Node& node) {
    std::ostringstream out;
    json::Print(json::Document{ node }, out);
    return out.str();
}

std::unordered_map < std::string_view, int> JsonReader::ParseStopDistances(const json::Dict& stops) {
    std::unordered_map<std::string_view, int> result;
    for (const auto& stop : stops) {
        result.emplace(stop.first, stop.second.AsInt());
    }
    return result;
}

std::vector<std::string_view> JsonReader::ParseRoute(const json::Array& route, const bool& is_roundtrip) {
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

void JsonReader::ApplyCommands(json::Document& commands, catalogue::TransportCatalogue& catalogue) {
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
            catalogue.AddStop(info.AsMap().at("name").AsString(), geo::Coordinates{ info.AsMap().at("latitude").AsDouble(), info.AsMap().at("longitude").AsDouble() });
        }
        else if (info.AsMap().at("type").AsString() == "Bus") {
            buses.push_back(info);
        }
    }
    
    for (const auto& stop : stop_distances) {
        auto stops = ParseStopDistances(stop.second);
        catalogue.AddStopDistances(stop.first, stops);
    }
    
    for (const auto& bus : buses) {
        catalogue.AddRoute(bus.AsMap().at("name").AsString(), ParseRoute(bus.AsMap().at("stops").AsArray(), bus.AsMap().at("is_roundtrip").AsBool()));
    }
    
    for (const auto& stop : stops) {
        
        catalogue.AddStop(stop.AsMap().at("name").AsString(), geo::Coordinates{ stop.AsMap().at("latitude").AsDouble(), stop.AsMap().at("longitude").AsDouble() });
    }
    
}



json::Array JsonReader::MakeArray(std::set<std::string_view>& original) {
    json::Array result;
    for (auto& element : original) {
        result.push_back(std::string(element));
    }
    return result;
}


std::vector<geo::Coordinates> JsonReader::GetVectorOfCoordinates(const std::vector<std::string_view> stops, const catalogue::TransportCatalogue& catalogue) {
    std::vector<geo::Coordinates> result;
    for (const auto& stop : stops) {
        result.push_back(geo::Coordinates{catalogue.SearchStop(stop).value()->lanlong.lat,catalogue.SearchStop(stop).value()->lanlong.lng });
    }
    return result;
}



std::vector<geo::Coordinates> JsonReader::AllCoordinates(std::set<std::string> stops,const  catalogue::TransportCatalogue& catalogue) {
    std::vector<geo::Coordinates> result;
    for (const auto& stop : stops) {
        if(catalogue.GetStopBuses(stop) != std::nullopt && !catalogue.GetStopBuses(stop).value().buses_to_stop.empty())
            result.push_back(geo::Coordinates{ catalogue.SearchStop(stop).value()->lanlong.lat, catalogue.SearchStop(stop).value()->lanlong.lng });
    }
    return result;
}

void JsonReader::ApplyRenderSettings(json::Document& commands,const catalogue::TransportCatalogue& catalogue, std::ostream& out) {
    render::MapRenderer map_render;
    render::MapRenderer::RenderSettings render_settings = map_render.FillRenderSettings(commands.GetRoot().AsMap().at("render_settings").AsMap());
    
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
        map_render.FillMap(GetVectorOfCoordinates(catalogue.SearchRoute(bus.first).value()->route_stops_, catalogue), map, render_settings,proj, counter, bus.second,bus.first, bus_label);
    }
    for (auto& bus : bus_label) {
        map.Add(bus);
    }
    
    for (auto& stop : stops) {
        if (catalogue.GetStopBuses(stop) != std::nullopt && !catalogue.GetStopBuses(stop).value().buses_to_stop.empty() ) {
            map_render.FillStops(render_settings, map, proj, stop, geo::Coordinates{ catalogue.SearchStop(stop).value()->lanlong.lat, catalogue.SearchStop(stop).value()->lanlong.lng }, stop_label);
        }
    }
    for (auto& stop : stop_label) {
        map.Add(stop);
    }
    
    map.Render(out);
}


void JsonReader::ParseAndPrintStat(json::Document& commands, const catalogue::TransportCatalogue& catalogue, std::ostream& output) {
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
                
                auto bus = catalogue.GetRouteInfo(req.AsMap().at("name").AsString());
                json::Node bus_stat{ json::Dict{{"curvature", (bus->real_l/bus->l)}, {"request_id",req.AsMap().at("id").AsInt()}, {"route_length", bus->real_l},{"stop_count", bus->stop_count},{"unique_stop_count", bus->unique}}};
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

}

}
