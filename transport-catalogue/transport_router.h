#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <string_view>
#include <unordered_map>
#include <memory>

namespace router {
    
struct RouterSettings {
    int bus_wait_time = 0;
    double bus_velocity = 0.0;
};
    
struct RouteInfo {
    std::string name;
    int span_count = 0;
    double time = 0.0;
    graph::EdgeType type;
};
    
struct RouteData {
	double total_time = 0.0;
	std::vector<RouteInfo> items;
	bool is_found = false;
};
    
class TransportRouter {
public:
    TransportRouter(tcat::TransportCatalogue& catalogue);
    
    void LoadSettings(RouterSettings settings);
    
    RouteData CalculateRoute(std::string_view from, std::string_view to);
    
    
private:
    tcat::TransportCatalogue& tc_;
    RouterSettings settings_;
    
    std::unordered_map<std::string_view, size_t> wait_vertexes_;
    std::unordered_map<std::string_view, size_t> travel_vertexes_;
    
    graph::DirectedWeightedGraph<double> graph_;
    std::unique_ptr<graph::Router<double>> router_ = nullptr;
    
    void BuildGraph();
    
};
    
}