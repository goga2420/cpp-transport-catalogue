#pragma once
#include "domain.h"
#include <algorithm>
#include <iostream>
#include <deque>
#include <sstream>
#include <optional>



namespace catalogue
{

class TransportCatalogue {
    // Реализуйте класс самостоятельно
public:
    
    void AddRoute(const std::string& route_name, const std::vector<std::string_view>& stops);
    void AddStop(const std::string& stop, geo::Coordinates latlong);
    void AddStopDistances(const std::string& stop, std::unordered_map<std::string_view, int>& stop_lengths);
    int DistanceBetweenStops(const std::string& start, const std::string& end);
    int DistanceForEdges(std::string_view stop1, std::string_view stop2) const;
    const std::deque<entity::Bus>GetAllRoutes() const;
    std::optional<const entity::Bus*> SearchRoute(std::string_view name) const;
    std::optional<const entity::Stop*> SearchStop(std::string_view name) const;
    std::optional<entity::Info> GetRouteInfo(std::string_view name) const ;
    std::optional<entity::BusesToStop> GetStopBuses(std::string_view stop) const;
    std::deque<entity::Stop> GetStops();
private:
    
    std::deque<entity::Bus> routes_;
    std::deque<entity::Stop> stops_;
    std::unordered_map<std::string_view, std::optional<const entity::Bus*>> route_index;
    std::unordered_map<std::string_view, std::optional<const entity::Stop*>> stop_index;
    std::unordered_map<std::string, std::unordered_map<std::string, int>> stop_length;
};
}
