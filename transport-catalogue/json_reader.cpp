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
    for (const json::Node& stop : route) {
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
    if (!commands.GetRoot().IsDict()) {
        return;
    }
    std::unordered_map<std::string, json::Dict> stop_distances;
    json::Array buses;
    json::Array stops;
    const json::Array& com = commands.GetRoot().AsDict().at("base_requests").AsArray();
    
    for (const json::Node& info : com) {
        if (info.AsDict().at("type").AsString() == "Stop") {
            stops.push_back(info);
            stop_distances[info.AsDict().at("name").AsString()] = info.AsDict().at("road_distances").AsDict();
            catalogue.AddStop(info.AsDict().at("name").AsString(), geo::Coordinates{ info.AsDict().at("latitude").AsDouble(), info.AsDict().at("longitude").AsDouble() });
        }
        else if (info.AsDict().at("type").AsString() == "Bus") {
            buses.push_back(info);
        }
    }
    
    for (const auto& stop : stop_distances) {
        auto stops = ParseStopDistances(stop.second);
        catalogue.AddStopDistances(stop.first, stops);
    }
    
    for (const json::Node& bus : buses) {
        catalogue.AddRoute(bus.AsDict().at("name").AsString(), ParseRoute(bus.AsDict().at("stops").AsArray(), bus.AsDict().at("is_roundtrip").AsBool()));
    }
    
    for (const json::Node& stop : stops) {
        
        catalogue.AddStop(stop.AsDict().at("name").AsString(), geo::Coordinates{ stop.AsDict().at("latitude").AsDouble(), stop.AsDict().at("longitude").AsDouble() });
    }
    
    const json::Dict& rooting_settings = commands.GetRoot().AsDict().at("routing_settings").AsDict();
    
    transport_router_.SetVelocity(rooting_settings.at("bus_velocity").AsDouble());
    transport_router_.SetWaitTime(rooting_settings.at("bus_wait_time").AsInt());
    transport_router_.SetCatalogue(catalogue);
    transport_router_.BuildGraph();
    
}

json::Dict JsonReader::PrintGraph(const json::Node& req, const graph::Router<double>& transport_router)
{
    using namespace std::literals;
    std::string from = req.AsDict().at("from").AsString();
    std::string to = req.AsDict().at("to").AsString();
    std::unordered_map<std::string, std::pair<size_t, size_t>> stops_edges = transport_router_.GetStopEdges();
    if (from == to) {
        return json::Builder{}.StartDict().Key("total_time").Value(0).Key("request_id").Value(req.AsDict().at("id").AsInt()).Key("items").StartArray().EndArray().EndDict().Build().AsDict();
    }
    else {
        const std::optional<graph::Router<double>::RouteInfo> info = transport_router.BuildRoute(stops_edges.at(from).first, stops_edges.at(to).first);
        if (info.has_value()) {
            const std::vector<graph::EdgeId>& elem = info.value().edges;
            json::Array rout_arr;

            double total_time = 0.0;
            for (const graph::EdgeId& el : elem) {
                json::Dict item_map;
                const graph::Edge<double> edge = transport_router_.GetGraph().GetEdge(el);
                if (edge.bus.empty()) {
                    item_map["type"] = "Wait"s;
                    item_map["stop_name"] = edge.stop;
                    item_map["time"] = edge.weight;

                    rout_arr.push_back(item_map);
                }
                else {
                    item_map["type"] = "Bus"s;
                    item_map["bus"] = edge.bus;
                    item_map["span_count"] = edge.span_count;
                    item_map["time"] = edge.weight;
                    rout_arr.push_back(item_map);
                }
                total_time += edge.weight;
            }
            return json::Builder{}
                .StartDict()
                .Key("total_time").Value(total_time)
                .Key("request_id").Value(req.AsDict().at("id").AsInt()).Key("items").Value(rout_arr)
                .EndDict()
                .Build()
                .AsDict();
        }
        
    }
    return
        json::Builder{}
        .StartDict()
        .Key("request_id").Value(req.AsDict().at("id").AsInt())
        .Key("error_message").Value("not found")
        .EndDict()
        .Build()
        .AsDict();
}


json::Array JsonReader::MakeArray(std::set<std::string_view>& original) {
    json::Array result;
    for (const std::string_view& element : original) {
        result.push_back(std::string(element));
    }
    return result;
}


