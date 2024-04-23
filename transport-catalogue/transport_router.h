#pragma once
#include "router.h"
#include "json.h"
#include "transport_catalogue.h"

class TransportRouter{
public:
    
    TransportRouter() = default;
    
    TransportRouter(int bus_wait, double velocity): bus_wait_(bus_wait), velocity_(velocity){}
    
    void SetWaitTime(int wait_time);
    void SetVelocity(double velocity);
    
    int GetWaitTime() const;
    double GetVelocity() const;
    
    void BuildGraph(catalogue::TransportCatalogue& catalogue, json::Array& stops);
    void FillCatalogueGraph();
    
    graph::DirectedWeightedGraph<double>& GetGraph();
    std::unordered_map<std::string, std::pair<size_t, size_t>> GetStopEdges();
    graph::Router<double>* GetRouter();
    
private:
    int bus_wait_ = 0;
    double velocity_ = 0.0;
    graph::DirectedWeightedGraph<double>graph_;
    std::unique_ptr<graph::Router<double>> router_ = nullptr;
    std::unordered_map<std::string, std::pair<size_t, size_t>> stop_edge;
};
