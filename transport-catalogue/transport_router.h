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
    TransportRouter(tcat::TransportCatalogue& catalogue, graph::DirectedWeightedGraph<double> graph
                   , std::unordered_map<std::string, size_t> wait_vertexes
                   , std::unordered_map<std::string, size_t> travel_vertexes);
    
    void LoadSettings(RouterSettings settings);
    const RouterSettings& GetSettings() const;
    
    const graph::DirectedWeightedGraph<double>& GetGraph() const;
    const std::unordered_map<std::string, size_t>& GetWaitVertexes() const;
    const std::unordered_map<std::string, size_t>& GetTravelVertexes() const;
    
    RouteData CalculateRoute(std::string from, std::string to);
    
    
private:
    tcat::TransportCatalogue& tc_;
    RouterSettings settings_;
    
    std::unordered_map<std::string, size_t> wait_vertexes_;
    std::unordered_map<std::string, size_t> travel_vertexes_;
    
    graph::DirectedWeightedGraph<double> graph_;
    std::unique_ptr<graph::Router<double>> router_ = nullptr;
    
    void BuildGraph();
    
};
    
}
