#include "transport_router.h"

void TransportRouter::SetWaitTime(int bus_wait){
    bus_wait_ = bus_wait;
}
void TransportRouter::SetVelocity(double velocity){
    velocity_ = velocity;
}

int TransportRouter::GetWaitTime() const{
    return bus_wait_;
}
double TransportRouter::GetVelocity() const{
    return velocity_;
}


void TransportRouter::BuildGraph(catalogue::TransportCatalogue& catalogue, json::Array& stops){

    size_t k = 0;
    graph::DirectedWeightedGraph<double> graph(stops.size()*2);
    for (const json::Node& stop : stops) {
        stop_edge[stop.AsString()] = {k,k + 1};
        graph.AddEdge(graph::Edge<double>{ k, k + 1, bus_wait_*1.0, "", stop.AsString(), 0  });
        k += 2;
    }
    for (const catalogue::entity::Bus& bus : catalogue.GetAllRoutes()) {
        for (int i = 0; i < bus.route_stops_.size() - 1; ++i) {
            int span_count=0;
            double road_distance = 0.0;
            for (int j = i + 1; j < bus.route_stops_.size(); ++j) {
                
                road_distance += (catalogue.DistanceBetweenStops(std::string(bus.route_stops_[j-1]), std::string(bus.route_stops_[j])))*1.0;
                
                graph.AddEdge(graph::Edge<double>{stop_edge[std::string(bus.route_stops_[i])].second, stop_edge[std::string(bus.route_stops_[j])].first, (road_distance) / (velocity_ * 100 / 6), bus.route_id,"",++span_count});
               
            }
        }
    }
    graph_ = std::move(graph);
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
