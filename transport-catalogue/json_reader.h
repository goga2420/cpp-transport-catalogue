

#pragma once
#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include <sstream>
#include <unordered_set>

json::Document LoadJSON(const std::string& s);

void BaseRequest(catalogue::TransportCatalogue& catalogue, std::istream& input, std::ostream& output);

std::string Print(const json::Node& node);

std::unordered_map < std::string_view, int> ParseStopDistances(const json::Dict& stops);

std::vector<std::string_view> ParseRoute(const json::Array& route, const bool& is_roundtrip);

json::Array MakeArray(std::set<std::string_view> original);

void ApplyCommands(json::Document& commands, catalogue::TransportCatalogue& catalogue);

render::RenderSettings FillRenderSettings(const json::Dict& setting);

std::vector<geo::Coordinates> GetVectorOfCoordinates(const std::vector<std::string_view> stops, catalogue::TransportCatalogue& catalogue);

std::vector<geo::Coordinates> AllCoordinates(std::set<std::string> stops, catalogue::TransportCatalogue& catalogue);

void ApplyRenderSettings(json::Document& commands, const catalogue::TransportCatalogue& catalogue, std::ostream& out);

void ParseAndPrintStat(json::Document& commands, const catalogue::TransportCatalogue& catalogue, std::ostream& output);
