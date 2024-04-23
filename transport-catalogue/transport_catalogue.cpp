#include "transport_catalogue.h"


namespace  catalogue
{

void TransportCatalogue::AddRoute(const std::string& route_name, const std::vector<std::string_view>& stops){
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

void TransportCatalogue::AddStop(const std::string& stop, geo::Coordinates latlong)
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
    
    
   
    stops_.push_back({stop, latlong, buses});
    stop_index[stops_.back().stop_name_] = &stops_.back();
   
}

void TransportCatalogue::AddStopDistances(const std::string& stop, std::unordered_map<std::string_view, int>& stop_lengths){
    std::unordered_map<std::string_view, int> stop_lengths_copy;
       for (const auto& [key, value] : stop_lengths) {
           stop_lengths_copy.insert({key, value});
       }
       
       // Обновляем значения в stop_lengths_copy, если они есть в stop_index
    std::unordered_map<std::string, int> updated_stop_lengths_copy;

    // Обновляем значения в stop_lengths_copy, если они есть в stop_index
    for(const auto& [key, value] : stop_lengths_copy)
    {
        auto it = stop_index.find(key);
        if (it != stop_index.end()) {
            // Используем новый ключ и сохраняем значение value
            updated_stop_lengths_copy.emplace(it->second.value()->stop_name_, value);
        } else {
            // Если значение не найдено в stop_index, оставляем его без изменений
            updated_stop_lengths_copy.emplace(key, value);
        }
    }
    stop_length.insert({stop, updated_stop_lengths_copy});
}

int TransportCatalogue::DistanceBetweenStops(const std::string& start, const std::string& end){
    if(stop_length.find(start) != stop_length.end()){
        if(stop_length.at(start).find(end) != stop_length.at(start).end())
            return stop_length.at(start).at(end);
        else
            return stop_length.at(end).at(start);
    }
    else
        return stop_length.at(end).at(start);
}

int TransportCatalogue::DistanceForEdges(std::string_view stop1, std::string_view stop2) const {
    const entity::Stop* from = SearchStop(stop1).value();
    const entity::Stop* to = SearchStop(stop2).value();
    
    if (stop_length.count(from->stop_name_) && stop_length.at(from->stop_name_).count(to->stop_name_)) {
        return stop_length.at(from->stop_name_).at(to->stop_name_);
    }
    return 0;
}

const std::deque<entity::Bus> TransportCatalogue::GetAllRoutes() const{
    return routes_;
}

std::optional<const entity::Bus*> TransportCatalogue::SearchRoute(std::string_view name) const{
    auto it = route_index.find(name);
    if(it != route_index.end())
        return it->second;
    return nullptr;
}



std::optional<const entity::Stop*> TransportCatalogue::SearchStop(std::string_view name) const{
    auto it = stop_index.find(name);
    if(it != stop_index.end())
        return it->second;
    else
        return nullptr;
}


std::optional<entity::Info> TransportCatalogue::GetRouteInfo(std::string_view name) const
{
    
    auto it = SearchRoute(name);
    if(it == nullptr)
        return std::nullopt;
    
    else
    {
        //return (*it).second;
        int R = 1;
        geo::Coordinates from, to;
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
        double total_length = 0.0;
        for (int i = 1; i < (*it)->route_stops_.size(); i++)
        {
            const auto &current_stop = std::string((*it)->route_stops_[i]);
            const auto &previous_stop = std::string((*it)->route_stops_[i - 1]);

            // Проверяем, существует ли информация о расстоянии до следующей остановки
            if (stop_length.find(previous_stop) != stop_length.end())
            {
                const auto &lengths = stop_length.at(previous_stop);
                const auto &lengths_cicle = stop_length.at(current_stop);

                // Проверяем, существует ли информация о расстоянии до текущей остановки
                if (lengths.find(current_stop) != lengths.end())
                {
                    // Если информация о расстоянии есть, добавляем ее к суммарной длине маршрута
                    total_length += lengths.at(current_stop);
                }
                else if (lengths_cicle.find(previous_stop) != lengths.end())
                {
                    total_length += lengths_cicle.at(previous_stop);
                }
            }
        }
        entity::Info info;
        info.name = name;
        info.stop_count = R;
        info.unique = U;
        info.l = L;
        info.real_l = total_length;
        return info;
    }
}


std::optional<entity::BusesToStop> TransportCatalogue::GetStopBuses(std::string_view stop) const
{
    auto it = SearchStop(stop);
    if(it != nullptr)
    {
        entity::BusesToStop buses;
        
        for(auto bus:(it).value()->buses_to_stop)
        {
            
            buses.buses_to_stop.insert(bus);
        }
        
        
        return buses;
    }
    return std::nullopt;
}
}
