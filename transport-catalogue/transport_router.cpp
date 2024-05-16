#include "transport_router.h"

double MINUTES_IN_HOUR = 60;
double METERS_IN_KM = 1000;

void TransportRouter::SetWaitTime(int bus_wait){
    bus_specs.bus_wait_ = bus_wait;
}
void TransportRouter::SetVelocity(double velocity){
    bus_specs.velocity_ = velocity;
}
void TransportRouter::SetCatalogue(catalogue::TransportCatalogue transport_catalogue){
    transport_catalogue_ = transport_catalogue;
}


void TransportRouter::FillStopsEdges(std::deque<catalogue::entity::Stop>& stops, graph::DirectedWeightedGraph<double>& graph){
    size_t k = 0;
    for (catalogue::entity::Stop stop : stops) {
        stop_edge[stop.stop_name_] = {k,k + 1};
        graph.AddEdge(graph::Edge<double>{ k, k + 1, bus_specs.bus_wait_*1.0, "", stop.stop_name_, 0  });
        k += 2;
    }
}

void TransportRouter::FillGraphBuses(graph::DirectedWeightedGraph<double>& graph){
    
    for (const catalogue::entity::Bus& bus : transport_catalogue_.GetAllRoutes()) {
        for (int i = 0; i < bus.route_stops_.size() - 1; ++i) {
            int span_count=0;
            double road_distance = 0.0;
            for (int j = i + 1; j < bus.route_stops_.size(); ++j) {
                
                road_distance += (transport_catalogue_.DistanceBetweenStops(std::string(bus.route_stops_[j-1]), std::string(bus.route_stops_[j])))*1.0;
                
                graph::VertexId wait_edge = stop_edge[std::string(bus.route_stops_[j])].first;
                graph::VertexId go_edge = stop_edge[std::string(bus.route_stops_[i])].second;
                
                graph.AddEdge(graph::Edge<double>{go_edge, wait_edge, (road_distance) / (bus_specs.velocity_ * METERS_IN_KM / MINUTES_IN_HOUR), bus.route_id,"",++span_count});
               
            }
        }
    }
}

void TransportRouter::BuildGraph(){

    auto stops_ = transport_catalogue_.GetStops();
    graph::DirectedWeightedGraph<double> graph(stops_.size()*2);
    FillStopsEdges(stops_, graph);
    FillGraphBuses(graph);
    graph_ = graph;
    router_ = std::make_unique<graph::Router<double>>(graph_);
}

std::vector<graph::Edge<double>> TransportRouter::GetBestRoad(std::string from, std::string to) {
    const auto route_parts = router_.get()->BuildRoute(stop_edge.at(from).first, stop_edge.at(to).first);
    std::vector<graph::Edge<double>> best_route;
    if (route_parts.has_value()) {
        const std::vector<graph::EdgeId>& elem = route_parts.value().edges;
        for (const graph::EdgeId& el : elem) {
            best_route.push_back(graph_.GetEdge(el));
        }
    }
    return best_route;
}
