#include "transport_catalogue.h"


namespace  catalogue
{

void TransportCatalogue::AddRoute(const std::string route_name, const std::vector<std::string_view> stops){
    std::vector<std::string_view>stops2;
    for(std::string_view stop:stops)
    {
        auto it = stop_index.find(stop);
        if(it!= stop_index.end())
            stops2.push_back(it->first);
    }
    routes_.push_back({route_name, stops2});
    route_index[routes_.back().route_id] = &routes_.back();
}


void TransportCatalogue::AddStop(const std::string stop, Coordinates latlong)
{
    std::unordered_set<std::string_view>buses;
    //std::unordered_map<std::string, std::unordered_set<std::string_view>>buses_to_stop;
    for(auto [bus, strukt]:route_index)
    {
        if(std::find(strukt.value()->route_stops_.begin(), strukt.value()->route_stops_.end(), stop)!= strukt.value()->route_stops_.end())
        {
            buses.insert(bus);
        }
    }
    
    //stops_.push_back({stop, latlong});
    stops_.push_back({stop, latlong, buses});
    stop_index[stops_.back().stop_name_] = &stops_.back();
}


std::optional<const TransportCatalogue::Bus*> TransportCatalogue::SearchRoute(std::string_view name) const{
    auto it = route_index.find(name);
    if(it != route_index.end())
        return it->second;
    return nullptr;
}


std::optional<const TransportCatalogue::Stop*> TransportCatalogue::SearchStop(std::string_view name) const{
    auto it = stop_index.find(name);
    if(it != stop_index.end())
        return it->second;
    else
        return nullptr;
}

//std::optional<const TransportCatalogue::Bus*>
std::optional<TransportCatalogue::Info> TransportCatalogue::GetRouteInfo(std::string_view name) const
{
    
    auto it = SearchRoute(name);
    if(it == nullptr)
        return std::nullopt;
    
    else
    {
        //return (*it).second;
        int R = 1;
        Coordinates from, to;
        double L = 0.0;
        for(int i =1; i<(*it)->route_stops_.size(); i++)
        {
            from.lat = SearchStop((*it)->route_stops_[i-1]).value()->lanlong.lat;
            from.lng = SearchStop((*it)->route_stops_[i-1]).value()->lanlong.lng;
            to.lat = SearchStop((*it)->route_stops_[i]).value()->lanlong.lat;
            to.lng = SearchStop((*it)->route_stops_[i]).value()->lanlong.lng;
            L += ComputeDistance(from, to);
            R += 1;
        }
        std::set<std::string_view> unique;
        unique.insert((*it)->route_stops_.begin(), (*it)->route_stops_.end());
        int U = static_cast<int>(unique.size());
        Info info;
        info.name = name;
        info.r = R;
        info.u = U;
        info.l = L;
        return info;
    }
}

std::optional<TransportCatalogue::BusesToStop> TransportCatalogue::GetStopBuses(std::string_view stop) const
{
    auto it = SearchStop(stop);
    if(it != nullptr)
    {
        BusesToStop buses;
        //std::set<std::string_view>buses;
        for(auto bus:(it).value()->buses_to_stop)
        {
            //auto str = bus;
            buses.buses_to_stop.insert(bus);
        }
        
        //buses.buses_to_stop.insert(*it.value()->buses_to_stop.begin(), *it.value()->buses_to_stop.end());
        return buses;
    }
    return std::nullopt;
}
}