std::vector<geo::Coordinates> JsonReader::GetVectorOfCoordinates(const std::vector<std::string_view> stops, const catalogue::TransportCatalogue& catalogue) {
    std::vector<geo::Coordinates> result;
    for (const std::string_view& stop : stops) {
        result.push_back(geo::Coordinates{catalogue.SearchStop(stop).value()->lanlong.lat,catalogue.SearchStop(stop).value()->lanlong.lng });
    }
    return result;
}



std::vector<geo::Coordinates> JsonReader::AllCoordinates(std::set<std::string> stops,const  catalogue::TransportCatalogue& catalogue) {
    std::vector<geo::Coordinates> result;
    for (const std::string& stop : stops) {
        if(catalogue.GetStopBuses(stop) != std::nullopt && !catalogue.GetStopBuses(stop).value().buses_to_stop.empty())
            result.push_back(geo::Coordinates{ catalogue.SearchStop(stop).value()->lanlong.lat, catalogue.SearchStop(stop).value()->lanlong.lng });
    }
    return result;
}

void JsonReader::ApplyRenderSettings(json::Document& commands,const catalogue::TransportCatalogue& catalogue, std::ostream& out) {
    render::MapRenderer map_render;
    render::MapRenderer::RenderSettings render_settings = map_render.FillRenderSettings(commands.GetRoot().AsDict().at("render_settings").AsDict());
    
    svg::Document map;
    std::vector<svg::Text> bus_label, stop_label;
    std::map<std::string, bool> buses;
    std::set<std::string> stops;
    const auto& com = commands.GetRoot().AsDict().at("base_requests").AsArray();
    for (const auto& info : com) {
        if (info.AsDict().at("type").AsString() == "Bus") {
            buses.insert({ info.AsDict().at("name").AsString(), info.AsDict().at("is_roundtrip").AsBool() });
        }
        else if (info.AsDict().at("type").AsString() == "Stop") {
            stops.insert(info.AsDict().at("name").AsString());
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
    graph::DirectedWeightedGraph<double> graph = transport_router_.GetGraph();
    graph::Router<double> transport_router(graph);
    using namespace std::literals;
    if (!commands.GetRoot().IsDict()) {
        return;
    }
    const json::Array& requests = commands.GetRoot().AsDict().at("stat_requests").AsArray();
    json::Array all_stat;
    for (const json::Node& req : requests) {
        if (req.AsDict().at("type").AsString() == "Bus") {
            if (catalogue.SearchRoute(req.AsDict().at("name").AsString()) == nullptr) {

                all_stat.push_back(json::Builder{}.StartDict().Key("request_id"s).Value(req.AsDict().at("id").AsInt()).Key("error_message"s).Value("not found"s).EndDict().Build());
            }
            else {
               
                auto bus = catalogue.GetRouteInfo(req.AsDict().at("name").AsString());
               
                all_stat.push_back(json::Builder{}.StartDict().Key("curvature"s).Value((bus->real_l/bus->l)).Key("request_id"s).Value(req.AsDict().at("id").AsInt()).Key("route_length"s)
                                    .Value(bus->real_l).Key("stop_count"s).Value(bus->stop_count).Key("unique_stop_count"s).Value(bus->unique).EndDict().Build());
            }
        }
        else if (req.AsDict().at("type").AsString() == "Stop") {
            if (catalogue.SearchStop(req.AsDict().at("name").AsString()) == nullptr) {

                all_stat.push_back(json::Builder{}.StartDict().Key("request_id"s).Value(req.AsDict().at("id").AsInt()).Key("error_message"s).Value("not found"s).EndDict().Build());
            }
            else {
                std::set<std::string_view> buses_for_stop = (catalogue.GetStopBuses(req.AsDict().at("name").AsString())).value().buses_to_stop;
                json::Array ar = MakeArray(buses_for_stop);

                all_stat.push_back(json::Builder{}.StartDict().Key("buses"s).Value(ar).Key("request_id"s).Value(req.AsDict().at("id").AsInt()).EndDict().Build());
            }
        }
        else if (req.AsDict().at("type").AsString() == "Map") {
            std::ostringstream map;
            ApplyRenderSettings(commands,catalogue, map);

            all_stat.push_back(json::Builder{}.StartDict().Key("map"s).Value(map.str()).Key("request_id"s).Value(req.AsDict().at("id").AsInt()).EndDict().Build());
        }
        else if (req.AsDict().at("type").AsString() == "Route") {
            all_stat.push_back(PrintGraph(req, transport_router));
        }
    }
    json::Print(json::Document{ json::Node{all_stat} }, output);

}



}
