#include "transport_router.h"

#include "graph.h"
#include "router.h"

#include <utility>
#include <memory>
#include <string_view>

using namespace std;

namespace router {
    
    
TransportRouter::TransportRouter(tcat::TransportCatalogue& tc) : tc_(tc), graph_(tc.GetAllStopsCount() * 2) {
	//BuildGraph();
}
    
TransportRouter::TransportRouter(tcat::TransportCatalogue& tc, graph::DirectedWeightedGraph<double> graph
                   , std::unordered_map<std::string, size_t> wait_vertexes
                   , std::unordered_map<std::string, size_t> travel_vertexes) 
    : tc_(tc), graph_(graph), wait_vertexes_(wait_vertexes), travel_vertexes_(travel_vertexes) {
        router_ = make_unique<graph::Router<double>>(graph);
    }
    
void TransportRouter::LoadSettings(RouterSettings settings) {
    settings_ = move(settings);
}
    
const RouterSettings& TransportRouter::GetSettings() const {
    return settings_;
}
    
const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const {
    return graph_;
}
    
const unordered_map<std::string, size_t>& TransportRouter::GetWaitVertexes() const {
    return wait_vertexes_;
}
const unordered_map<std::string, size_t>& TransportRouter::GetTravelVertexes() const {
    return travel_vertexes_;
}
    
RouteData TransportRouter::CalculateRoute(string from, string to) {
    if (!router_) {
        BuildGraph();
    }
    RouteData result;
    auto calculated_route = router_->BuildRoute(wait_vertexes_.at(from), wait_vertexes_.at(to));
    
    if (calculated_route) {
        result.is_found = true;
        for (const auto& id : calculated_route->edges) {
            auto edge = graph_.GetEdge(id);
            result.total_time += edge.weight;
            
            result.items.emplace_back(RouteInfo{
			    	     edge.name,
                                     (edge.type == graph::EdgeType::TRAVEL) ? edge.span_count : 0,
                                      edge.weight,
                                      edge.type});
        }
    }
    return result;
}
    
void TransportRouter::BuildGraph() {
    size_t vertex_id = 0;
    
    for (const auto& [_, stop_ptr] : tc_.GetAllStops()) {
	string name = stop_ptr->name;
        wait_vertexes_[name] = vertex_id++;
        travel_vertexes_[name] = vertex_id++;
            
        graph_.AddEdge({
            wait_vertexes_.at(name),
            travel_vertexes_.at(name),
	    name,
            graph::EdgeType::WAIT,
            0,
            settings_.bus_wait_time * 1.0
        });
    }
    
    for (const auto& [_, bus_ptr] : tc_.GetAllBuses()) {
        for (size_t it_from = 0; it_from < bus_ptr->stops.size() - 1; ++it_from) {
            int span_count = 0;
            for (size_t it_to = it_from + 1; it_to < bus_ptr->stops.size(); ++it_to) {
                double road_length = 0.0;
                for (size_t it = it_from + 1; it <= it_to; ++it) {
                    road_length += static_cast<double>(tc_.GetDistance(bus_ptr->stops[it - 1], bus_ptr->stops[it]));
                }
                graph_.AddEdge({
                    travel_vertexes_.at(bus_ptr->stops.at(it_from)->name),
                    wait_vertexes_.at(bus_ptr->stops.at(it_to)->name),
		    bus_ptr->name,
                    graph::EdgeType::TRAVEL,
                    ++span_count,
                    road_length / (settings_.bus_velocity * 1000. / 60.)
                });
            }
        }
    }
    router_ = make_unique<graph::Router<double>>(graph_);
}
    
}
