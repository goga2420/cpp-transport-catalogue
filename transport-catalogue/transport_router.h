#pragma once
#include "router.h"
#include "json.h"
#include "transport_catalogue.h"
#include <memory>


class TransportRouter{
public:
    
    TransportRouter() = default;
    
    explicit TransportRouter(int bus_wait, double velocity, catalogue::TransportCatalogue& transport_catalogue){
        bus_specs.bus_wait_ = bus_wait;
        bus_specs.velocity_ = velocity;
        transport_catalogue_ = transport_catalogue;
        BuildGraph();
    }
    
    void SetWaitTime(int wait_time);
    void SetVelocity(double velocity);
    void SetCatalogue(catalogue::TransportCatalogue transport_catalogue);
    void BuildGraph();
    std::vector<graph::Edge<double>> GetBestRoad(std::string from, std::string to);
    
private:
    void FillStopsEdges(std::deque<catalogue::entity::Stop>& stops, graph::DirectedWeightedGraph<double>& graph);
    void FillGraphBuses(graph::DirectedWeightedGraph<double>& graph);
    
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
