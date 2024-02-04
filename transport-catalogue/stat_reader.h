#pragma once
#include <iosfwd>
#include <string_view>
#include <iomanip>
#include <set>
#include "transport_catalogue.h"
#include "geo.h"

void ParseAndPrintStat(const catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output) ;
