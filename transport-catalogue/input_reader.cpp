

#include "input_reader.h"
//#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace input
{

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");
    
    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');
    
    if (comma == str.npos) {
        return {nan, nan};
    }
    
    auto not_space2 = str.find_first_not_of(' ', comma + 1);
    
    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));
    
    return {lat, lng};
}



std::unordered_map<std::string_view, int> ParseStopLengths(std::string_view str)
{
    std::unordered_map<std::string_view, int> result;
    
    auto comma = str.find(',');
    str = str.substr(comma+1, str.size());
    comma = str.find(',');
    str = str.substr(comma+2, str.size());
    while(str.size()!=0)
    {
        auto to = str.find("to");
        comma = str.find(',');
        if(str.find(',') == std::string::npos)
        {
            auto len = str.substr(0, to-1);
            auto stop = str.substr(to+3, str.size()-1);
            result.insert({stop, std::stoi(std::string(len.substr(0, len.size()-1)))});
            //str = str.substr(comma+2);
            break;
        }
        auto len = str.substr(0, to-1);
        auto stop = str.substr(to+3, comma-to-3);
        result.insert({stop, std::stoi(std::string(len.substr(0, len.size()-1)))});
        str = str.substr(comma+2);
        
    }
    
    return result;
}


/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;
    
    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }
    
    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }
    
    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());
    
    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }
    
    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }
    
    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }
    
    return {std::string(line.substr(0, space_pos)),
        std::string(line.substr(not_space, colon_pos - not_space)),
        std::string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] catalogue::TransportCatalogue& catalogue) const {
    // Реализуйте метод самостоятельно
    for(int i = 0; i < commands_.size(); i++)
    {
        if(commands_[i].command == "Stop")
        {
            
            Coordinates lat_long;
            lat_long = ParseCoordinates(commands_[i].description);
            
            std::unordered_map<std::string_view, int> stop_lengths;
            
            stop_lengths = ParseStopLengths(commands_[i].description);
            
            // Вызываем метод AddStop с полученными координатами
            catalogue.AddStop(commands_[i].id, lat_long, stop_lengths);
        }
        else if(commands_[i].command == "Bus")
        {
            std::vector<std::string_view>stops = ParseRoute(commands_[i].description);
            std::vector<std::string_view>stops_correct;
            
            for(auto stop:stops)
            {
                stop = Trim(stop);
                stops_correct.push_back(stop);
                
                
            }
            catalogue.AddRoute(commands_[i].id, stops_correct);
        }
        
    }
}

void InputReader::ReadInputAndGetStats()
{
    catalogue::TransportCatalogue catalogue;
    int base_request_count;
    std::ifstream inputFile("/Users/georgijzukov/Desktop/Transport_Catalogue V2/Transport_Catalogue V2/input2.txt");

    //inputFile >> base_request_count >> std::ws;
    //код выше для проверки тестов с чтением из файла
    
    std::cin >> base_request_count >> std::ws;
    {
        
        InputReader reader;
        std::vector<std::string> save_buses;
        std::vector<std::string> save_stops;
        for (int i = 0; i < base_request_count; ++i) {
            std::string line;
            getline(std::cin, line);
            //getline(inputFile, line);
            if(line[0] == 'B')
                save_buses.push_back(line);
            else
                save_stops.push_back(line);
            reader.ParseLine(line);
        }
        reader.ApplyCommands(catalogue);
        
        for (int i = 0; i < save_buses.size(); ++i) {
            reader.ParseLine(save_buses[i]);
        }
        reader.ApplyCommands(catalogue);
        
        for (int i = 0; i < save_stops.size(); ++i) {
            reader.ParseLine(save_stops[i]);
        }
        reader.ApplyCommands(catalogue);
    }
    
    int stat_request_count;
    std::cin >> stat_request_count >> std::ws;
    //inputFile >> stat_request_count >> std::ws;
    for (int i = 0; i < stat_request_count; ++i) {
        std::string line;
        getline(std::cin, line);
        ParseAndPrintStat(catalogue, line, std::cout);
    }
}
}
