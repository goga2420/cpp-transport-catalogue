
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include "geo.h"

namespace catalogue{

namespace entity{

struct Info{
    std::string name;
    int stop_count;
    int unique;
    double l;
    int real_l;
};

struct BusesToStop{
    std::set<std::string_view> buses_to_stop;
};

struct Stop{
    std::string stop_name_;
    geo::Coordinates lanlong;
    std::unordered_set<std::string_view> buses_to_stop;
};
struct Bus{
    std::string route_id;
    //std::vector<Stop*> route_stops_;
    std::vector<std::string_view> route_stops_;
};

}
}
