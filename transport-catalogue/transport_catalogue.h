#pragma once
#include <algorithm>
#include <iostream>
#include "geo.h"
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <sstream>
#include <optional>
#include <vector>
#include <set>

namespace catalogue
{

class TransportCatalogue {
    // Реализуйте класс самостоятельно
public:
    
    struct Info{
        std::string name;
        int r;
        int u;
        double l;
    };
    
    struct BusesToStop{
        std::set<std::string_view>buses_to_stop;
    };
    
    struct Stop{
        std::string stop_name_;
        Coordinates lanlong;
        std::unordered_set<std::string_view>buses_to_stop;
    };
    struct Bus{
        std::string route_id;
        //std::vector<Stop*> route_stops_;
        std::vector<std::string_view> route_stops_;
    };
    
    void AddRoute(const std::string& route_name, const std::vector<std::string_view>& stops);
    void AddStop(const std::string& stop, Coordinates latlong);
    std::optional<const TransportCatalogue::Bus*> SearchRoute(std::string_view name) const;
    std::optional<const TransportCatalogue::Stop*> SearchStop(std::string_view name) const;
    std::optional<TransportCatalogue::Info> GetRouteInfo(std::string_view name) const ;
    std::optional<TransportCatalogue::BusesToStop> GetStopBuses(std::string_view stop) const;
private:
    
    std::deque<Bus> routes_;
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, std::optional<const Bus*>> route_index;
    std::unordered_map<std::string_view, std::optional<const Stop*>> stop_index;
};
}
