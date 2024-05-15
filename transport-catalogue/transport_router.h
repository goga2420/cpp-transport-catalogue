#pragma once
#include "router.h"
#include "json.h"
#include "transport_catalogue.h"


class TransportRouter{
public:
    
    TransportRouter() = default;
    
    TransportRouter(int bus_wait, double velocity, catalogue::TransportCatalogue transport_catalogue){
        bus_specs.bus_wait_ = bus_wait;
        bus_specs.velocity_ = velocity;
        transport_catalogue_ = transport_catalogue;
    }
    
    void SetWaitTime(int wait_time);
    void SetVelocity(double velocity);
    void SetCatalogue(catalogue::TransportCatalogue transport_catalogue);
    void BuildGraph();
    graph::DirectedWeightedGraph<double>& GetGraph();
    std::unordered_map<std::string, std::pair<size_t, size_t>> GetStopEdges();
    graph::Router<double>* GetRouter();
    
private:
    
    graph::DirectedWeightedGraph<double> FillStopsEdges(std::deque<catalogue::entity::Stop> stops, graph::DirectedWeightedGraph<double> graph);
    graph::DirectedWeightedGraph<double> FillGraphBuses(graph::DirectedWeightedGraph<double> graph);
    
    struct BusSpecs{
        int bus_wait_ = 0;
        double velocity_ = 0.0;
    };
    
    BusSpecs bus_specs;
    graph::DirectedWeightedGraph<double>graph_;
    std::unique_ptr<graph::Router<double>> router_ = nullptr;
    std::unordered_map<std::string, std::pair<size_t, size_t>> stop_edge;
    catalogue::TransportCatalogue transport_catalogue_;
};
