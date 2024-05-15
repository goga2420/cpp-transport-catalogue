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

graph::DirectedWeightedGraph<double> TransportRouter::FillStopsEdges(std::deque<catalogue::entity::Stop> stops, graph::DirectedWeightedGraph<double> graph){
    size_t k = 0;
    for (catalogue::entity::Stop stop : stops) {
        stop_edge[stop.stop_name_] = {k,k + 1};
        graph.AddEdge(graph::Edge<double>{ k, k + 1, bus_specs.bus_wait_*1.0, "", stop.stop_name_, 0  });
        k += 2;
    }
    return graph;
}

graph::DirectedWeightedGraph<double> TransportRouter::FillGraphBuses(graph::DirectedWeightedGraph<double> graph){
    
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
    return graph;
}

void TransportRouter::BuildGraph(){

    auto stops_ = transport_catalogue_.GetStops();
    graph::DirectedWeightedGraph<double> graph(stops_.size()*2);

    graph_ = FillGraphBuses(FillStopsEdges(stops_, graph));
}

graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph(){
    return graph_;
}

std::unordered_map<std::string, std::pair<size_t, size_t>> TransportRouter::GetStopEdges(){
    return stop_edge;
}
graph::Router<double>* TransportRouter::GetRouter()
{
        return router_.get();
}
