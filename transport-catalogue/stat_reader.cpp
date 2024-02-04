#include "stat_reader.h"


void ParseAndPrintStat(const catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output)
{
    if(request[0] == 'B')
    {
        request = request.substr(4);
        if(auto bus = transport_catalogue.GetRouteInfo(request); bus.has_value())
        {
            output <<"Bus "<< request << ": "<<bus->r<<" stops on route, "<<bus->u<<" unique stops, "<<bus->l<<" route length"<<std::endl;
        }
        else output<<"Bus "<<request<<": not found"<<std::endl;
    }
    else if(request[0] == 'S')
    {
        request = request.substr(5);
        if(auto buses = transport_catalogue.GetStopBuses(request); buses.has_value())
        {
            if(buses.value().buses_to_stop.size() != 0)
            {
                output<<"Stop "<<request<<": buses ";
                for(auto bus:buses.value().buses_to_stop)
                {
                    output<<bus<<" ";
                }
                output<<std::endl;
                return;
            }
            output <<"Stop "<<request<<": no buses"<<std::endl;
            return;
        }
        else{
            output<<"Stop "<<request<<": not found"<<std::endl;
            return;
        }
    }
}
