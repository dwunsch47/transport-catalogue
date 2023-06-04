#include "transport_router.h"

#include "graph.h"
#include "router.h"

#include <utility>
#include <memory>
#include <string_view>

using namespace std;

namespace router {
    
    
TransportRouter::TransportRouter(tcat::TransportCatalogue& tc) : tc_(tc), graph_(tc.GetAllStopsCount() * 2) {}
      
void TransportRouter::LoadSettings(RouterSettings settings) {
    settings_ = move(settings);
}
    
RouteData TransportRouter::CalculateRoute(string_view from, string_view to) {
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
            
            result.items.emplace_back(RouteInfo{edge.name,
                                     (edge.type == graph::EdgeType::TRAVEL) ? edge.span_count : 0,
                                      edge.weight,
                                      edge.type});
        }
    }
    return result;
}
    
void TransportRouter::BuildGraph() {
    size_t vertex_id = 0;
    
    for (const auto& [name, _] : tc_.GetAllStops()) {
        wait_vertexes_[name] = vertex_id++;
        travel_vertexes_[name] = vertex_id++;
            
        graph_.AddEdge({
            wait_vertexes_.at(name),
            travel_vertexes_.at(name),
            string(name),
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